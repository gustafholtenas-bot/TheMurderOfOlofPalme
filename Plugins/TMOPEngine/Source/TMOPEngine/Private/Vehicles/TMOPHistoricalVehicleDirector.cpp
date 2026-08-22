#include "Vehicles/TMOPHistoricalVehicleDirector.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "World/TMOPWorldSubsystem.h"

namespace
{
    const FName HistoricalVehicleObjectType(TEXT("HistoricalVehicle"));
}

ATMOPHistoricalVehicleDirector::ATMOPHistoricalVehicleDirector()
{
    PrimaryActorTick.bCanEverTick = true;
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

void ATMOPHistoricalVehicleDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock == nullptr)
    {
        return;
    }

    const int32 CurrentSecond =
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE &&
        CurrentSecond < LastEvaluatedSecond)
    {
        InitializeHistoricalVehicles();
        return;
    }
    if (CurrentSecond != LastEvaluatedSecond)
    {
        SpawnDueVehicles(CurrentSecond);
        DespawnDueVehicles(CurrentSecond);
        LastEvaluatedSecond = CurrentSecond;
    }
}

void ATMOPHistoricalVehicleDirector::DespawnDueVehicles(
    const int32 CurrentSecond)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        if (!IsValid(Vehicle)) continue;

        const FTMOPHistoricalVehicleTimelineEntry* DespawnEntry =
            Runtime.Profile.Timeline.FindByPredicate(
                [CurrentSecond](const FTMOPHistoricalVehicleTimelineEntry& Entry)
                {
                    return Entry.Action ==
                            ETMOPHistoricalVehicleAction::Despawn &&
                        Entry.Time.ToSecondsFromMidnight() <= CurrentSecond;
                });
        if (DespawnEntry == nullptr) continue;

        UE_LOG(LogTemp, Display,
            TEXT("TMOP Historical Vehicles: despawned '%s' at %02d:%02d:%02d."),
            *Runtime.Profile.VehicleId.ToString(),
            DespawnEntry->Time.Hour,
            DespawnEntry->Time.Minute,
            DespawnEntry->Time.Second);
        UnregisterVehicle(Vehicle);
        Vehicle->Destroy();
        Runtime.Vehicle.Reset();
        Runtime.bSpawnedByDirector = false;
        Runtime.bDeferredPlacedVehicle = false;
        Runtime.bTimelineDespawned = true;
    }
}

int32 ATMOPHistoricalVehicleDirector::InitializeHistoricalVehicles()
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (Runtime.bSpawnedByDirector && Runtime.Vehicle.IsValid())
        {
            UnregisterVehicle(Runtime.Vehicle.Get());
            Runtime.Vehicle->Destroy();
        }
    }
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
        // Rows with spawning disabled may intentionally be archive-only
        // records. They do not need a timeline and must not be reported as
        // invalid runtime vehicles.
        if (!Row->bSpawnInSimulation && Row->Timeline.IsEmpty())
        {
            UE_LOG(LogTemp, Verbose,
                TEXT("TMOP Historical Vehicles: archive-only row '%s' has no timeline and will not be registered for simulation."),
                *Pair.Key.ToString());
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
        Runtime.InitialSpawnSecond = GetInitialSpawnSecond(*Row);
        RuntimeVehicles.Add(Row->VehicleId, MoveTemp(Runtime));
    }

    if (bReusePlacedVehicles)
    {
        DiscoverPlacedVehicles();
    }

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    ApplyDeferredPlacedVehicleState(CurrentSecond);
    LastEvaluatedSecond = CurrentSecond;
    return bSpawnVehiclesAutomatically
        ? SpawnDueVehicles(CurrentSecond)
        : RuntimeVehicles.Num();
}

int32 ATMOPHistoricalVehicleDirector::SpawnEnabledVehicles()
{
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    return SpawnDueVehicles(CurrentSecond);
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

int32 ATMOPHistoricalVehicleDirector::SpawnDueVehicles(
    const int32 CurrentSecond)
{
    int32 AvailableCount = 0;
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (!ShouldSpawn(Runtime.Profile, false) ||
            Runtime.bTimelineDespawned ||
            Runtime.InitialSpawnSecond == INDEX_NONE ||
            Runtime.InitialSpawnSecond > CurrentSecond)
        {
            continue;
        }

        if (Runtime.bDeferredPlacedVehicle && Runtime.Vehicle.IsValid())
        {
            ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
            Vehicle->SetActorHiddenInGame(false);
            Vehicle->SetActorEnableCollision(true);
            Vehicle->SetActorTickEnabled(true);
            Runtime.bDeferredPlacedVehicle = false;
            RegisterVehicle(Vehicle);
            UE_LOG(LogTemp, Display,
                TEXT("TMOP Historical Vehicles: activated placed vehicle '%s' at scheduled spawn time."),
                *Runtime.Profile.VehicleId.ToString());
        }
        else if (!Runtime.Vehicle.IsValid())
        {
            FTransform ClearSpawnTransform;
            if (!FindClearInitialSpawnTransform(
                Runtime.Profile, ClearSpawnTransform))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("TMOP VehicleSpawnBlocked: '%s' waits because its entry point is occupied."),
                    *Runtime.Profile.VehicleId.ToString());
                continue;
            }
            SpawnVehicle(Runtime, &ClearSpawnTransform);
        }
        if (Runtime.Vehicle.IsValid())
        {
            ++AvailableCount;
        }
    }
    return AvailableCount;
}

int32 ATMOPHistoricalVehicleDirector::GetInitialSpawnSecond(
    const FTMOPHistoricalVehicleRow& Profile) const
{
    for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Profile.Timeline)
    {
        if (Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
            Entry.Action == ETMOPHistoricalVehicleAction::Spawn)
        {
            int32 SpawnSecond = Entry.Time.ToSecondsFromMidnight();
            const FString AnchorId = Entry.PlacementAnchorId.ToString();
            const bool bBoundaryEntry =
                Entry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor &&
                AnchorId.StartsWith(TEXT("Enter"),
                    ESearchCase::IgnoreCase);
            if (bBoundaryEntry)
            {
                for (const FTMOPHistoricalVehicleTimelineEntry& Later :
                    Profile.Timeline)
                {
                    if (Later.Action ==
                            ETMOPHistoricalVehicleAction::BeginDriving ||
                        Later.Action ==
                            ETMOPHistoricalVehicleAction::EnterTrafficRoute)
                    {
                        SpawnSecond = FMath::Max(
                            0,
                            Later.Time.ToSecondsFromMidnight() -
                                EntrySpawnLeadSeconds);
                        break;
                    }
                }
            }
            return SpawnSecond;
        }
    }
    return INDEX_NONE;
}

bool ATMOPHistoricalVehicleDirector::FindClearInitialSpawnTransform(
    const FTMOPHistoricalVehicleRow& Profile,
    FTransform& OutTransform) const
{
    UWorld* World = GetWorld();
    if (World == nullptr) return false;
    const FTMOPHistoricalVehicleTimelineEntry* Placement =
        Profile.Timeline.FindByPredicate(
            [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
            {
                return Entry.Action ==
                        ETMOPHistoricalVehicleAction::InitialPlacement ||
                    Entry.Action ==
                        ETMOPHistoricalVehicleAction::Spawn;
            });
    if (Placement == nullptr ||
        Placement->PlacementMode !=
            ETMOPHistoricalVehiclePlacementMode::Anchor ||
        !Placement->PlacementAnchorId.ToString().StartsWith(
            TEXT("Enter"), ESearchCase::IgnoreCase))
    {
        OutTransform = GetInitialTransform(Profile);
        return true;
    }

    const FTransform Base = GetInitialTransform(Profile);
    const FVector Backward = -Base.GetRotation().GetForwardVector();
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(
        SCENE_QUERY_STAT(TMOPVehicleEntrySpawn), false, this);

    const int32 SlotCount = FMath::Max(1, EntrySpawnQueueSlots);
    for (int32 Slot = 0; Slot < SlotCount; ++Slot)
    {
        FTransform Candidate = Base;
        Candidate.AddToTranslation(
            Backward * EntrySpawnQueueSpacingCm * Slot);
        TArray<FOverlapResult> Overlaps;
        World->OverlapMultiByObjectType(
            Overlaps, Candidate.GetLocation(), FQuat::Identity, Objects,
            FCollisionShape::MakeSphere(EntrySpawnClearanceRadiusCm), Query);
        const bool bVehicleOccupied = Overlaps.ContainsByPredicate(
            [](const FOverlapResult& Overlap)
            {
                return IsValid(Cast<ATMOPVehicleBase>(Overlap.GetActor()));
            });
        if (!bVehicleOccupied)
        {
            OutTransform = Candidate;
            return true;
        }
    }
    return false;
}

void ATMOPHistoricalVehicleDirector::ApplyDeferredPlacedVehicleState(
    const int32 CurrentSecond)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        if (!IsValid(Vehicle) || Runtime.bSpawnedByDirector)
        {
            continue;
        }

        const bool bShouldBeActive =
            ShouldSpawn(Runtime.Profile, false) &&
            Runtime.InitialSpawnSecond != INDEX_NONE &&
            Runtime.InitialSpawnSecond <= CurrentSecond;
        if (!bShouldBeActive)
        {
            Vehicle->SetActorHiddenInGame(true);
            Vehicle->SetActorEnableCollision(false);
            Vehicle->SetActorTickEnabled(false);
            Runtime.bDeferredPlacedVehicle = true;
            UnregisterVehicle(Vehicle);
        }
    }
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
        Vehicle->DisplayName = Runtime->Profile.DisplayName;
        Vehicle->VehicleCategoryId = Runtime->Profile.CategoryId;
        Vehicle->RefreshNameLabel();
        if (ATMOPConfiguredVehicle* Configured =
            Cast<ATMOPConfiguredVehicle>(Vehicle))
        {
            Configured->VehicleModel = Runtime->Profile.ModelData;
            Configured->bOverrideBodyColor =
                Runtime->Profile.bOverrideBodyColor;
            Configured->BodyColor = Runtime->Profile.BodyColor;
            Configured->ApplyConfiguration();
        }
        RegisterVehicle(Vehicle);
    }
}

ATMOPVehicleBase* ATMOPHistoricalVehicleDirector::SpawnVehicle(
    FHistoricalVehicleRuntime& Runtime,
    const FTransform* SpawnTransformOverride)
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return nullptr;
    }

    // A valid ModelData row must never fall back to a Blueprint's proxy mesh.
    // If no explicit class resolves, use the native configured vehicle.
    TSubclassOf<ATMOPVehicleBase> SpawnClass = DefaultVehicleClass;
    if (IsValid(Runtime.Profile.ModelData))
    {
        SpawnClass = ATMOPConfiguredVehicle::StaticClass();
    }
    if (UClass* RowClass = Runtime.Profile.VehicleClass.Get())
    {
        if (RowClass->IsChildOf(ATMOPVehicleBase::StaticClass()))
        {
            // Configured Blueprint subclasses can contain a permanently visible
            // proxy mesh. ModelData is authoritative, so use the clean native
            // configured actor for that case. Bespoke non-configured classes
            // (bus, ambulance, etc.) remain supported.
            SpawnClass =
                IsValid(Runtime.Profile.ModelData) &&
                RowClass->IsChildOf(ATMOPConfiguredVehicle::StaticClass())
                ? ATMOPConfiguredVehicle::StaticClass()
                : RowClass;
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

    const FTransform InitialTransform = SpawnTransformOverride != nullptr
        ? *SpawnTransformOverride : GetInitialTransform(Runtime.Profile);
    ATMOPVehicleBase* Vehicle = World->SpawnActorDeferred<ATMOPVehicleBase>(
        SpawnClass, InitialTransform, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if (!IsValid(Vehicle))
    {
        return nullptr;
    }

    Vehicle->VehicleId = Runtime.Profile.VehicleId;
    Vehicle->DisplayName = Runtime.Profile.DisplayName;
    Vehicle->VehicleCategoryId = Runtime.Profile.CategoryId;
    if (ATMOPConfiguredVehicle* Configured = Cast<ATMOPConfiguredVehicle>(Vehicle))
    {
        Configured->VehicleModel = Runtime.Profile.ModelData;
        Configured->bOverrideBodyColor =
            Runtime.Profile.bOverrideBodyColor;
        Configured->BodyColor = Runtime.Profile.BodyColor;
    }
    UGameplayStatics::FinishSpawningActor(Vehicle, InitialTransform);

    // Blueprint construction may restore the spawned class defaults (commonly
    // the white Volvo 240 proxy). Re-apply the row after construction so the
    // DataTable remains authoritative for both model and body colour.
    if (ATMOPConfiguredVehicle* Configured =
        Cast<ATMOPConfiguredVehicle>(Vehicle))
    {
        Configured->VehicleModel = Runtime.Profile.ModelData;
        Configured->bOverrideBodyColor =
            Runtime.Profile.bOverrideBodyColor;
        Configured->BodyColor = Runtime.Profile.BodyColor;
        const bool bApplied = Configured->ApplyConfiguration();
        const bool bCorrectMesh =
            bApplied &&
            IsValid(Runtime.Profile.ModelData) &&
            IsValid(Runtime.Profile.ModelData->BodyMesh) &&
            IsValid(Configured->BodyMesh) &&
            Configured->BodyMesh->GetStaticMesh() ==
                Runtime.Profile.ModelData->BodyMesh;

        if (!bCorrectMesh)
        {
            if (IsValid(Configured->BodyMesh))
                Configured->BodyMesh->SetStaticMesh(nullptr);
            UE_LOG(LogTemp, Error,
                TEXT("TMOP Historical Vehicles: '%s' refused proxy mesh. ModelData='%s', expected BodyMesh='%s'."),
                *Runtime.Profile.VehicleId.ToString(),
                IsValid(Runtime.Profile.ModelData)
                    ? *Runtime.Profile.ModelData->GetPathName()
                    : TEXT("None"),
                IsValid(Runtime.Profile.ModelData) &&
                    IsValid(Runtime.Profile.ModelData->BodyMesh)
                    ? *Runtime.Profile.ModelData->BodyMesh->GetPathName()
                    : TEXT("None"));
        }
        else
        {
            UE_LOG(LogTemp, Display,
                TEXT("TMOP Historical Vehicles: '%s' uses ModelData '%s' and mesh '%s'."),
                *Runtime.Profile.VehicleId.ToString(),
                *Runtime.Profile.ModelData->GetPathName(),
                *Configured->BodyMesh->GetStaticMesh()->GetPathName());
        }
    }

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
        if (Row->bSpawnInSimulation && Row->Timeline.IsEmpty())
        {
            OutErrors.Add(Prefix +
                TEXT(" is enabled for simulation but has no Timeline entries."));
        }
        if (Row->bOverrideBodyColor && !IsValid(Row->ModelData.Get()))
        {
            OutErrors.Add(Prefix +
                TEXT(" overrides Body Color but has no Model Data."));
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
    const TArray<FName>& PassAnchorIds,
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

        TArray<FName> RouteAnchorIds = PassAnchorIds;
        RouteAnchorIds.RemoveAll(
            [](const FName AnchorId) { return AnchorId.IsNone(); });
        RouteAnchorIds.Add(DestinationAnchorId);

        FName SegmentStartLaneId = RouteStartLaneId;
        for (const FName RouteAnchorId : RouteAnchorIds)
        {
            ATMOPHistoricalAnchor* RouteAnchor =
                Anchors->FindAnchor(RouteAnchorId);
            if (!IsValid(RouteAnchor))
            {
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP driving: pass/destination anchor '%s' is unavailable."),
                    *RouteAnchorId.ToString());
                return false;
            }

            FName SegmentDestinationLaneId;
            float SegmentDestinationDistance = 0.0f;
            if (!Network->FindNearestLane(
                RouteAnchor->GetAnchorLocation(),
                SegmentDestinationLaneId,
                SegmentDestinationDistance))
            {
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP driving: no lane was found near pass/destination anchor '%s'."),
                    *RouteAnchorId.ToString());
                return false;
            }

            TArray<FName> AutomaticSegment;
            if (!Network->FindLaneRoute(
                SegmentStartLaneId,
                SegmentDestinationLaneId,
                AutomaticSegment))
            {
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP driving: no connected lane route from '%s' to '%s' through anchor '%s'."),
                    *SegmentStartLaneId.ToString(),
                    *SegmentDestinationLaneId.ToString(),
                    *RouteAnchorId.ToString());
                return false;
            }
            if (!Route.IsEmpty() && !AutomaticSegment.IsEmpty() &&
                Route.Last() == AutomaticSegment[0])
            {
                AutomaticSegment.RemoveAt(0);
            }
            Route.Append(AutomaticSegment);
            SegmentStartLaneId = SegmentDestinationLaneId;
        }
        UE_LOG(LogTemp, Display,
            TEXT("TMOP driving: calculated %d lane(s) through %d pass anchor(s) to '%s'."),
            Route.Num(), PassAnchorIds.Num(),
            *DestinationAnchorId.ToString());
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
