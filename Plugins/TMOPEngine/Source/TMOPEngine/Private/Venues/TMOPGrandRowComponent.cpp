#include "Venues/TMOPGrandRowComponent.h"

#include "Engine/GameInstance.h"
#include "Venues/TMOPGrandRowSubsystem.h"

UTMOPGrandRowComponent::UTMOPGrandRowComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPGrandRowComponent::BeginPlay()
{
    Super::BeginPlay();
    if (UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (UTMOPGrandRowSubsystem* Rows = GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>())
        {
            Rows->RegisterRow(this);
        }
    }
}

void UTMOPGrandRowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (UTMOPGrandRowSubsystem* Rows = GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>())
        {
            Rows->UnregisterRow(this);
        }
    }
    Super::EndPlay(EndPlayReason);
}

bool UTMOPGrandRowComponent::ContainsSeat(const FName SeatId) const
{
    return !SeatId.IsNone() && OrderedSeatIds.Contains(SeatId);
}

int32 UTMOPGrandRowComponent::GetSeatIndex(const FName SeatId) const
{
    return OrderedSeatIds.IndexOfByKey(SeatId);
}

ETMOPGrandAisleSide UTMOPGrandRowComponent::ChooseNearestAisle(const FName SeatId) const
{
    const int32 Index = GetSeatIndex(SeatId);
    if (Index == INDEX_NONE || OrderedSeatIds.Num() < 2)
    {
        return ETMOPGrandAisleSide::Left;
    }
    const int32 DistanceToLeft = Index;
    const int32 DistanceToRight = OrderedSeatIds.Num() - 1 - Index;
    return DistanceToLeft <= DistanceToRight
        ? ETMOPGrandAisleSide::Left
        : ETMOPGrandAisleSide::Right;
}

FTransform UTMOPGrandRowComponent::GetAisleWorldTransform(ETMOPGrandAisleSide Side) const
{
    if (Side == ETMOPGrandAisleSide::Automatic)
    {
        Side = ETMOPGrandAisleSide::Left;
    }
    const bool bLeft = Side == ETMOPGrandAisleSide::Left;
    const FTransform Local(
        bLeft ? LeftAisleRotationOffset : RightAisleRotationOffset,
        bLeft ? LeftAisleLocalOffset : RightAisleLocalOffset,
        FVector::OneVector);
    return Local * GetComponentTransform();
}

bool UTMOPGrandRowComponent::ValidateRow(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (RowId.IsNone()) OutErrors.Add(TEXT("RowId is missing."));
    if (VenueId.IsNone()) OutErrors.Add(TEXT("VenueId is missing."));
    if (AuditoriumId.IsNone()) OutErrors.Add(TEXT("AuditoriumId is missing."));
    if (OrderedSeatIds.IsEmpty()) OutErrors.Add(TEXT("OrderedSeatIds is empty."));

    TSet<FName> UniqueIds;
    for (const FName SeatId : OrderedSeatIds)
    {
        if (SeatId.IsNone())
        {
            OutErrors.Add(TEXT("OrderedSeatIds contains an empty SeatId."));
        }
        else if (UniqueIds.Contains(SeatId))
        {
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' occurs more than once in row '%s'."),
                *SeatId.ToString(), *RowId.ToString()));
        }
        UniqueIds.Add(SeatId);
    }
    return OutErrors.IsEmpty();
}
