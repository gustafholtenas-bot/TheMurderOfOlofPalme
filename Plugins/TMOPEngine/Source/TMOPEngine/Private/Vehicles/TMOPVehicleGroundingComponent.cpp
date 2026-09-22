#include "Vehicles/TMOPVehicleGroundingComponent.h"

#include "Vehicles/TMOPGroundFitMath.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/HitResult.h"
#include "CoreGlobals.h"
#include "Math/RotationMatrix.h"

UTMOPVehicleGroundingComponent::UTMOPVehicleGroundingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTMOPVehicleGroundingComponent::BeginPlay()
{
    Super::BeginPlay();
    InvalidateGroundCache();
    AddTickPrerequisiteActor(GetOwner());
    if (auto* Movement = GetOwner()->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
        AddTickPrerequisiteComponent(Movement);
}

void UTMOPVehicleGroundingComponent::InvalidateGroundCache() { bHaveCache = false; bGroundValid = false; AcceptedFrame = MAX_uint64; }

bool UTMOPVehicleGroundingComponent::BuildWheels(TArray<FTMOPGroundWheel>& Out, bool& bApproximate) const
{
    bApproximate = false;
    if (!Wheels.IsEmpty()) { Out = Wheels; return Out.Num() >= 3; }
    const auto* Vehicle = Cast<ATMOPConfiguredVehicle>(GetOwner());
    if (Vehicle && Vehicle->VehicleModel && Vehicle->VisualRoot)
    {
        const auto& Setup = Vehicle->VehicleModel->Wheels;
        const FTransform Bases[] = { Setup.FrontLeft, Setup.FrontRight, Setup.RearLeft, Setup.RearRight };
        const UStaticMeshComponent* Parts[] = { Vehicle->WheelFrontLeft, Vehicle->WheelFrontRight, Vehicle->WheelRearLeft, Vehicle->WheelRearRight };
        // Identity wheel transforms are common in older data. Do not treat them as four real contact points.
        if (FVector::Dist(Bases[0].GetLocation(), Bases[1].GetLocation()) > 20 &&
            FVector::Dist(Bases[0].GetLocation(), Bases[2].GetLocation()) > 50)
        {
            const FTransform VisualToActor = Vehicle->VisualRoot->GetComponentTransform().GetRelativeTransform(Vehicle->GetActorTransform());
            for (int32 I = 0; I < 4; ++I)
            {
                FTMOPGroundWheel W;
                W.LocalCenter = VisualToActor.TransformPosition(Bases[I].GetLocation());
                W.RadiusCm = Setup.WheelRadiusCm * VisualToActor.GetScale3D().GetAbsMax();
                if (Parts[I]) W.WheelComponentName = Parts[I]->GetFName();
                Out.Add(W);
            }
            return true;
        }
    }
    const auto* Base = Cast<ATMOPVehicleBase>(GetOwner());
    if (!Base || !Base->VehicleCollision) return false;
    const FVector Extent = Base->VehicleCollision->GetUnscaledBoxExtent();
    bApproximate = true;
    for (int32 I = 0; I < 4; ++I)
    {
        FTMOPGroundWheel W;
        W.LocalCenter = FVector((I < 2 ? 1 : -1) * Extent.X * 0.65,
            (I % 2 ? 1 : -1) * Extent.Y * 0.85, -Extent.Z + W.RadiusCm);
        Out.Add(W);
    }
    return true;
}

bool UTMOPVehicleGroundingComponent::TraceGround(const FVector& Center, float ExpectedGroundZ, FHitResult& Out) const
{
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_WorldStatic);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPWheelGround), bTraceComplex, GetOwner());
    TArray<FHitResult> Hits;
    GetWorld()->LineTraceMultiByObjectType(Hits, Center + FVector(0, 0, SearchUpCm),
        Center - FVector(0, 0, SearchDownCm), Objects, Params);
    bool bFound = false;
    float Best = TNumericLimits<float>::Max();
    const float MinNormalZ = FMath::Cos(FMath::DegreesToRadians(MaxSlopeDegrees));
    for (const auto& Hit : Hits)
    {
        const auto* Primitive = Hit.GetComponent();
        const AActor* HitActor = Hit.GetActor();
        if (!Primitive || !HitActor || HitActor->IsA<APawn>() || HitActor->IsAttachedTo(GetOwner()) ||
            Primitive->Mobility != EComponentMobility::Static || Hit.bStartPenetrating || Hit.ImpactNormal.Z < MinNormalZ) continue;
        if (HitActor->ActorHasTag(TEXT("TMOP_NoGround")) || Primitive->ComponentHasTag(TEXT("TMOP_NoGround"))) continue;
        if (!RequiredGroundTag.IsNone() && !HitActor->ActorHasTag(RequiredGroundTag) && !Primitive->ComponentHasTag(RequiredGroundTag)) continue;
        const float Distance = FMath::Abs(Hit.ImpactPoint.Z - ExpectedGroundZ);
        if (Distance < Best) { Out = Hit; Best = Distance; bFound = true; }
    }
    if (bDrawContacts)
    {
        DrawDebugLine(GetWorld(), Center + FVector(0,0,SearchUpCm), Center - FVector(0,0,SearchDownCm), bFound ? FColor::Green : FColor::Red, false, 0.1f);
        if (bFound) DrawDebugSphere(GetWorld(), Out.ImpactPoint, 4, 8, FColor::Green, false, 0.1f);
    }
    return bFound;
}

bool UTMOPVehicleGroundingComponent::Solve(const FTransform& Requested, FTMOPGroundSolution& Out, float DeltaTime) const
{
    Out = FTMOPGroundSolution(); Out.Pose = Requested;
    if (!GetWorld() || !GetOwner() || Requested.ContainsNaN()) { Out.Error = TEXT("Invalid world/pose"); return false; }
    if (DeltaTime > 0 && bHaveCache && bGroundValid && Cached.bSettled && Requested.Equals(LastApplied, 0.00001))
    { Out = Cached; return true; }
    TArray<FTMOPGroundWheel> Contacts;
    if (!BuildWheels(Contacts, Out.bApproximate)) { Out.Error = TEXT("At least three non-collinear wheel contacts required"); return false; }
    const FVector Scale = Requested.GetScale3D();
    if (Scale.GetMin() <= 0) { Out.Error = TEXT("Vehicle scale must be positive"); return false; }
    if (!FMath::IsNearlyEqual(Scale.GetMin(), Scale.GetMax(), 0.0001))
    { Out.Error = TEXT("Use uniform vehicle scale; non-uniform scale changes tyre radii"); return false; }
    const FQuat Heading = FRotator(0, Requested.Rotator().Yaw, 0).Quaternion();
    FQuat Tilt = Heading;
    // Re-probe at the tilted contact positions. This matters for long buses on slopes.
    for (int32 Pass = 0; Pass < 3; ++Pass)
    {
        TArray<TMOPGroundFit::Point> Samples;
        TArray<double> GroundZ;
        for (const auto& Contact : Contacts)
        {
            if (Contact.RadiusCm <= 0 || Contact.LocalCenter.ContainsNaN()) { Out.Error = TEXT("Invalid wheel radius/centre"); return false; }
            const FVector Local = Contact.LocalCenter * Scale;
            const FVector Center = Requested.GetLocation() + Tilt.RotateVector(Local);
            const double Radius = Contact.RadiusCm * Scale.GetAbsMax();
            FHitResult Hit;
            if (!TraceGround(Center, Center.Z-Radius, Hit)) { Out.Error = TEXT("Missing static road collision under a wheel"); return false; }
            const FVector Horizontal = Heading.UnrotateVector(Center - Requested.GetLocation());
            // Remove unequal rest heights/radii before estimating body tilt.
            Samples.Add({ Horizontal.X, Horizontal.Y, Hit.ImpactPoint.Z + Radius - Local.Z });
            GroundZ.Add(Hit.ImpactPoint.Z + Radius + TireClearanceCm);
        }
        TMOPGroundFit::Plane Plane;
        if (!TMOPGroundFit::Fit(Samples.GetData(), Samples.Num(), Plane)) { Out.Error = TEXT("Degenerate wheel layout"); return false; }
        const double Slope = FMath::RadiansToDegrees(FMath::Atan(FMath::Sqrt(Plane.X*Plane.X + Plane.Y*Plane.Y)));
        if (Slope > MaxSlopeDegrees) { Out.Error = TEXT("Ground exceeds maximum slope"); return false; }
        const FVector Normal = Heading.RotateVector(FVector(-Plane.X, -Plane.Y, 1).GetSafeNormal());
        // Preserve the requested horizontal heading while making forward tangent to the support plane.
        const FVector Forward = Heading.RotateVector(FVector(1, 0, Plane.X)).GetSafeNormal();
        Tilt = FRotationMatrix::MakeFromXZ(Forward, Normal).ToQuat();
        if (Pass == 2)
        {
            const FQuat TargetTilt = Tilt;
            if (bHaveCache && DeltaTime > 0 && FVector::DistSquared2D(Requested.GetLocation(), LastApplied.GetLocation()) < FMath::Square(500.0))
            {
                FRotator Previous = LastApplied.Rotator(); Previous.Yaw = Requested.Rotator().Yaw;
                Tilt = FQuat::Slerp(Previous.Quaternion(), Tilt, 1-FMath::Exp(-TiltResponse*DeltaTime)).GetNormalized();
            }
            Out.bSettled = Tilt.AngularDistance(TargetTilt) < FMath::DegreesToRadians(0.1);
            // Recheck actual wheel XY after smoothing; never reuse the opposite side of a kerb.
            for (int32 I = 0; I < Contacts.Num(); ++I)
            {
                const FVector Center = Requested.GetLocation() + Tilt.RotateVector(Contacts[I].LocalCenter * Scale);
                const double Radius = Contacts[I].RadiusCm * Scale.GetAbsMax();
                FHitResult Hit;
                if (!TraceGround(Center, Center.Z-Radius, Hit)) { Out.Error = TEXT("Missing road at final wheel position"); return false; }
                GroundZ[I] = Hit.ImpactPoint.Z + Radius + TireClearanceCm;
            }
            double Height = 0;
            for (int32 I = 0; I < Contacts.Num(); ++I)
                Height += GroundZ[I] - Tilt.RotateVector(Contacts[I].LocalCenter * Scale).Z;
            Height /= Contacts.Num();
            if (FMath::Abs(Height - Requested.GetLocation().Z) > MaxCorrectionCm) { Out.Error = TEXT("Required height correction exceeds limit"); return false; }
            FVector Location = Requested.GetLocation(); Location.Z = Height;
            Out.Pose = FTransform(Tilt, Location, Scale);
            for (int32 I = 0; I < Contacts.Num(); ++I)
            {
                FVector Center = Out.Pose.TransformPosition(Contacts[I].LocalCenter);
                const double Travel = GroundZ[I] - Center.Z;
                if (FMath::Abs(Travel) > SuspensionTravelCm) { Out.Error = TEXT("Required wheel travel exceeds suspension limit"); return false; }
                Center.Z = GroundZ[I];
                Out.WheelCenters.Add(Center); Out.WheelNames.Add(Contacts[I].WheelComponentName);
            }
        }
    }
    return true;
}

void UTMOPVehicleGroundingComponent::ApplyWheels(const FTMOPGroundSolution& Solution)
{
    TArray<USceneComponent*> Components; GetOwner()->GetComponents(Components);
    for (int32 I = 0; I < Solution.WheelNames.Num(); ++I)
        if (!Solution.WheelNames[I].IsNone())
            for (auto* Part : Components)
                if (Part->GetFName() == Solution.WheelNames[I]) { Part->SetWorldLocation(Solution.WheelCenters[I]); break; }
}

void UTMOPVehicleGroundingComponent::AcceptSupportedPose(const FTMOPGroundSolution& Solution)
{
    if (!GetOwner()->GetActorTransform().Equals(Solution.Pose, 0.001)) return;
    LastRequested = LastApplied = Solution.Pose; Cached = Solution; bHaveCache = true;
    AcceptedFrame = GFrameCounter;
    bGroundValid = true; bUsingApproximateFootprint = Solution.bApproximate;
    GroundStatus = Solution.bApproximate ? TEXT("Approximate footprint: configure wheels") : TEXT("Wheel contacts valid");
    ApplyWheels(Solution);
}

void UTMOPVehicleGroundingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
    Super::TickComponent(DeltaTime, TickType, TickFunction);
    UpdateGroundContact(DeltaTime);
}

void UTMOPVehicleGroundingComponent::UpdateGroundContact(float DeltaTime)
{
    if (!bEnabled || GetOwner()->IsHidden() || GetOwner()->ActorHasTag(TEXT("TMOP_AuthoritativeHistory"))) return;
    const FTransform Requested = GetOwner()->GetActorTransform();
    if (bHaveCache && AcceptedFrame == GFrameCounter && Requested.Equals(LastApplied, 0.001))
    { ApplyWheels(Cached); return; }
    const bool bSamePosition = bHaveCache && Requested.GetLocation().Equals(LastApplied.GetLocation(), 0.01) &&
        Requested.GetRotation().Equals(LastApplied.GetRotation(), 0.00001) && Requested.GetScale3D().Equals(LastApplied.GetScale3D());
    // A lane may reapply the same uncorrected transform every frame while stopped.
    if (bGroundValid && Cached.bSettled && (bSamePosition || (bHaveCache && Requested.Equals(LastRequested, 0.00001))))
    {
        GetOwner()->SetActorTransform(LastApplied, false, nullptr, ETeleportType::TeleportPhysics);
        ApplyWheels(Cached);
        return;
    }
    FTMOPGroundSolution Solution;
    if (!Solve(Requested, Solution, DeltaTime))
    {
        bGroundValid = false; GroundStatus = Solution.Error;
        if (GetWorld()->GetTimeSeconds() - LastFailureLogTime > 5)
        {
            UE_LOG(LogTemp, Warning, TEXT("TMOP ground %s: %s"), *GetOwner()->GetName(), *GroundStatus);
            LastFailureLogTime = GetWorld()->GetTimeSeconds();
        }
        return;
    }
    LastRequested = Requested; LastApplied = Solution.Pose; Cached = Solution; bHaveCache = true;
    AcceptedFrame = GFrameCounter;
    bGroundValid = true; bUsingApproximateFootprint = Solution.bApproximate;
    GroundStatus = Solution.bApproximate ? TEXT("Approximate footprint: configure wheel contacts for exact placement") : TEXT("Wheel contacts valid");
    GetOwner()->SetActorTransform(Solution.Pose, false, nullptr, ETeleportType::TeleportPhysics);
    ApplyWheels(Solution);
}

void UTMOPVehicleGroundingComponent::ValidateGroundContact()
{
    FTMOPGroundSolution Solution;
    bGroundValid = Solve(GetOwner()->GetActorTransform(), Solution);
    bUsingApproximateFootprint = Solution.bApproximate;
    GroundStatus = bGroundValid ? (Solution.bApproximate ? TEXT("Approximate contacts: configure wheels") : TEXT("Wheel contacts valid")) : Solution.Error;
    UE_LOG(LogTemp, Display, TEXT("TMOP ground %s: %s"), *GetOwner()->GetName(), *GroundStatus);
}

void UTMOPVehicleGroundingComponent::SnapToGround()
{
    InvalidateGroundCache();
    if (GetOwner()) GetOwner()->Modify();
    UpdateGroundContact(0);
}
