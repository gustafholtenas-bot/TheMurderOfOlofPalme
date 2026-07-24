#include "Vehicles/TMOPHistoricalVehicleDirector.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "World/TMOPWorldSubsystem.h"

namespace
{
    const FName HistoricalVehicleObjectType(TEXT("HistoricalVehicle"));
}

ATMOPHistoricalVehicleDirector::ATMOPHistoricalVehicleDirector()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultVehicleClass = ATMOPConfiguredVehicle::StaticClass();
}

void ATMOPHistoricalVehicleDirector::BeginPlay()
{
    Super::BeginPlay();
    InitializeHistoricalVehicles();
}

void ATMOPHistoricalVehicleDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        if (ATMOPVehicleBase* Vehicle = Pair.Value.Vehicle.Get())
        {
            UnregisterVehicle(Vehicle);
        }
    }
    RuntimeVehicles.Reset();
    Super::EndPlay(EndPlayReason);
}

int32 ATMOPHistoricalVehicleDirector::InitializeHistoricalVehicles()
{
    RuntimeVehicles.Reset();

    TArray<FString> Errors;
    if (!ValidateHistoricalVehicleTable(Errors))
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP Historical Vehicles: %s"), *Error);
        }
        return 0;
    }

    const TMap<FName, uint8*>& Rows = HistoricalVehicleTable->GetRowMap();
    RuntimeVehicles.Reserve(Rows.Num());
    for (const TPair<FName, uint8*>& Pair : Rows)
    {
        const FTMOPHistoricalVehicleRow* Row =
            reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
        if (Row == nullptr)
        {
            continue;
        }

        FHistoricalVehicleRuntime Runtime;
        Runtime.RowName = Pair.Key;
        Runtime.Profile = *Row;
        RuntimeVehicles.Add(Row->VehicleId, MoveTemp(Runtime));
    }

    if (bReusePlacedVehicles)
    {
        DiscoverPlacedVehicles();
    }

    return bSpawnVehiclesAutomatically ? SpawnEnabledVehicles() : RuntimeVehicles.Num();
}

int32 ATMOPHistoricalVehicleDirector::SpawnEnabledVehicles()
{
    return SpawnVehicles(false);
}

void ATMOPHistoricalVehicleDirector::SpawnAllVehiclesForStaging()
{
    if (RuntimeVehicles.IsEmpty())
    {
        const bool bWasAutomatic = bSpawnVehiclesAutomatically;
        bSpawnVehiclesAutomatically = false;
        InitializeHistoricalVehicles();
        bSpawnVehiclesAutomatically = bWasAutomatic;
    }
    const int32 SpawnedCount = SpawnVehicles(true);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP Historical Vehicles: %d staging vehicles are available."),
        SpawnedCount);
}

int32 ATMOPHistoricalVehicleDirector::SpawnVehicles(const bool bIgnoreRowFlags)
{
    int32 AvailableCount = 0;
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (Runtime.Vehicle.IsValid())
        {
            ++AvailableCount;
            continue;
        }
        if (!ShouldSpawn(Runtime.Profile, bIgnoreRowFlags))
        {
            continue;
        }
        if (SpawnVehicle(Runtime) != nullptr)
        {
            ++AvailableCount;
        }
    }
    return AvailableCount;
}

void ATMOPHistoricalVehicleDirector::DiscoverPlacedVehicles()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
    {
        ATMOPVehicleBase* Vehicle = *It;
        if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone())
        {
            continue;
        }

        FHistoricalVehicleRuntime* Runtime = RuntimeVehicles.Find(Vehicle->VehicleId);
        if (Runtime == nullptr)
        {
            continue;
        }
        if (Runtime->Vehicle.IsValid() && Runtime->Vehicle.Get() != Vehicle)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Historical Vehicles: duplicate placed VehicleId '%s'."),
                *Vehicle->VehicleId.ToString());
            continue;
        }

        Runtime->Vehicle = Vehicle;
        Runtime->bSpawnedByDirector = false;
        RegisterVehicle(Vehicle);
    }
}

ATMOPVehicleBase* ATMOPHistoricalVehicleDirector::SpawnVehicle(
    FHistoricalVehicleRuntime& Runtime)
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return nullptr;
    }

    TSubclassOf<ATMOPVehicleBase> SpawnClass = DefaultVehicleClass;
    if (UClass* RowClass = Runtime.Profile.VehicleClass.Get())
    {
        if (RowClass->IsChildOf(ATMOPVehicleBase::StaticClass()))
        {
            SpawnClass = RowClass;
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Historical Vehicles: class on '%s' is not a TMOPVehicleBase."),
                *Runtime.Profile.VehicleId.ToString());
        }
    }
    if (SpawnClass.Get() == nullptr)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP Historical Vehicles: '%s' has no VehicleClass and no default."),
            *Runtime.Profile.VehicleId.ToString());
        return nullptr;
    }

    const FTransform InitialTransform = GetInitialTransform(Runtime.Profile);
    ATMOPVehicleBase* Vehicle = World->SpawnActorDeferred<ATMOPVehicleBase>(
        SpawnClass, InitialTransform, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if (!IsValid(Vehicle))
    {
        return nullptr;
    }

    Vehicle->VehicleId = Runtime.Profile.VehicleId;
    if (ATMOPConfiguredVehicle* Configured = Cast<ATMOPConfiguredVehicle>(Vehicle))
    {
        Configured->VehicleModel = Runtime.Profile.ModelData;
    }
    UGameplayStatics::FinishSpawningActor(Vehicle, InitialTransform);

    Runtime.Vehicle = Vehicle;
    Runtime.bSpawnedByDirector = true;
    RegisterVehicle(Vehicle);
    return Vehicle;
}

FTransform ATMOPHistoricalVehicleDirector::GetInitialTransform(
    const FTMOPHistoricalVehicleRow& Profile) const
{
    for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Profile.Timeline)
    {
        if (Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
            Entry.Action == ETMOPHistoricalVehicleAction::Spawn)
        {
            return Entry.WorldTransform;
        }
    }
    return FTransform::Identity;
}

bool ATMOPHistoricalVehicleDirector::ShouldSpawn(
    const FTMOPHistoricalVehicleRow& Profile,
    const bool bIgnoreRowFlag) const
{
    if (!bIgnoreRowFlag && bRespectRowSpawnFlags && !Profile.bSpawnInSimulation)
    {
        return false;
    }
    return Profile.Timeline.ContainsByPredicate(
        [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
        {
            return Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
        });
}

void ATMOPHistoricalVehicleDirector::RegisterVehicle(ATMOPVehicleBase* Vehicle) const
{
    if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone() || GetGameInstance() == nullptr)
    {
        return;
    }
    if (UTMOPWorldSubsystem* Registry =
        GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
    {
        Registry->RegisterWorldObject(
            Vehicle->VehicleId, HistoricalVehicleObjectType, Vehicle);
    }
}

void ATMOPHistoricalVehicleDirector::UnregisterVehicle(ATMOPVehicleBase* Vehicle) const
{
    if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone() || GetGameInstance() == nullptr)
    {
        return;
    }
    if (UTMOPWorldSubsystem* Registry =
        GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
    {
        Registry->UnregisterWorldObject(Vehicle->VehicleId, Vehicle);
    }
}

bool ATMOPHistoricalVehicleDirector::ValidateHistoricalVehicleTable(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!IsValid(HistoricalVehicleTable) ||
        HistoricalVehicleTable->GetRowStruct() !=
            FTMOPHistoricalVehicleRow::StaticStruct())
    {
        OutErrors.Add(
            TEXT("Historical Vehicle Table is missing or has the wrong row structure."));
        return false;
    }

    TSet<FName> VehicleIds;
    for (const TPair<FName, uint8*>& Pair : HistoricalVehicleTable->GetRowMap())
    {
        const FTMOPHistoricalVehicleRow* Row =
            reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
        if (Row == nullptr)
        {
            OutErrors.Add(FString::Printf(TEXT("Row '%s' is null."), *Pair.Key.ToString()));
            continue;
        }
        const FString Prefix = FString::Printf(TEXT("Row '%s'"), *Pair.Key.ToString());
        if (Row->VehicleId.IsNone())
        {
            OutErrors.Add(Prefix + TEXT(" has no VehicleId."));
        }
        else
        {
            if (Pair.Key != Row->VehicleId)
            {
                OutErrors.Add(Prefix + TEXT(" does not match its VehicleId."));
            }
            if (VehicleIds.Contains(Row->VehicleId))
            {
                OutErrors.Add(Prefix + TEXT(" duplicates another VehicleId."));
            }
            VehicleIds.Add(Row->VehicleId);
        }
        if (Row->Timeline.IsEmpty())
        {
            OutErrors.Add(Prefix + TEXT(" has no Timeline entries."));
        }
    }
    return OutErrors.IsEmpty();
}

ATMOPVehicleBase* ATMOPHistoricalVehicleDirector::FindHistoricalVehicle(
    const FName VehicleId) const
{
    const FHistoricalVehicleRuntime* Runtime = RuntimeVehicles.Find(VehicleId);
    return Runtime != nullptr ? Runtime->Vehicle.Get() : nullptr;
}
