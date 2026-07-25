#include "Vehicles/TMOPHistoricalVehicleDirector.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
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
    bRespectRowSpawnFlags = false;
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
    ValidateHistoricalVehicleTable(Errors);
    for (const FString& Error : Errors)
        UE_LOG(LogTemp, Error, TEXT("TMOP Historical Vehicles: %s"), *Error);
    if (!IsValid(HistoricalVehicleTable) ||
        HistoricalVehicleTable->GetRowStruct() !=
            FTMOPHistoricalVehicleRow::StaticStruct())
        return 0;

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
        if (Row->VehicleId.IsNone() ||
            !Row->Timeline.ContainsByPredicate(
                [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
                {
                    return Entry.Action ==
                            ETMOPHistoricalVehicleAction::InitialPlacement ||
                        Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
                }))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Historical Vehicles: skipped invalid row '%s'."),
                *Pair.Key.ToString());
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
    // DT_TMOP_HistoricalVehicles is the authoritative scenario inventory.
    // Every valid row is present at scenario start; bSpawnInSimulation remains
    // metadata for older tables but no longer suppresses historical cars.
    return SpawnVehicles(true);
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
            if (Entry.PlacementMode ==
                ETMOPHistoricalVehiclePlacementMode::Anchor)
            {
                UTMOPAnchorSubsystem* Anchors =
                    GetGameInstance() != nullptr
                    ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>()
                    : nullptr;
                if (Anchors != nullptr)
                {
                    // Vehicle and anchor BeginPlay order is not guaranteed.
                    Anchors->DiscoverAnchorsInWorld();
                }
                ATMOPHistoricalAnchor* Anchor =
                    Anchors != nullptr
                    ? Anchors->FindAnchor(Entry.PlacementAnchorId)
                    : nullptr;
                if (IsValid(Anchor))
                {
                    return Entry.AnchorLocalOffset *
                        FTransform(
                            Anchor->GetAnchorRotation(),
                            Anchor->GetAnchorLocation(),
                            FVector::OneVector);
                }
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP Historical Vehicles: placement anchor '%s' for vehicle '%s' was not found; using World Transform fallback."),
                    *Entry.PlacementAnchorId.ToString(),
                    *Profile.VehicleId.ToString());
            }
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
        for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Row->Timeline)
        {
            const bool bPlacementEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
            if (bPlacementEntry &&
                Entry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor &&
                Entry.PlacementAnchorId.IsNone())
            {
                OutErrors.Add(Prefix +
                    TEXT(" has anchor placement without a Placement Anchor ID."));
            }
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

const FTMOPHistoricalVehicleTimelineEntry*
ATMOPHistoricalVehicleDirector::FindDrivingEntry(
    const FTMOPHistoricalVehicleRow& Profile,
    const FName DriverEntityId) const
{
    for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Profile.Timeline)
    {
        const bool bDrivingAction =
            Entry.Action == ETMOPHistoricalVehicleAction::BeginDriving ||
            Entry.Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
        const bool bDriverMatches = Entry.DriverEntityId.IsNone() ||
            Entry.DriverEntityId == DriverEntityId;
        if (bDrivingAction && bDriverMatches &&
            !Entry.OrderedLaneIds.IsEmpty())
            return &Entry;
    }
    return nullptr;
}

bool ATMOPHistoricalVehicleDirector::BeginDrivingVehicle(
    const FName VehicleId,
    const FName DriverEntityId,
    const TArray<FName>& OrderedLaneIds,
    const ETMOPVehicleRouteMode RouteMode,
    const FName DestinationAnchorId,
    const float StartDistanceAlongFirstLaneCm)
{
    if (RuntimeVehicles.IsEmpty())
        InitializeHistoricalVehicles();

    FHistoricalVehicleRuntime* Runtime = RuntimeVehicles.Find(VehicleId);
    ATMOPVehicleBase* Vehicle =
        Runtime != nullptr ? Runtime->Vehicle.Get() : nullptr;
    if (Runtime == nullptr || !IsValid(Vehicle))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP driving: vehicle '%s' is not spawned."),
            *VehicleId.ToString());
        return false;
    }

    ATMOPHistoricalAgent* Driver = Vehicle->GetDriverAgent();
    const FName OccupantId =
        IsValid(Driver) && IsValid(Driver->EntityIdentity)
        ? Driver->EntityIdentity->EntityId : NAME_None;
    if (DriverEntityId.IsNone() || OccupantId != DriverEntityId)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP driving: '%s' is not in driver seat of '%s'."),
            *DriverEntityId.ToString(), *VehicleId.ToString());
        return false;
    }
    if (!Runtime->Profile.KnownDriverEntityId.IsNone() &&
        Runtime->Profile.KnownDriverEntityId != DriverEntityId)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP driving: '%s' does not match known driver '%s' for '%s'."),
            *DriverEntityId.ToString(),
            *Runtime->Profile.KnownDriverEntityId.ToString(),
            *VehicleId.ToString());
        return false;
    }

    TArray<FName> Route = OrderedLaneIds;
    if (RouteMode == ETMOPVehicleRouteMode::ManualLaneRoute &&
        Route.IsEmpty())
        if (const FTMOPHistoricalVehicleTimelineEntry* DrivingEntry =
            FindDrivingEntry(Runtime->Profile, DriverEntityId))
            Route = DrivingEntry->OrderedLaneIds;
    Route.RemoveAll([](const FName LaneId) { return LaneId.IsNone(); });

    float ResolvedStartDistance = StartDistanceAlongFirstLaneCm;
    if (RouteMode != ETMOPVehicleRouteMode::ManualLaneRoute)
    {
        if (DestinationAnchorId.IsNone() || GetGameInstance() == nullptr)
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP driving: automatic route for '%s' needs a Destination Anchor ID."),
                *VehicleId.ToString());
            return false;
        }
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Destination =
            Anchors != nullptr ? Anchors->FindAnchor(DestinationAnchorId) : nullptr;
        UTMOPTrafficNetworkSubsystem* Network =
            GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>();
        if (!IsValid(Destination) || Network == nullptr)
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP driving: destination anchor '%s' or traffic network is unavailable."),
                *DestinationAnchorId.ToString());
            return false;
        }
        Network->DiscoverLanesInWorld();

        FName DestinationLaneId;
        float DestinationDistance = 0.0f;
        if (!Network->FindNearestLane(
            Destination->GetAnchorLocation(),
            DestinationLaneId,
            DestinationDistance))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP driving: no lane was found near anchor '%s'."),
                *DestinationAnchorId.ToString());
            return false;
        }

        FName RouteStartLaneId;
        if (RouteMode == ETMOPVehicleRouteMode::ManualThenAutomatic &&
            !Route.IsEmpty())
        {
            RouteStartLaneId = Route.Last();
        }
        else
        {
            float NearestStartDistance = 0.0f;
            if (!Network->FindNearestLane(
                Vehicle->GetActorLocation(),
                RouteStartLaneId,
                NearestStartDistance))
            {
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP driving: no start lane was found near '%s'."),
                    *VehicleId.ToString());
                return false;
            }
            Route.Reset();
            if (StartDistanceAlongFirstLaneCm <= 0.0f)
            {
                ResolvedStartDistance = NearestStartDistance;
            }
        }

        TArray<FName> AutomaticTail;
        if (!Network->FindLaneRoute(
            RouteStartLaneId, DestinationLaneId, AutomaticTail))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP driving: no connected lane route from '%s' to '%s' (anchor '%s')."),
                *RouteStartLaneId.ToString(), *DestinationLaneId.ToString(),
                *DestinationAnchorId.ToString());
            return false;
        }
        if (!Route.IsEmpty() && !AutomaticTail.IsEmpty() &&
            Route.Last() == AutomaticTail[0])
        {
            AutomaticTail.RemoveAt(0);
        }
        Route.Append(AutomaticTail);
        UE_LOG(LogTemp, Display,
            TEXT("TMOP driving: calculated %d lane(s) to anchor '%s'."),
            Route.Num(), *DestinationAnchorId.ToString());
    }

    if (Route.IsEmpty())
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP driving: vehicle '%s' has no ordered lane route."),
            *VehicleId.ToString());
        return false;
    }

    UTMOPTrafficVehicleMovementComponent* Movement =
        Vehicle->FindComponentByClass<
            UTMOPTrafficVehicleMovementComponent>();
    if (!IsValid(Movement))
    {
        Movement = NewObject<UTMOPTrafficVehicleMovementComponent>(
            Vehicle, TEXT("HistoricalTrafficMovement"));
        Vehicle->AddInstanceComponent(Movement);
        Movement->bStartDrivingAutomatically = false;
        Movement->RegisterComponent();
    }

    Movement->StopDriving();
    Movement->PlannedLaneIds = Route;
    Movement->InitialLaneId = Route[0];
    if (!Movement->InitializeOnLane(
        Route[0], ResolvedStartDistance))
        return false;
    Movement->StartDriving();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP driving: '%s' started '%s' on %d ordered lane(s)."),
        *DriverEntityId.ToString(), *VehicleId.ToString(), Route.Num());
    return true;
}
