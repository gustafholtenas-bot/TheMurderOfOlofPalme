#include "Venues/TMOPVenueFurnitureGenerator.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

struct FTMOPVenueFurnitureSeatRecord
{
    ATMOPHistoricalAnchor* Anchor = nullptr;
    FName AnchorId = NAME_None;
    FString VenueKey;
    FString GroupKey;
    bool bBarSeat = false;
};

namespace
{
    const FName GeneratedFurnitureTag(TEXT("TMOP_GENERATED_VENUE_FURNITURE"));
    const FString FurnitureKeyPrefix(TEXT("TMOP_FURNITURE_KEY_"));

    FString SanitizeFurnitureKey(FString Value)
    {
        Value.ReplaceInline(TEXT(" "), TEXT("_"));
        Value.ReplaceInline(TEXT("/"), TEXT("_"));
        Value.ReplaceInline(TEXT("\\"), TEXT("_"));
        Value.ReplaceInline(TEXT("."), TEXT("_"));
        return Value;
    }

    FName MakeFurnitureKey(const FString& Value)
    {
        return FName(*(FurnitureKeyPrefix + SanitizeFurnitureKey(Value)));
    }
}

ATMOPVenueFurnitureGenerator::ATMOPVenueFurnitureGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

bool ATMOPVenueFurnitureGenerator::IsGrandAnchor(
    const ATMOPHistoricalAnchor* Anchor) const
{
    if (!IsValid(Anchor)) return true;
    const FString Id = Anchor->GetAnchorId().ToString();
    const FString Parent = Anchor->ParentAnchorId.ToString();
    FString Label = Anchor->GetName();
#if WITH_EDITOR
    Label = Anchor->GetActorLabel();
#endif
    return Anchor->AnchorCategory == ETMOPAnchorCategory::CinemaSeat ||
        Id.Contains(TEXT("grand"), ESearchCase::IgnoreCase) ||
        Parent.Contains(TEXT("grand"), ESearchCase::IgnoreCase) ||
        Label.Contains(TEXT("grand"), ESearchCase::IgnoreCase);
}

bool ATMOPVenueFurnitureGenerator::ParseSeatAnchor(
    ATMOPHistoricalAnchor* Anchor,
    FTMOPVenueFurnitureSeatRecord& OutRecord) const
{
    if (!IsValid(Anchor)) return false;
    const FString Id = Anchor->GetAnchorId().ToString();
    const FString Lower = Id.ToLower();

    const FString BarMarker(TEXT("_bar_seat_"));
    const int32 BarIndex = Lower.Find(BarMarker);
    if (BarIndex != INDEX_NONE)
    {
        OutRecord.Anchor = Anchor;
        OutRecord.AnchorId = Anchor->GetAnchorId();
        OutRecord.VenueKey = Id.Left(BarIndex);
        OutRecord.GroupKey = OutRecord.VenueKey + TEXT("_bar");
        OutRecord.bBarSeat = true;
        return true;
    }

    const FString TableMarker(TEXT("_table_"));
    const int32 TableIndex = Lower.Find(TableMarker);
    const int32 SeatIndex = Lower.Find(TEXT("_seat_"),
        ESearchCase::CaseSensitive, ESearchDir::FromStart,
        TableIndex == INDEX_NONE ? 0 : TableIndex + TableMarker.Len());
    if (TableIndex != INDEX_NONE && SeatIndex != INDEX_NONE)
    {
        OutRecord.Anchor = Anchor;
        OutRecord.AnchorId = Anchor->GetAnchorId();
        OutRecord.VenueKey = Id.Left(TableIndex);
        OutRecord.GroupKey = Id.Left(SeatIndex);
        OutRecord.bBarSeat = false;
        return true;
    }
    return false;
}

AStaticMeshActor* ATMOPVenueFurnitureGenerator::FindGeneratedActor(
    const FName FurnitureKey) const
{
    if (UWorld* World = GetWorld())
        for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
            if (It->Tags.Contains(GeneratedFurnitureTag) &&
                It->Tags.Contains(FurnitureKey))
                return *It;
    return nullptr;
}

AStaticMeshActor* ATMOPVenueFurnitureGenerator::CreateOrUpdateMeshActor(
    const FName FurnitureKey, const FString& Label, UStaticMesh* Mesh,
    const FTransform& WorldTransform, const FString& VenueKey)
{
    if (!IsValid(Mesh) || GetWorld() == nullptr)
    {
        ++LastErrorCount;
        return nullptr;
    }

    AStaticMeshActor* Actor = FindGeneratedActor(FurnitureKey);
    if (!IsValid(Actor))
    {
        FActorSpawnParameters Params;
        Params.Name = MakeUniqueObjectName(GetWorld()->PersistentLevel,
            AStaticMeshActor::StaticClass(), FurnitureKey);
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transactional;
        Actor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), WorldTransform, Params);
    }
    if (!IsValid(Actor))
    {
        ++LastErrorCount;
        return nullptr;
    }

    Actor->Modify();
    Actor->Tags.AddUnique(GeneratedFurnitureTag);
    Actor->Tags.AddUnique(FurnitureKey);
    Actor->SetActorTransform(WorldTransform, false, nullptr,
        ETeleportType::TeleportPhysics);
    UStaticMeshComponent* Component = Actor->GetStaticMeshComponent();
    Component->SetMobility(EComponentMobility::Static);
    Component->SetStaticMesh(Mesh);
    Component->SetCollisionEnabled(bEnableFurnitureCollision
        ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Component->SetCollisionProfileName(bEnableFurnitureCollision
        ? FName(TEXT("BlockAll")) : FName(TEXT("NoCollision")));

#if WITH_EDITOR
    Actor->SetActorLabel(Label);
    Actor->SetFolderPath(*FString::Printf(
        TEXT("TMOP/VenueFurniture/%s"), *VenueKey));
#endif
    Actor->MarkPackageDirty();
    return Actor;
}

void ATMOPVenueFurnitureGenerator::GenerateOrUpdateVenueFurniture()
{
    LastSeatAnchorCount = LastChairCount = LastTableCount = 0;
    LastBarCounterCount = LastSkippedGrandCount = LastErrorCount = 0;
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        ++LastErrorCount;
        return;
    }

    TArray<FTMOPVenueFurnitureSeatRecord> Seats;
    TMap<FString, TArray<int32>> TableGroups;
    TMap<FString, TArray<int32>> BarGroups;
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        FTMOPVenueFurnitureSeatRecord Record;
        if (!ParseSeatAnchor(*It, Record)) continue;
        if (IsGrandAnchor(*It))
        {
            ++LastSkippedGrandCount;
            continue;
        }
        const int32 Index = Seats.Add(Record);
        (Record.bBarSeat ? BarGroups : TableGroups)
            .FindOrAdd(Record.GroupKey).Add(Index);
    }
    LastSeatAnchorCount = Seats.Num();

    for (const FTMOPVenueFurnitureSeatRecord& Seat : Seats)
    {
        UStaticMesh* Mesh = Seat.bBarSeat ? BarStoolMesh.Get() : ChairMesh.Get();
        const FTransform& Offset = Seat.bBarSeat
            ? BarStoolLocalOffset : ChairLocalOffset;
        const FString Kind = Seat.bBarSeat ? TEXT("BarStool") : TEXT("Chair");
        if (CreateOrUpdateMeshActor(
            MakeFurnitureKey(Kind + TEXT("_") + Seat.AnchorId.ToString()),
            TEXT("TMOP_") + Kind + TEXT("_") + Seat.AnchorId.ToString(), Mesh,
            Offset * Seat.Anchor->GetActorTransform(), Seat.VenueKey))
            ++LastChairCount;
    }

    for (const TPair<FString, TArray<int32>>& Pair : TableGroups)
    {
        if (Pair.Value.IsEmpty()) continue;
        FVector Centre = FVector::ZeroVector;
        for (const int32 Index : Pair.Value)
            Centre += Seats[Index].Anchor->GetActorLocation();
        Centre /= static_cast<float>(Pair.Value.Num());
        const float Yaw = Seats[Pair.Value[0]].Anchor->GetActorRotation().Yaw;
        const FTransform Base(FRotator(0.0f, Yaw, 0.0f), Centre);
        const FString VenueKey = Seats[Pair.Value[0]].VenueKey;
        if (CreateOrUpdateMeshActor(MakeFurnitureKey(TEXT("Table_") + Pair.Key),
            TEXT("TMOP_Table_") + Pair.Key, TableMesh.Get(),
            TableLocalOffset * Base, VenueKey))
            ++LastTableCount;
    }

    for (const TPair<FString, TArray<int32>>& Pair : BarGroups)
    {
        if (Pair.Value.IsEmpty()) continue;
        FVector Centre = FVector::ZeroVector;
        FVector AverageForward = FVector::ZeroVector;
        for (const int32 Index : Pair.Value)
        {
            const ATMOPHistoricalAnchor* Anchor = Seats[Index].Anchor;
            Centre += Anchor->GetActorLocation();
            AverageForward += Anchor->GetActorForwardVector().GetSafeNormal2D();
        }
        Centre /= static_cast<float>(Pair.Value.Num());
        AverageForward = AverageForward.GetSafeNormal2D();
        if (AverageForward.IsNearlyZero())
            AverageForward = Seats[Pair.Value[0]].Anchor
                ->GetActorForwardVector().GetSafeNormal2D();
        const FVector AlongBar(-AverageForward.Y, AverageForward.X, 0.0f);
        Centre += AverageForward * BarCounterForwardOffsetCm;

        FTransform Base(AlongBar.Rotation(), Centre);
        if (bAutoScaleBarCounterLength && IsValid(BarCounterMesh))
        {
            float Minimum = TNumericLimits<float>::Max();
            float Maximum = -TNumericLimits<float>::Max();
            for (const int32 Index : Pair.Value)
            {
                const float Along = FVector::DotProduct(
                    Seats[Index].Anchor->GetActorLocation() - Centre, AlongBar);
                Minimum = FMath::Min(Minimum, Along);
                Maximum = FMath::Max(Maximum, Along);
            }
            const float RequiredLength = FMath::Max(1.0f,
                Maximum - Minimum + BarCounterEndPaddingCm * 2.0f);
            const float MeshLength = FMath::Max(1.0f,
                BarCounterMesh->GetBounds().BoxExtent.X * 2.0f);
            FVector Scale = BarCounterLocalOffset.GetScale3D();
            Scale.X *= RequiredLength / MeshLength;
            FTransform AdjustedOffset = BarCounterLocalOffset;
            AdjustedOffset.SetScale3D(Scale);
            Base = AdjustedOffset * Base;
        }
        else
        {
            Base = BarCounterLocalOffset * Base;
        }
        const FString VenueKey = Seats[Pair.Value[0]].VenueKey;
        if (CreateOrUpdateMeshActor(MakeFurnitureKey(TEXT("BarCounter_") + Pair.Key),
            TEXT("TMOP_BarCounter_") + Pair.Key, BarCounterMesh.Get(),
            Base, VenueKey))
            ++LastBarCounterCount;
    }

    MarkPackageDirty();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP venue furniture: %d seat anchors, %d chairs/stools, %d tables, %d bar counters, %d Grand anchors skipped, %d errors."),
        LastSeatAnchorCount, LastChairCount, LastTableCount,
        LastBarCounterCount, LastSkippedGrandCount, LastErrorCount);
}
