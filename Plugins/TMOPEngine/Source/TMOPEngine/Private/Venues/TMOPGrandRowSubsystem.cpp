#include "Venues/TMOPGrandRowSubsystem.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"

void UTMOPGrandRowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Rows.Reset();
}

void UTMOPGrandRowSubsystem::Deinitialize()
{
    Rows.Reset();
    Super::Deinitialize();
}

bool UTMOPGrandRowSubsystem::RegisterRow(UTMOPGrandRowComponent* Row)
{
    if (!IsValid(Row) || Row->RowId.IsNone()) return false;
    if (TWeakObjectPtr<UTMOPGrandRowComponent>* Existing = Rows.Find(Row->RowId))
    {
        if (Existing->IsValid())
        {
            if (Existing->Get() == Row) return true;
            UE_LOG(LogTemp, Error, TEXT("TMOP duplicate Grand RowId '%s': '%s' conflicts with '%s'."),
                *Row->RowId.ToString(), *Row->GetPathName(), *Existing->Get()->GetPathName());
            return false;
        }
        Rows.Remove(Row->RowId);
    }
    Rows.Add(Row->RowId, Row);
    return true;
}

bool UTMOPGrandRowSubsystem::UnregisterRow(UTMOPGrandRowComponent* Row)
{
    if (Row == nullptr || Row->RowId.IsNone()) return false;
    TWeakObjectPtr<UTMOPGrandRowComponent>* Existing = Rows.Find(Row->RowId);
    if (Existing == nullptr || Existing->Get() != Row) return false;
    Rows.Remove(Row->RowId);
    return true;
}

UTMOPGrandRowComponent* UTMOPGrandRowSubsystem::FindRow(const FName RowId) const
{
    const TWeakObjectPtr<UTMOPGrandRowComponent>* Found = Rows.Find(RowId);
    return Found != nullptr ? Found->Get() : nullptr;
}

UTMOPGrandRowComponent* UTMOPGrandRowSubsystem::FindRowForSeat(const FName SeatId) const
{
    for (const TPair<FName, TWeakObjectPtr<UTMOPGrandRowComponent>>& Pair : Rows)
    {
        UTMOPGrandRowComponent* Row = Pair.Value.Get();
        if (IsValid(Row) && Row->ContainsSeat(SeatId)) return Row;
    }
    return nullptr;
}

int32 UTMOPGrandRowSubsystem::DiscoverRowsInWorld()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    for (auto It = Rows.CreateIterator(); It; ++It)
    {
        if (!It.Value().IsValid()) It.RemoveCurrent();
    }
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPGrandRowComponent*> Components;
        It->GetComponents<UTMOPGrandRowComponent>(Components);
        for (UTMOPGrandRowComponent* Row : Components) RegisterRow(Row);
    }
    return GetRowCount();
}

int32 UTMOPGrandRowSubsystem::GetRowCount() const
{
    int32 Count = 0;
    for (const TPair<FName, TWeakObjectPtr<UTMOPGrandRowComponent>>& Pair : Rows)
        Count += Pair.Value.IsValid() ? 1 : 0;
    return Count;
}

FTransform UTMOPGrandRowSubsystem::ResolveSeatAisleTransform(const FName SeatId,
    ETMOPGrandAisleSide PreferredSide, bool& bFound) const
{
    UTMOPGrandRowComponent* Row = FindRowForSeat(SeatId);
    bFound = IsValid(Row);
    if (!bFound) return FTransform::Identity;
    const ETMOPGrandAisleSide Side = PreferredSide == ETMOPGrandAisleSide::Automatic
        ? Row->ChooseNearestAisle(SeatId) : PreferredSide;
    return Row->GetAisleWorldTransform(Side);
}

bool UTMOPGrandRowSubsystem::ValidateLayout(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TMap<FName, FName> SeatOwners;
    for (const TPair<FName, TWeakObjectPtr<UTMOPGrandRowComponent>>& Pair : Rows)
    {
        UTMOPGrandRowComponent* Row = Pair.Value.Get();
        if (!IsValid(Row))
        {
            OutErrors.Add(FString::Printf(TEXT("Row '%s' has an invalid component."), *Pair.Key.ToString()));
            continue;
        }
        TArray<FString> RowErrors;
        Row->ValidateRow(RowErrors);
        for (const FString& Error : RowErrors)
            OutErrors.Add(FString::Printf(TEXT("Row '%s': %s"), *Pair.Key.ToString(), *Error));
        for (const FName SeatId : Row->OrderedSeatIds)
        {
            if (const FName* Owner = SeatOwners.Find(SeatId))
                OutErrors.Add(FString::Printf(TEXT("Seat '%s' belongs to both row '%s' and '%s'."),
                    *SeatId.ToString(), *Owner->ToString(), *Row->RowId.ToString()));
            else if (!SeatId.IsNone()) SeatOwners.Add(SeatId, Row->RowId);
        }
    }
    return OutErrors.IsEmpty();
}
