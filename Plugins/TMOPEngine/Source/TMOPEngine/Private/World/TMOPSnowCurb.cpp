#include "World/TMOPSnowCurb.h"

#include "Components/SplineComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ATMOPSnowCurb::ATMOPSnowCurb()
{
    PrimaryActorTick.bCanEverTick = false;
    CurbSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CurbSpline"));
    SetRootComponent(CurbSpline);
    CurbSpline->SetClosedLoop(false);
}

void ATMOPSnowCurb::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (bGenerateInEditor)
    {
        RebuildSnow();
    }
}

void ATMOPSnowCurb::ClearSnow()
{
    for (UHierarchicalInstancedStaticMeshComponent* Component : SnowHISMComponents)
    {
        if (IsValid(Component))
        {
            Component->ClearInstances();
        }
    }
}

void ATMOPSnowCurb::RebuildSnow()
{
    if (!CurbSpline) return;

    // Reuse components across construction reruns; no separate actors per clump.
    while (SnowHISMComponents.Num() < SnowMeshes.Num())
    {
        const int32 Index = SnowHISMComponents.Num();
        const FName ComponentName(*FString::Printf(TEXT("SnowHISM_%d"), Index));
        UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, ComponentName, RF_Transactional);
        HISM->SetupAttachment(CurbSpline);
        HISM->SetMobility(EComponentMobility::Static);
        HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HISM->SetCanEverAffectNavigation(false);
        HISM->SetCastShadow(false);
        HISM->SetCullDistances(0, FMath::Max(0, FMath::RoundToInt(EndCullDistanceCm)));
        AddInstanceComponent(HISM);
        HISM->RegisterComponent();
        SnowHISMComponents.Add(HISM);
    }

    for (int32 Index = 0; Index < SnowHISMComponents.Num(); ++Index)
    {
        UHierarchicalInstancedStaticMeshComponent* HISM = SnowHISMComponents[Index];
        if (!IsValid(HISM)) continue;
        HISM->ClearInstances();
        const bool bValidMesh = SnowMeshes.IsValidIndex(Index) && IsValid(SnowMeshes[Index]);
        HISM->SetStaticMesh(bValidMesh ? SnowMeshes[Index] : nullptr);
        HISM->SetVisibility(bValidMesh);
        HISM->SetCullDistances(0, FMath::Max(0, FMath::RoundToInt(EndCullDistanceCm)));
    }

    const float Length = CurbSpline->GetSplineLength();
    const float Step = FMath::Max(10.0f, SpacingCm);
    if (Length < 1.0f) return;

    TArray<int32> ValidIndices;
    for (int32 Index = 0; Index < SnowMeshes.Num(); ++Index)
    {
        if (IsValid(SnowMeshes[Index]) && SnowHISMComponents.IsValidIndex(Index) && IsValid(SnowHISMComponents[Index]))
            ValidIndices.Add(Index);
    }
    if (ValidIndices.IsEmpty()) return;

    FRandomStream Random(RandomSeed);
    const float MinScale = FMath::Max(0.01f, FMath::Min(UniformScaleRange.X, UniformScaleRange.Y));
    const float MaxScale = FMath::Max(MinScale, FMath::Max(UniformScaleRange.X, UniformScaleRange.Y));
    const int32 Count = FMath::FloorToInt(Length / Step) + 1;

    for (int32 I = 0; I < Count; ++I)
    {
        const float Distance = FMath::Clamp(I * Step + Random.FRandRange(-AlongJitterCm, AlongJitterCm), 0.0f, Length);
        const FVector Position = CurbSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
        const FVector Tangent = CurbSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
        const FVector Offset = Side * Random.FRandRange(-SideJitterCm, SideJitterCm) + FVector::UpVector * HeightOffsetCm;
        const float Yaw = Tangent.Rotation().Yaw + MeshYawOffsetDegrees + Random.FRandRange(-YawJitterDegrees, YawJitterDegrees);
        const float Scale = Random.FRandRange(MinScale, MaxScale);
        const FTransform InstanceTransform(FRotator(0.0f, Yaw, 0.0f), Position + Offset, FVector(Scale));
        const int32 MeshIndex = ValidIndices[Random.RandRange(0, ValidIndices.Num() - 1)];
        SnowHISMComponents[MeshIndex]->AddInstance(InstanceTransform);
    }
}
