#include "World/TMOPSnowCurb.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"

ATMOPSnowCurb::ATMOPSnowCurb()
{
    PrimaryActorTick.bCanEverTick = false;

    CurbSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CurbSpline"));
    SetRootComponent(CurbSpline);
    CurbSpline->SetMobility(EComponentMobility::Static);
    CurbSpline->SetClosedLoop(false);
    CurbSpline->SetDrawDebug(true); // Also enable Viewport Show > Splines.

    // A visible ten-metre spline by default. Its points remain editable.
    CurbSpline->ClearSplinePoints(false);
    CurbSpline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local, false);
    CurbSpline->AddSplinePoint(FVector(1000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local, true);
    CurbSpline->SetSplinePointType(0, ESplinePointType::Linear, false);
    CurbSpline->SetSplinePointType(1, ESplinePointType::Linear, true);

    // Native subobjects survive reruns of construction and Blueprint reinstancing.
    SnowHISMComponents.Reserve(MaxMeshTypes);
    for (int32 Index = 0; Index < MaxMeshTypes; ++Index)
    {
        const FName SlotName(*FString::Printf(TEXT("SnowCurbNativeHISM_%02d"), Index));
        UHierarchicalInstancedStaticMeshComponent* HISM =
            CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(SlotName);

        HISM->SetupAttachment(CurbSpline);
        HISM->SetMobility(EComponentMobility::Static);
        HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HISM->SetGenerateOverlapEvents(false);
        HISM->SetCanEverAffectNavigation(false);
        HISM->SetCastShadow(false);
        HISM->SetVisibility(false);
        SnowHISMComponents.Add(HISM);
    }
}

void ATMOPSnowCurb::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (bGenerateInEditor)
    {
        RebuildSnow();
    }
    else
    {
        ClearSnow();
    }
}

void ATMOPSnowCurb::ClearSnow()
{
    GeneratedInstanceCount = 0;
    for (UHierarchicalInstancedStaticMeshComponent* HISM : SnowHISMComponents)
    {
        if (IsValid(HISM))
        {
            HISM->ClearInstances();
        }
    }
}

void ATMOPSnowCurb::RebuildSnow()
{
    GeneratedInstanceCount = 0;
    if (!IsValid(CurbSpline))
    {
        return;
    }

    // Always clear instances and unused mesh slots before rebuilding.
    const int32 SlotsToCheck = FMath::Min(SnowMeshes.Num(), MaxMeshTypes);
    const int32 EndCullDistance = FMath::Max(0, FMath::RoundToInt(EndCullDistanceCm));
    TArray<int32, TInlineAllocator<MaxMeshTypes>> ValidMeshSlots;

    for (int32 Index = 0; Index < SnowHISMComponents.Num(); ++Index)
    {
        UHierarchicalInstancedStaticMeshComponent* HISM = SnowHISMComponents[Index];
        if (!IsValid(HISM))
        {
            continue;
        }

        HISM->ClearInstances();
        UStaticMesh* Mesh = Index < SlotsToCheck ? SnowMeshes[Index].Get() : nullptr;
        const bool bHasMesh = IsValid(Mesh);
        HISM->SetStaticMesh(bHasMesh ? Mesh : nullptr);
        HISM->SetVisibility(bHasMesh);
        HISM->SetCullDistances(0, EndCullDistance);

        if (bHasMesh)
        {
            ValidMeshSlots.Add(Index);
        }
    }

    if (SnowMeshes.Num() > MaxMeshTypes)
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOPSnowCurb '%s': only the first %d SnowMeshes are supported."),
            *GetName(), MaxMeshTypes);
    }

    if (ValidMeshSlots.IsEmpty() || CurbSpline->GetNumberOfSplinePoints() < 2)
    {
        return;
    }

    CurbSpline->UpdateSpline();
    const float SplineLength = CurbSpline->GetSplineLength();
    if (SplineLength < 1.0f)
    {
        return;
    }

    const float Step = FMath::Max(10.0f, SpacingCm);
    const int32 Count = FMath::Clamp(FMath::FloorToInt(SplineLength / Step) + 1, 1, FMath::Max(1, MaxInstances));
    const float JitterAlong = FMath::Clamp(AlongJitterCm, 0.0f, Step * 0.45f);
    const float JitterSide = FMath::Max(0.0f, SideJitterCm);
    const float JitterYaw = FMath::Clamp(YawJitterDegrees, 0.0f, 180.0f);
    const float MinScale = FMath::Max(0.01f, FMath::Min(UniformScaleRange.X, UniformScaleRange.Y));
    const float MaxScale = FMath::Max(MinScale, FMath::Max(UniformScaleRange.X, UniformScaleRange.Y));
    FRandomStream Random(RandomSeed);

    // Batch instance creation per mesh instead of triggering a HISM update
    // for every individual snow clump while a spline point is being dragged.
    TArray<TArray<FTransform>> InstanceBatches;
    InstanceBatches.SetNum(MaxMeshTypes);

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const float Distance = FMath::Clamp(Index * Step + Random.FRandRange(-JitterAlong, JitterAlong),
            0.0f, SplineLength);
        // HISM and spline share a parent-local coordinate system.
        const FVector Position = CurbSpline->GetLocationAtDistanceAlongSpline(
            Distance, ESplineCoordinateSpace::Local);
        const FVector Tangent = CurbSpline->GetDirectionAtDistanceAlongSpline(
            Distance, ESplineCoordinateSpace::Local);
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();

        const FVector Offset = Side * Random.FRandRange(-JitterSide, JitterSide)
            + FVector::UpVector * HeightOffsetCm;
        const float Yaw = Tangent.Rotation().Yaw + MeshYawOffsetDegrees
            + Random.FRandRange(-JitterYaw, JitterYaw);
        const float Scale = Random.FRandRange(MinScale, MaxScale);
        const int32 Slot = ValidMeshSlots[Random.RandRange(0, ValidMeshSlots.Num() - 1)];

        InstanceBatches[Slot].Add(FTransform(FRotator(0.0f, Yaw, 0.0f),
            Position + Offset, FVector(Scale)));
        ++GeneratedInstanceCount;
    }

    for (int32 Slot : ValidMeshSlots)
    {
        if (!InstanceBatches[Slot].IsEmpty())
        {
            // Local-space transforms; collision/navmesh are disabled on all HISMs.
            SnowHISMComponents[Slot]->AddInstances(InstanceBatches[Slot], false, false, false);
        }
    }
}
