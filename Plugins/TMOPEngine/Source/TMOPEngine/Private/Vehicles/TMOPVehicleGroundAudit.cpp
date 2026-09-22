#include "Vehicles/TMOPVehicleGroundAudit.h"
#include "Vehicles/TMOPVehicleGroundingComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

void ATMOPVehicleGroundAudit::ValidateLanesAndAnchors()
{
    CheckedSamples = FailedSamples = 0; Findings.Reset();
    if (!IsValid(ReferenceVehicle) || !ReferenceVehicle->Grounding || !GetWorld())
    { Findings.Add(TEXT("Choose a reference vehicle with Grounding first.")); return; }
    auto* Ground = ReferenceVehicle->Grounding.Get();
    if (Ground->RequiredGroundTag.IsNone()) Findings.Add(TEXT("Ground tag is empty: static upward surfaces are eligible. Tag roads for strict filtering."));
    bool bReportedApproximation = false;
    auto Check = [&](FTransform Pose, const FString& Label)
    {
        ++CheckedSamples;
        Pose.SetScale3D(ReferenceVehicle->GetActorScale3D());
        FTMOPGroundSolution Solution;
        if (!Ground->Solve(Pose, Solution))
        {
            ++FailedSamples;
            Findings.Add(Label + TEXT(": ") + Solution.Error);
            DrawDebugSphere(GetWorld(), Pose.GetLocation(), 30, 12, FColor::Red, false, 30);
        }
        else
        {
            const double Correction = Solution.Pose.GetLocation().Z - Pose.GetLocation().Z;
            if (FMath::Abs(Correction) > WarnCorrectionCm)
                Findings.Add(Label + FString::Printf(TEXT(": height correction %.1f cm"), Correction));
            if (Solution.bApproximate && !bReportedApproximation)
            {
                bReportedApproximation = true;
                Findings.Add(TEXT("Reference vehicle uses approximate contacts. Configure its wheel centres/radii before final validation."));
            }
        }
    };
    if (bCheckAllLanes)
    {
        const auto* Movement = ReferenceVehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        const FTransform Offset = Movement ? FTransform(Movement->VehicleRotationOffset, Movement->VehicleLocalOffset, FVector::OneVector) : FTransform::Identity;
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Lanes; It->GetComponents(Lanes);
            for (auto* Lane : Lanes)
            {
                const float Length = Lane->GetSplineLength();
                const int32 Steps = FMath::Max(1, FMath::CeilToInt(Length/FMath::Max(25.0f, SampleSpacingCm)));
                for (int32 I = 0; I <= Steps; ++I)
                {
                    const float Distance = Length*I/Steps;
                    Check(Offset * Lane->GetLaneTransformAtDistance(Distance), FString::Printf(TEXT("Lane %s at %.0f cm"), *Lane->LaneId.ToString(), Distance));
                }
            }
        }
    }
    for (const auto& Anchor : ParkingAnchors)
        if (IsValid(Anchor.Get())) Check(AnchorLocalOffset * FTransform(Anchor->GetAnchorRotation(), Anchor->GetAnchorLocation()), Anchor->GetName());
    UE_LOG(LogTemp, Display, TEXT("TMOP ground audit: %d samples, %d failures, %d findings"), CheckedSamples, FailedSamples, Findings.Num());
    for (int32 I = 0; I < FMath::Min(Findings.Num(), 200); ++I) UE_LOG(LogTemp, Warning, TEXT("%s"), *Findings[I]);
}
