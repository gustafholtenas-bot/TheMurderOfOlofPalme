#include "People/TMOPPersonRegistryDirector.h"

#include "Actions/TMOPActionExecutorComponent.h"
#include "AIController.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Groups/TMOPGroupDirector.h"
#include "Groups/TMOPGroupProfileTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "People/TMOPPersonRegistrySubsystem.h"
#include "Schedules/TMOPScheduleTypes.h"
#include "Sound/SoundBase.h"
#include "Time/TMOPClockSubsystem.h"
#include "Testing/TMOPTimelineValidationDirector.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Transit/TMOPBusServiceComponent.h"
#include "Transit/TMOPBusStopComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"
#include "World/TMOPWorldSubsystem.h"

ATMOPPersonRegistryDirector::ATMOPPersonRegistryDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPPersonRegistryDirector::BeginPlay()
{
    Super::BeginPlay();
    UTMOPPersonRegistrySubsystem* Registry = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    if (Registry == nullptr || !Registry->ConfigureProfileTable(PersonProfileTable))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP Person Registry Director has no valid profile table."));
        SetActorTickEnabled(false);
        return;
    }
    if (!Registry->ConfigureAppearanceAssetTable(AppearanceAssetTable))
        UE_LOG(LogTemp, Error, TEXT(
            "TMOP AppearanceAssetTable has the wrong row struct; modular assets are disabled."));

    if (bEnableAutomaticTimelineValidation)
    {
        bool bValidatorExists = false;
        for (TActorIterator<ATMOPTimelineValidationDirector> It(GetWorld()); It; ++It)
        {
            bValidatorExists = true;
            break;
        }
        if (!bValidatorExists)
            GetWorld()->SpawnActor<ATMOPTimelineValidationDirector>();
    }

    RefreshAllActiveProfiles();
    if (bSpawnPeopleAutomatically) InitializePersonSimulation();
}

void ATMOPPersonRegistryDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
        if (Pair.Value.bSpawnedByDirector && Pair.Value.Agent.IsValid())
            Pair.Value.Agent->Destroy();
    RuntimePeople.Reset();
    Super::EndPlay(EndPlayReason);
}

void ATMOPPersonRegistryDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateWorldFallSafety(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock == nullptr) return;
    const int32 CurrentSecond = Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE && CurrentSecond < LastEvaluatedSecond)
    {
        InitializePersonSimulation();
        return;
    }
    if (CurrentSecond != LastEvaluatedSecond)
    {
        const int32 PreviousSecond = LastEvaluatedSecond;
        EvaluatePeople(CurrentSecond, false);
        EvaluateAutomaticSpeech(CurrentSecond, PreviousSecond);
        LastEvaluatedSecond = CurrentSecond;
    }
}

void ATMOPPersonRegistryDirector::UpdateWorldFallSafety(
    const float DeltaSeconds)
{
    if (!bEnableWorldFallSafety || GetWorld() == nullptr) return;
    FallSafetyAccumulator += DeltaSeconds;
    if (FallSafetyAccumulator < FallSafetySampleIntervalSeconds) return;
    FallSafetyAccumulator = 0.0f;

    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* Pawn = *It;
        if (!IsValid(Pawn)) continue;

        const FVector Location = Pawn->GetActorLocation();
        const TWeakObjectPtr<APawn> PawnKey(Pawn);
        FTransform* LastSafe = LastSafePawnTransforms.Find(PawnKey);
        if (Location.Z < FallRecoveryTriggerZ)
        {
            FTransform Recovery = LastSafe != nullptr
                ? *LastSafe : Pawn->GetActorTransform();
            FVector RecoveryLocation = Recovery.GetLocation();
            if (LastSafe == nullptr)
                RecoveryLocation.Z = FallRecoveryTriggerZ + 1000.0f;
            else
                RecoveryLocation.Z += FallRecoveryHeightOffsetCm;
            Recovery.SetLocation(RecoveryLocation);

            if (ACharacter* Character = Cast<ACharacter>(Pawn))
                if (UCharacterMovementComponent* Movement =
                    Character->GetCharacterMovement())
                    Movement->StopMovementImmediately();
            if (AController* Controller = Pawn->GetController())
                Controller->StopMovement();
            Pawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            Pawn->SetActorTransform(
                Recovery, false, nullptr, ETeleportType::TeleportPhysics);
            LastSafePawnTransforms.Add(PawnKey, Recovery);
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP fall safety recovered pawn '%s' from Z %.1f to %s."),
                *Pawn->GetName(), Location.Z,
                *RecoveryLocation.ToString());
            continue;
        }

        bool bGrounded = true;
        if (const ACharacter* Character = Cast<ACharacter>(Pawn))
            if (const UCharacterMovementComponent* Movement =
                Character->GetCharacterMovement())
                bGrounded = Movement->IsMovingOnGround();
        if (bGrounded)
            LastSafePawnTransforms.Add(PawnKey, Pawn->GetActorTransform());
        else if (LastSafe == nullptr &&
            Location.Z > FallRecoveryTriggerZ + 500.0f)
            LastSafePawnTransforms.Add(PawnKey, Pawn->GetActorTransform());
    }
}

int32 ATMOPPersonRegistryDirector::RefreshAllActiveProfiles()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    int32 Loaded = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPPersonProfileComponent*> Components;
        It->GetComponents<UTMOPPersonProfileComponent>(Components);
        for (UTMOPPersonProfileComponent* Component : Components)
            Loaded += IsValid(Component) && Component->LoadProfile() ? 1 : 0;
    }
    return Loaded;
}

int32 ATMOPPersonRegistryDirector::InitializePersonSimulation()
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
        if (Pair.Value.bSpawnedByDirector && Pair.Value.Agent.IsValid())
            Pair.Value.Agent->Destroy();
    RuntimePeople.Reset();
    if (!IsValid(PersonProfileTable) ||
        PersonProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct()) return 0;

    // Existing level actors may begin play before or after this director.
    // Discovering here makes person spawning independent of BeginPlay order,
    // while each anchor's auto-registration component handles later streaming.
    if (UGameInstance* GameInstance = GetGameInstance())
        if (UTMOPAnchorSubsystem* Anchors =
            GameInstance->GetSubsystem<UTMOPAnchorSubsystem>())
        {
            const int32 Discovered = Anchors->DiscoverAnchorsInWorld();
            UE_LOG(LogTemp, Display,
                TEXT("TMOP People: discovered %d loaded anchor/place actor(s) before spawn."),
                Discovered);
        }

    TArray<FString> Errors;
    ValidatePeopleTable(Errors);
    for (const FString& Error : Errors)
        UE_LOG(LogTemp, Error, TEXT("TMOP People: %s"), *Error);

    const TArray<FName> RowNames = PersonProfileTable->GetRowNames();
    for (const FName RowName : RowNames)
    {
        const FTMOPPersonProfileRow* Row = PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
            RowName, TEXT("TMOPPersonSimulation"), false);
        if (Row == nullptr || !Row->bSpawnInSimulation || Row->EntityId.IsNone() ||
            Row->Timeline.IsEmpty()) continue;
        FPersonRuntime Runtime;
        Runtime.RowName = RowName;
        Runtime.Profile = *Row;
        // Timeline array order is authoritative. Shared/arrival times cannot be
        // correctly sorted by their fallback absolute Time value.
        UTMOPPersonRegistrySubsystem* Registry =
            GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>();
        if (Registry != nullptr)
            Runtime.Agent = Registry->FindActiveAgent(Row->EntityId);
        RuntimePeople.Add(Row->EntityId, MoveTemp(Runtime));
    }

    if (HasValidGroupTable()) ApplyGroupTableMemberships();

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : SimulationEpoch.ToSecondsFromMidnight();

    // Spawn every person whose initial placement exists at the scenario epoch
    // before groups are built. This guarantees that a seek directly to a later
    // time cannot execute the leader's first move before the group exists.
    const int32 EpochSecond = SimulationEpoch.ToSecondsFromMidnight();
    const int32 InitialEvaluationSecond =
        FMath::Min(CurrentSecond, EpochSecond);
    EvaluatePeople(InitialEvaluationSecond, true);
    if (bCreateGroupsFromGroupTable && HasValidGroupTable())
        RebuildGroupsFromGroupTable();
    else if (bCreateGroupsFromPeopleTable)
        RebuildGroupsFromPeople();

    if (CurrentSecond > InitialEvaluationSecond)
        EvaluatePeople(CurrentSecond, bCatchUpToCurrentClockOnBeginPlay);
    EvaluateAutomaticSpeech(CurrentSecond, CurrentSecond - 1);
    LastEvaluatedSecond = CurrentSecond;
    UE_LOG(LogTemp, Display, TEXT("TMOP People: initialized %d timeline profile(s), %d active agent(s)."),
        RuntimePeople.Num(), RefreshAllActiveProfiles());
    return RuntimePeople.Num();
}

int32 ATMOPPersonRegistryDirector::InitializePersonSimulationForWorldBake()
{
    TGuardValue<bool> Guard(bRestoringWorldBake, true);
    return InitializePersonSimulation();
}

void ATMOPPersonRegistryDirector::EvaluateAutomaticSpeech(
    const int32 CurrentSecond, const int32 PreviousSecond)
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
    {
        FPersonRuntime& Runtime = Pair.Value;
        const FTMOPTimedSpeechLine* LatestDueLine = nullptr;
        while (Runtime.Profile.AutomaticSpeech.IsValidIndex(
            Runtime.NextAutomaticSpeechIndex))
        {
            const FTMOPTimedSpeechLine& Line =
                Runtime.Profile.AutomaticSpeech[
                    Runtime.NextAutomaticSpeechIndex];
            int32 LineSecond = INDEX_NONE;
            if (!ResolveAutomaticSpeechSecond(
                Runtime, Line, LineSecond))
                break;
            if (LineSecond > CurrentSecond) break;
            if (LineSecond > PreviousSecond && !Line.Text.IsEmpty())
                LatestDueLine = &Line;
            Runtime.LastResolvedAutomaticSpeechSecond = LineSecond;
            ++Runtime.NextAutomaticSpeechIndex;
        }

        ATMOPHistoricalAgent* Agent = Runtime.Agent.Get();
        if (LatestDueLine == nullptr || !IsValid(Agent)) continue;
        USoundBase* VoiceOver = LatestDueLine->VoiceOver.IsNull()
            ? nullptr : LatestDueLine->VoiceOver.LoadSynchronous();
        Agent->ShowAutomaticSpeech(
            LatestDueLine->Text, VoiceOver,
            LatestDueLine->DisplayDurationOverrideSeconds);
    }
}

bool ATMOPPersonRegistryDirector::ResolveAutomaticSpeechSecond(
    const FPersonRuntime& Runtime,
    const FTMOPTimedSpeechLine& Line,
    int32& OutSecond) const
{
    OutSecond = INDEX_NONE;
    switch (Line.TimingMode)
    {
        case ETMOPSpeechTimingMode::Absolute:
            OutSecond = Line.Time.ToSecondsFromMidnight();
            return true;

        case ETMOPSpeechTimingMode::RelativeToPreviousLine:
            if (Runtime.LastResolvedAutomaticSpeechSecond == INDEX_NONE)
                return false;
            OutSecond = Runtime.LastResolvedAutomaticSpeechSecond +
                Line.OffsetSeconds;
            return true;

        case ETMOPSpeechTimingMode::RelativeToSharedEvent:
        {
            if (Line.SharedEventId.IsNone() || GetGameInstance() == nullptr)
                return false;
            const UTMOPHistoricalEventSubsystem* Events =
                GetGameInstance()->GetSubsystem<
                    UTMOPHistoricalEventSubsystem>();
            FTMOPHistoricalEventRuntime EventRuntime;
            if (!IsValid(Events) ||
                !Events->TryGetEventRuntime(
                    Line.SharedEventId, EventRuntime) ||
                !EventRuntime.bHasResolvedTime)
                return false;
            OutSecond =
                EventRuntime.ResolvedTime.ToSecondsFromMidnight() +
                Line.OffsetSeconds;
            return true;
        }
    }
    return false;
}

void ATMOPPersonRegistryDirector::EvaluatePeople(const int32 CurrentSecond,
    const bool bCatchUp)
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
    {
        FPersonRuntime& Runtime = Pair.Value;
        while (!Runtime.bCompleted && Runtime.Profile.Timeline.IsValidIndex(Runtime.NextTimelineIndex))
        {
            const FTMOPPersonTimelineEntry& Entry =
                Runtime.Profile.Timeline[Runtime.NextTimelineIndex];
            int32 ResolvedSecond = INDEX_NONE;
            if (!ResolveEntrySecond(Runtime, Entry, ResolvedSecond) ||
                ResolvedSecond > CurrentSecond) break;

            // Documentation contributes to chronology/relative timing but never
            // creates or changes a physical runtime person.
            if (Entry.Usage == ETMOPPersonTimelineUsage::DocumentationOnly)
            {
                Runtime.LastResolvedTimelineSecond = ResolvedSecond;
                ++Runtime.NextTimelineIndex;
                Runtime.CachedResolvedSecond = INDEX_NONE;
                continue;
            }

            // A planned anchor is a valid research reference before it exists.
            // Once an actor with the same ID is added, runtime-enabled entries
            // begin working without a data migration.
            if (Entry.AnchorReferenceMode == ETMOPAnchorReferenceMode::PlannedFuture &&
                !Entry.TargetAnchorId.IsNone())
            {
                UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
                    ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
                if (Anchors == nullptr || !IsValid(Anchors->FindAnchor(Entry.TargetAnchorId)))
                {
                    Runtime.LastResolvedTimelineSecond = ResolvedSecond;
                    ++Runtime.NextTimelineIndex;
                    Runtime.CachedResolvedSecond = INDEX_NONE;
                    continue;
                }
            }

            if (!Runtime.Agent.IsValid())
            {
                if ((Entry.Action == ETMOPPersonTimelineAction::InitialPlacement ||
                     Entry.Action == ETMOPPersonTimelineAction::Spawn) &&
                    (Entry.LocationType == ETMOPPersonLocationType::Unknown ||
                     Entry.LocationType == ETMOPPersonLocationType::NotPresent))
                {
                    // Preserve the historical unknown: no physical person exists yet.
                    Runtime.LastResolvedTimelineSecond = ResolvedSecond;
                    ++Runtime.NextTimelineIndex;
                    Runtime.CachedResolvedSecond = INDEX_NONE;
                    continue;
                }
                if (Entry.Action != ETMOPPersonTimelineAction::InitialPlacement &&
                    Entry.Action != ETMOPPersonTimelineAction::Spawn)
                    break;
                if (!SpawnPerson(Runtime, Entry)) break;
                Runtime.LastResolvedTimelineSecond = ResolvedSecond;
                ++Runtime.NextTimelineIndex;
                Runtime.CachedResolvedSecond = INDEX_NONE;
                continue;
            }

            ATMOPHistoricalAgent* Agent = Runtime.Agent.Get();
            // Being late during a live simulation is not the same thing as
            // restoring/fast-forwarding world state.  Treating every overdue
            // entry as catch-up allowed MoveToAnchor entries with
            // bTeleportDuringCatchUp to snap agents to their destinations as
            // soon as an earlier movement, crowd blockage, or nav delay made
            // the timeline even one second late.  Only the director's explicit
            // initial restore/seek pass may use catch-up placement.  During
            // normal play the busy check below keeps the overdue entry pending;
            // it is executed physically as soon as the previous action ends.
            const bool bEntryCatchUp = bCatchUp;
            const bool bFollower =
                ShouldFollowGroupLeader(Runtime.Profile, Agent);
            if (bFollower && Runtime.NextTimelineIndex > 0 &&
                Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor &&
                !bEntryCatchUp)
            {
                // The group director moves this agent through the leader, but
                // validation still needs the follower's resolved timeline time.
                OnTimelineEntryApplied.Broadcast(
                    Runtime.Profile.EntityId,
                    Entry,
                    ResolvedSecond,
                    true,
                    false);
                Runtime.LastResolvedTimelineSecond = ResolvedSecond;
                ++Runtime.NextTimelineIndex;
                Runtime.CachedResolvedSecond = INDEX_NONE;
                continue;
            }

            if (!bEntryCatchUp && IsAgentBusy(Agent)) break;
            const bool bApplied =
                ApplyTimelineEntry(Runtime, Entry, bEntryCatchUp);
            OnTimelineEntryApplied.Broadcast(
                Runtime.Profile.EntityId,
                Entry,
                ResolvedSecond,
                bApplied,
                bEntryCatchUp);
            if (!bApplied) break;
            Runtime.LastResolvedTimelineSecond = ResolvedSecond;
            ++Runtime.NextTimelineIndex;
            Runtime.CachedResolvedSecond = INDEX_NONE;
        }
    }
}

bool ATMOPPersonRegistryDirector::SpawnPerson(FPersonRuntime& Runtime,
    const FTMOPPersonTimelineEntry& InitialEntry)
{
    if (InitialEntry.LocationType == ETMOPPersonLocationType::Unknown ||
        InitialEntry.LocationType == ETMOPPersonLocationType::NotPresent || GetWorld() == nullptr)
        return false;

    TSubclassOf<ATMOPHistoricalAgent> Class = Runtime.Profile.AgentClass != nullptr
        ? Runtime.Profile.AgentClass : DefaultAgentClass;
    if (Class == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP person '%s': no Agent Class and no Default Agent Class."),
            *Runtime.Profile.EntityId.ToString());
        Runtime.bCompleted = true;
        return false;
    }

    FTransform SpawnTransform = InitialEntry.WorldTransform;
    if (InitialEntry.LocationType == ETMOPPersonLocationType::Anchor)
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(InitialEntry.TargetAnchorId) : nullptr;
        if (!IsValid(Anchor)) return false;
        SpawnTransform = Anchor->GetActorTransform();
        SpawnTransform.SetLocation(Anchor->GetPlacementLocation(Runtime.Profile.EntityId));
    }
    else if (InitialEntry.LocationType == ETMOPPersonLocationType::VenueSeat)
    {
        UTMOPCinemaSeatSubsystem* Seats = GetGameInstance()->GetSubsystem<UTMOPCinemaSeatSubsystem>();
        if (Seats != nullptr) Seats->DiscoverSeatsInWorld();
        UTMOPCinemaSeatComponent* Seat = Seats != nullptr
            ? Seats->FindSeat(InitialEntry.TargetSeatId) : nullptr;
        if (!IsValid(Seat)) return false;
        SpawnTransform = Seat->GetSeatWorldTransform();
    }
    else if (InitialEntry.LocationType == ETMOPPersonLocationType::VehicleSeat ||
        InitialEntry.LocationType == ETMOPPersonLocationType::BusSeat)
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(InitialEntry.TargetEntityId);
        if (!IsValid(Vehicle)) return false;
        SpawnTransform = Vehicle->GetActorTransform();
        for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
            if (IsValid(Seat) && (InitialEntry.TargetSeatId.IsNone() ||
                Seat->SeatId == InitialEntry.TargetSeatId))
            {
                SpawnTransform = Seat->GetComponentTransform();
                break;
            }
    }
    else if (InitialEntry.LocationType == ETMOPPersonLocationType::StandingInVehicle)
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(InitialEntry.TargetEntityId);
        if (!IsValid(Vehicle)) return false;
        SpawnTransform = Vehicle->GetActorTransform();
    }

    ATMOPHistoricalAgent* Agent = GetWorld()->SpawnActorDeferred<ATMOPHistoricalAgent>(
        Class, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!IsValid(Agent)) return false;
    Agent->EntityIdentity->EntityId = Runtime.Profile.EntityId;
    Agent->EntityIdentity->EntityType = TEXT("Agent");
    Agent->DisplayName = Runtime.Profile.FullName;
    Agent->PersonCategoryId = Runtime.Profile.CategoryId;
    Agent->SourceReference = Runtime.Profile.GeneralSourceReference;
    Agent->MovementProfile = Runtime.Profile.MovementProfile;
    Agent->SocialGroupId = Runtime.Profile.SocialGroupId;
    Agent->ActivityState = InitialEntry.ActivityState;
    Agent->LifeState = InitialEntry.LifeState;
    UGameplayStatics::FinishSpawningActor(Agent, SpawnTransform);

    UTMOPPersonProfileComponent* ProfileComponent =
        Agent->FindComponentByClass<UTMOPPersonProfileComponent>();
    if (!IsValid(ProfileComponent))
    {
        ProfileComponent = NewObject<UTMOPPersonProfileComponent>(Agent, TEXT("PersonProfile"));
        Agent->AddInstanceComponent(ProfileComponent);
        ProfileComponent->RegisterComponent();
    }
    if (IsValid(ProfileComponent)) ProfileComponent->LoadProfile();

    if (bDisableCollisionForObservedUnknownPeople &&
        Runtime.Profile.CategoryId == FName(TEXT("OBSERVED_UNKNOWN")))
    {
        // These actors visualize uncertain observations.  They must not be
        // able to jam an otherwise deterministic historical traffic route.
        Agent->SetActorEnableCollision(false);
        if (UCapsuleComponent* Capsule = Agent->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Capsule->SetCanEverAffectNavigation(false);
        }
    }
    Runtime.Agent = Agent;
    Runtime.bSpawnedByDirector = true;
    if (!ApplyPlacement(Agent, InitialEntry, true))
    {
        Agent->Destroy();
        Runtime.Agent.Reset();
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("TMOP person '%s' spawned from row '%s'."),
        *Runtime.Profile.EntityId.ToString(), *Runtime.RowName.ToString());
    return true;
}

bool ATMOPPersonRegistryDirector::ApplyTimelineEntry(FPersonRuntime& Runtime,
    const FTMOPPersonTimelineEntry& Entry, const bool bCatchUp)
{
    ATMOPHistoricalAgent* Agent = Runtime.Agent.Get();
    if (!IsValid(Agent)) return false;
    switch (Entry.Action)
    {
    case ETMOPPersonTimelineAction::InitialPlacement:
    case ETMOPPersonTimelineAction::Spawn:
        return ApplyPlacement(Agent, Entry, bCatchUp);
    case ETMOPPersonTimelineAction::Despawn:
        Agent->Destroy();
        Runtime.Agent.Reset();
        Runtime.bSpawnedByDirector = false;
        return true;
    case ETMOPPersonTimelineAction::MoveToAnchor:
        if (bCatchUp && bRestoringWorldBake)
            return true;
        // A previous stationary interaction may have locked the agent's gaze
        // to a shop window or another world anchor.  Movement must release
        // that focus before path following resumes.
        Agent->EndDialogueFocus();
        if (bCatchUp && Entry.bTeleportDuringCatchUp)
            return ApplyPlacement(Agent, Entry, true);
        if (!Agent->SocialGroupId.IsNone() &&
            IsGroupLeader(Runtime.Profile, Agent))
        {
            UTMOPAnchorSubsystem* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
            ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
                ? Anchors->FindAnchor(Entry.TargetAnchorId) : nullptr;
            ATMOPGroupDirector* Groups = FindGroupDirector();
            if (IsValid(Groups)) Groups->RefreshWaitingGroups();
            if (IsValid(Anchor) && IsValid(Groups) &&
                Groups->DoesGroupExist(Agent->SocialGroupId))
            {
                TArray<FVector> RouteLocations;
                for (const FName PassAnchorId : Entry.PassAnchorIds)
                    if (ATMOPHistoricalAnchor* PassAnchor =
                        Anchors->FindAnchor(PassAnchorId))
                        RouteLocations.Add(PassAnchor->GetPlacementLocation(
                            Agent->SocialGroupId));
                RouteLocations.Add(Anchor->GetPlacementLocation(
                    Agent->SocialGroupId));

                if (Entry.bTimeIsArrival && !RouteLocations.IsEmpty())
                {
                    FVector Start = Agent->GetActorLocation();
                    double RemainingPathCm = 0.0;
                    for (const FVector& End : RouteLocations)
                    {
                        double SegmentCm = FVector::Dist2D(Start, End);
                        UNavigationSystemV1::GetPathLength(
                            GetWorld(), Start, End, SegmentCm, nullptr, nullptr);
                        RemainingPathCm += SegmentCm;
                        Start = End;
                    }
                    const int32 DepartureSecond =
                        Runtime.CachedResolvedSecond != INDEX_NONE
                        ? Runtime.CachedResolvedSecond
                        : Entry.Time.ToSecondsFromMidnight();
                    const int32 ArrivalSecond = DepartureSecond +
                        EstimateTravelSeconds(Runtime, Entry);
                    const UTMOPClockSubsystem* Clock =
                        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
                    const int32 RemainingSeconds = Clock != nullptr
                        ? ArrivalSecond - Clock->GetCurrentTime().ToSecondsFromMidnight()
                        : 0;
                    const float Multiplier =
                        Runtime.Profile.MovementProfile.PersonalSpeedMultiplier *
                        Agent->AppearanceMovementSpeedMultiplier;
                    const float MinimumSpeed =
                        Runtime.Profile.MovementProfile.SlowWalkSpeed * Multiplier;
                    const float MaximumSpeed =
                        (Entry.ActivityState == ETMOPAgentActivityState::Running ||
                         Entry.ActivityState == ETMOPAgentActivityState::Fleeing)
                        ? Runtime.Profile.MovementProfile.RunSpeed * Multiplier
                        : Entry.ActivityState == ETMOPAgentActivityState::Sprinting
                        ? Runtime.Profile.MovementProfile.SprintSpeed * Multiplier
                        : Runtime.Profile.MovementProfile.FastWalkSpeed * Multiplier;
                    const float TimedMinimumSpeed =
                        Entry.TravelSpeedOverrideCmPerSecond > 0.0f
                        ? Entry.TravelSpeedOverrideCmPerSecond
                        : MinimumSpeed;
                    const float TimedMaximumSpeed =
                        Entry.TravelSpeedOverrideCmPerSecond > 0.0f
                        ? Entry.TravelSpeedOverrideCmPerSecond
                        : MaximumSpeed;
                    const float RequiredSpeed = RemainingSeconds > 0
                        ? static_cast<float>(RemainingPathCm) / RemainingSeconds
                        : TimedMaximumSpeed;
                    const float ChosenSpeed = FMath::Clamp(
                        RequiredSpeed, TimedMinimumSpeed, TimedMaximumSpeed);
                    for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It)
                        if (It->SocialGroupId == Agent->SocialGroupId)
                            if (UCharacterMovementComponent* Movement =
                                It->GetCharacterMovement())
                                Movement->MaxWalkSpeed = ChosenSpeed;
                    if (RequiredSpeed > TimedMaximumSpeed)
                        UE_LOG(LogTemp, Error,
                            TEXT("TMOP precision: group '%s' cannot reach '%s' by %d without exceeding realistic speed (required %.0f, max %.0f cm/s)."),
                            *Agent->SocialGroupId.ToString(),
                            *Entry.TargetAnchorId.ToString(), ArrivalSecond,
                            RequiredSpeed, TimedMaximumSpeed);
                }
                return Groups->MoveGroupThroughLocations(
                    Agent->SocialGroupId,
                    RouteLocations,
                    FMath::Max(80.0f, Anchor->MinimumSpacingCm));
            }
        }
        if (!IsValid(Agent->ActionExecutor)) return false;
        {
            FTMOPScheduleEntry Action;
            Action.EntryId = Entry.EntryId;
            Action.AbsoluteTime = FTMOPTime::FromSecondsFromMidnight(
                Runtime.CachedResolvedSecond != INDEX_NONE
                    ? Runtime.CachedResolvedSecond
                    : Entry.Time.ToSecondsFromMidnight());
            Action.ActionType = ETMOPScheduleActionType::MoveToAnchor;
            Action.TargetAnchorId = Entry.TargetAnchorId;
            Action.PassAnchorIds = Entry.PassAnchorIds;
            Action.ActivityState = Entry.ActivityState == ETMOPAgentActivityState::Idle
                ? ETMOPAgentActivityState::Walking : Entry.ActivityState;
            Action.Confidence = Entry.Confidence;
            Action.SourceId = Entry.SourceReference;
            Action.Notes = Entry.Notes;

            if (Entry.bTimeIsArrival)
            {
                const float PersonalMultiplier =
                    Runtime.Profile.MovementProfile.PersonalSpeedMultiplier *
                    Agent->AppearanceMovementSpeedMultiplier;
                float MinimumSpeed =
                    Runtime.Profile.MovementProfile.SlowWalkSpeed;
                float MaximumSpeed =
                    Runtime.Profile.MovementProfile.FastWalkSpeed;
                switch (Action.ActivityState)
                {
                case ETMOPAgentActivityState::FastWalking:
                    MinimumSpeed = Runtime.Profile.MovementProfile.NormalWalkSpeed;
                    MaximumSpeed = Runtime.Profile.MovementProfile.FastWalkSpeed;
                    break;
                case ETMOPAgentActivityState::Jogging:
                    MinimumSpeed = Runtime.Profile.MovementProfile.FastWalkSpeed;
                    MaximumSpeed = Runtime.Profile.MovementProfile.JogSpeed;
                    break;
                case ETMOPAgentActivityState::Running:
                case ETMOPAgentActivityState::Fleeing:
                    MinimumSpeed = Runtime.Profile.MovementProfile.JogSpeed;
                    MaximumSpeed = Runtime.Profile.MovementProfile.RunSpeed;
                    break;
                case ETMOPAgentActivityState::Sprinting:
                    MinimumSpeed = Runtime.Profile.MovementProfile.RunSpeed;
                    MaximumSpeed = Runtime.Profile.MovementProfile.SprintSpeed;
                    break;
                default:
                    break;
                }
                MinimumSpeed *= PersonalMultiplier;
                MaximumSpeed *= PersonalMultiplier;
                if (Entry.TravelSpeedOverrideCmPerSecond > 0.0f)
                {
                    MinimumSpeed = Entry.TravelSpeedOverrideCmPerSecond;
                    MaximumSpeed = Entry.TravelSpeedOverrideCmPerSecond;
                }
                const int32 DepartureSecond =
                    Runtime.CachedResolvedSecond != INDEX_NONE
                    ? Runtime.CachedResolvedSecond
                    : Entry.Time.ToSecondsFromMidnight();
                const int32 ExpectedArrivalSecond = DepartureSecond +
                    EstimateTravelSeconds(Runtime, Entry);
                Agent->ActionExecutor->ConfigureNextTimedMove(
                    ExpectedArrivalSecond,
                    MinimumSpeed,
                    MaximumSpeed);
            }
            return Agent->ActionExecutor->ExecuteScheduleEntry(Action);
        }
    case ETMOPPersonTimelineAction::Wait:
    case ETMOPPersonTimelineAction::ChangeActivity:
        Agent->SetActivityState(Entry.ActivityState);
        return true;
    case ETMOPPersonTimelineAction::ChangeLifeState:
        Agent->SetLifeState(Entry.LifeState);
        return true;
    case ETMOPPersonTimelineAction::SitDown:
    case ETMOPPersonTimelineAction::StandUp:
    case ETMOPPersonTimelineAction::EnterVehicle:
    case ETMOPPersonTimelineAction::ExitVehicle:
        if (bCatchUp && bRestoringWorldBake)
            return true;
        return ApplyPlacement(Agent, Entry, bCatchUp);
    case ETMOPPersonTimelineAction::BeginDriving:
        if (bCatchUp && bRestoringWorldBake)
            return true;
        return ApplyPlacement(Agent, Entry, bCatchUp);
    case ETMOPPersonTimelineAction::CreateGroup:
    {
        if (bCatchUp && bRestoringWorldBake) return true;
        ATMOPGroupDirector* Groups = FindGroupDirector();
        return IsValid(Groups) &&
            Groups->CreateGroup(Entry.GroupDefinition);
    }
    case ETMOPPersonTimelineAction::JoinGroup:
    {
        if (bCatchUp && bRestoringWorldBake) return true;
        ATMOPGroupDirector* Groups = FindGroupDirector();
        const FName EntityId = Agent->EntityIdentity != nullptr
            ? Agent->EntityIdentity->EntityId : NAME_None;
        return IsValid(Groups) &&
            Groups->AddMember(Entry.TargetGroupId, EntityId);
    }
    case ETMOPPersonTimelineAction::LeaveGroup:
    {
        if (bCatchUp && bRestoringWorldBake) return true;
        ATMOPGroupDirector* Groups = FindGroupDirector();
        const FName EntityId = Agent->EntityIdentity != nullptr
            ? Agent->EntityIdentity->EntityId : NAME_None;
        const FName GroupId = Entry.TargetGroupId.IsNone()
            ? Agent->SocialGroupId : Entry.TargetGroupId;
        if (!IsValid(Groups)) return false;
        // Group-ending entries can legitimately share the same historical
        // second.  If the leader already dissolved the group, a follower's
        // LeaveGroup is complete rather than a permanent retry failure.
        if (GroupId.IsNone() || !Groups->DoesGroupExist(GroupId))
            return Agent->SocialGroupId.IsNone() ||
                Agent->SocialGroupId == GroupId;
        return Groups->RemoveMember(GroupId, EntityId);
    }
    case ETMOPPersonTimelineAction::SplitGroup:
    {
        if (bCatchUp && bRestoringWorldBake) return true;
        ATMOPGroupDirector* Groups = FindGroupDirector();
        return IsValid(Groups) &&
            Groups->SplitGroup(
                Entry.TargetGroupId, Entry.SplitGroupDefinitions);
    }
    case ETMOPPersonTimelineAction::DissolveGroup:
    {
        if (bCatchUp && bRestoringWorldBake) return true;
        ATMOPGroupDirector* Groups = FindGroupDirector();
        if (!IsValid(Groups)) return false;
        return Entry.TargetGroupId.IsNone() ||
            !Groups->DoesGroupExist(Entry.TargetGroupId) ||
            Groups->DissolveGroup(Entry.TargetGroupId);
    }
    case ETMOPPersonTimelineAction::SetGroupLeader:
    {
        if (bCatchUp && bRestoringWorldBake) return true;
        ATMOPGroupDirector* Groups = FindGroupDirector();
        return IsValid(Groups) &&
            Groups->SetGroupLeader(
                Entry.TargetGroupId,
                Entry.NewGroupLeaderEntityId);
    }
    case ETMOPPersonTimelineAction::Interact:
        if (!Entry.TargetAnchorId.IsNone() && GetGameInstance() != nullptr)
        {
            UTMOPAnchorSubsystem* Anchors =
                GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
            ATMOPHistoricalAnchor* FocusAnchor = Anchors != nullptr
                ? Anchors->FindAnchor(Entry.TargetAnchorId) : nullptr;
            if (IsValid(FocusAnchor))
                Agent->BeginDialogueFocus(FocusAnchor);
        }
        Agent->SetActivityState(ETMOPAgentActivityState::Interacting);
        return true;
    case ETMOPPersonTimelineAction::Custom:
        Agent->SetActivityState(ETMOPAgentActivityState::Interacting);
        return true;
    default:
        return false;
    }
}

bool ATMOPPersonRegistryDirector::ApplyPlacement(ATMOPHistoricalAgent* Agent,
    const FTMOPPersonTimelineEntry& Entry, const bool bCatchUp)
{
    if (!IsValid(Agent) || GetGameInstance() == nullptr) return false;

    if (Entry.Action == ETMOPPersonTimelineAction::StandUp)
    {
        UTMOPCinemaSeatSubsystem* Seats = GetGameInstance()->GetSubsystem<UTMOPCinemaSeatSubsystem>();
        UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(Entry.TargetSeatId) : nullptr;
        if (IsValid(Seat) && Seat->GetOccupyingAgent() == Agent) Seat->StandAgent(Agent);
        Agent->SetActivityState(ETMOPAgentActivityState::Standing);
        return true;
    }

    if (Entry.Action == ETMOPPersonTimelineAction::ExitVehicle)
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(Entry.TargetEntityId);
        if (!bCatchUp && IsValid(Vehicle) && !Entry.TargetStopId.IsNone())
        {
            UTMOPBusServiceComponent* Service =
                Vehicle->FindComponentByClass<UTMOPBusServiceComponent>();
            UTMOPBusStopComponent* Stop = IsValid(Service)
                ? Service->GetCurrentTargetStop() : nullptr;
            if (!IsValid(Service) || !Service->bDoorsOpen || !IsValid(Stop) ||
                Stop->StopId != Entry.TargetStopId) return false;
        }
        // Timetable times are targets, not permission to abandon a delayed
        // emergency vehicle halfway along its route. Keep occupants seated
        // until the traffic component has actually completed the current
        // planned route; EvaluatePeople will retry this entry every tick.
        if (!bCatchUp && IsValid(Vehicle))
            if (const UTMOPTrafficVehicleMovementComponent* Movement =
                Vehicle->FindComponentByClass<
                    UTMOPTrafficVehicleMovementComponent>())
                if (!Movement->PlannedLaneIds.IsEmpty() &&
                    Movement->TrafficState !=
                        ETMOPTrafficVehicleState::RouteComplete)
                    return false;
        return IsValid(Vehicle) && Vehicle->ExitVehicle(Agent);
    }

    if (Entry.Action == ETMOPPersonTimelineAction::BeginDriving)
    {
        const FName DriverEntityId =
            IsValid(Agent->EntityIdentity)
            ? Agent->EntityIdentity->EntityId : NAME_None;
        for (TActorIterator<ATMOPHistoricalVehicleDirector> It(GetWorld());
            It; ++It)
            return It->BeginDrivingVehicle(
                Entry.TargetEntityId,
                DriverEntityId,
                Entry.OrderedLaneIds,
                Entry.PassAnchorIds,
                Entry.VehicleRouteMode,
                Entry.DrivingDestinationAnchorId,
                Entry.VehicleStartDistanceAlongFirstLaneCm);
        UE_LOG(LogTemp, Error,
            TEXT("TMOP person '%s': no Historical Vehicle Director for BeginDriving."),
            *DriverEntityId.ToString());
        return false;
    }

    switch (Entry.LocationType)
    {
    case ETMOPPersonLocationType::Anchor:
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Entry.TargetAnchorId) : nullptr;
        if (!IsValid(Anchor)) return false;
        Agent->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        const FName StableKey = Agent->EntityIdentity != nullptr
            ? Agent->EntityIdentity->EntityId : NAME_None;
        Agent->SetActorLocationAndRotation(Anchor->GetPlacementLocation(StableKey), Anchor->GetActorRotation(),
            false, nullptr, ETeleportType::TeleportPhysics);
        Agent->SetActivityState(Entry.ActivityState);
        return true;
    }
    case ETMOPPersonLocationType::WorldTransform:
        Agent->SetActorTransform(Entry.WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
        Agent->SetActivityState(Entry.ActivityState);
        return true;
    case ETMOPPersonLocationType::VenueSeat:
    {
        UTMOPCinemaSeatSubsystem* Seats = GetGameInstance()->GetSubsystem<UTMOPCinemaSeatSubsystem>();
        if (Seats != nullptr) Seats->DiscoverSeatsInWorld();
        UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(Entry.TargetSeatId) : nullptr;
        return IsValid(Seat) && Seat->SeatAgent(Agent);
    }
    case ETMOPPersonLocationType::VehicleSeat:
    case ETMOPPersonLocationType::BusSeat:
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(Entry.TargetEntityId);
        if (!bCatchUp && IsValid(Vehicle) && !Entry.TargetStopId.IsNone())
        {
            UTMOPBusServiceComponent* Service =
                Vehicle->FindComponentByClass<UTMOPBusServiceComponent>();
            UTMOPBusStopComponent* Stop = IsValid(Service)
                ? Service->GetCurrentTargetStop() : nullptr;
            if (!IsValid(Service) || !Service->bDoorsOpen || !IsValid(Stop) ||
                Stop->StopId != Entry.TargetStopId) return false;
        }
        return IsValid(Vehicle) && Vehicle->EnterVehicle(Agent, Entry.TargetSeatId);
    }
    case ETMOPPersonLocationType::StandingInVehicle:
    {
        AActor* Vehicle = FindVehicle(Entry.TargetEntityId);
        if (!IsValid(Vehicle)) return false;
        Agent->AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
        Agent->SetActivityState(ETMOPAgentActivityState::RidingVehicle);
        return true;
    }
    case ETMOPPersonLocationType::Unknown:
    case ETMOPPersonLocationType::NotPresent:
    default:
        return Entry.Action == ETMOPPersonTimelineAction::Wait ||
            Entry.Action == ETMOPPersonTimelineAction::ChangeActivity;
    }
}

bool ATMOPPersonRegistryDirector::ResolveEntrySecond(FPersonRuntime& Runtime,
    const FTMOPPersonTimelineEntry& Entry, int32& OutSecond) const
{
    if (Runtime.CachedResolvedSecond != INDEX_NONE)
    {
        OutSecond = Runtime.CachedResolvedSecond;
        return true;
    }

    int32 BaseSecond = Entry.Time.ToSecondsFromMidnight();
    if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
    {
        UTMOPHistoricalEventSubsystem* Events = GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>() : nullptr;
        FTMOPHistoricalEventRuntime EventRuntime;
        if (Events == nullptr || Entry.SharedEventId.IsNone() ||
            !Events->TryGetEventRuntime(Entry.SharedEventId, EventRuntime) ||
            !EventRuntime.bHasResolvedTime) return false;
        BaseSecond = EventRuntime.ResolvedTime.ToSecondsFromMidnight() + Entry.EventOffsetSeconds;
    }
    else if (Entry.TimingMode ==
        ETMOPEventTimingMode::RelativeToPreviousEntry)
    {
        if (Runtime.NextTimelineIndex <= 0 ||
            Runtime.LastResolvedTimelineSecond == INDEX_NONE)
        {
            return false;
        }
        BaseSecond =
            Runtime.LastResolvedTimelineSecond + Entry.EventOffsetSeconds;
    }

    if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor && Entry.bTimeIsArrival)
        BaseSecond -= EstimateTravelSeconds(Runtime, Entry);
    Runtime.CachedResolvedSecond = FMath::Max(0, BaseSecond);
    OutSecond = Runtime.CachedResolvedSecond;
    return true;
}

int32 ATMOPPersonRegistryDirector::EstimateTravelSeconds(const FPersonRuntime& Runtime,
    const FTMOPPersonTimelineEntry& Entry) const
{
    if (GetWorld() == nullptr || GetGameInstance() == nullptr) return 0;
    const UTMOPAnchorSubsystem* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
    if (Anchors == nullptr) return 0;

    // Arrival-timed entries are scheduled before they execute.  Their travel
    // time must therefore be measured from the preceding timeline location,
    // not from the agent's current live position.  Using the live position for
    // every future entry makes each later departure progressively earlier; as
    // soon as the preceding move completes, all overdue moves then fire in a
    // burst (the former Palme-route speed-up).
    FVector Start = Runtime.Agent.IsValid()
        ? Runtime.Agent->GetActorLocation() : FVector::ZeroVector;
    const FName StableKey = Runtime.Profile.SocialGroupId.IsNone()
        ? Runtime.Profile.EntityId : Runtime.Profile.SocialGroupId;
    for (int32 Index = Runtime.NextTimelineIndex - 1; Index >= 0; --Index)
    {
        const FTMOPPersonTimelineEntry& Previous = Runtime.Profile.Timeline[Index];
        if (Previous.LocationType == ETMOPPersonLocationType::WorldTransform)
        {
            Start = Previous.WorldTransform.GetLocation();
            break;
        }
        if (Previous.LocationType == ETMOPPersonLocationType::Anchor)
            if (ATMOPHistoricalAnchor* PreviousAnchor =
                Anchors->FindAnchor(Previous.TargetAnchorId))
            {
                Start = PreviousAnchor->GetPlacementLocation(StableKey);
                break;
            }
    }

    TArray<FName> RouteAnchorIds = Entry.PassAnchorIds;
    RouteAnchorIds.Add(Entry.TargetAnchorId);
    double TotalPathLength = 0.0;
    for (const FName AnchorId : RouteAnchorIds)
    {
        ATMOPHistoricalAnchor* Target = Anchors->FindAnchor(AnchorId);
        if (!IsValid(Target)) return 0;
        const FVector TargetLocation = Target->GetPlacementLocation(StableKey);
        double SegmentLength = FVector::Dist2D(Start, TargetLocation);
        UNavigationSystemV1::GetPathLength(GetWorld(), Start, TargetLocation,
            SegmentLength, nullptr, nullptr);
        TotalPathLength += SegmentLength;
        Start = TargetLocation;
    }
    const float Speed = Entry.TravelSpeedOverrideCmPerSecond > 0.0f
        ? Entry.TravelSpeedOverrideCmPerSecond
        : Runtime.Profile.MovementProfile.NormalWalkSpeed *
            Runtime.Profile.MovementProfile.PersonalSpeedMultiplier;
    return Speed > KINDA_SMALL_NUMBER
        ? FMath::CeilToInt(TotalPathLength / Speed) : 0;
}

ATMOPGroupDirector* ATMOPPersonRegistryDirector::FindGroupDirector() const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It) return *It;
    return nullptr;
}

bool ATMOPPersonRegistryDirector::HasValidGroupTable() const
{
    return IsValid(GroupDefinitionTable) &&
        GroupDefinitionTable->GetRowStruct() ==
            FTMOPGroupProfileRow::StaticStruct();
}

const FTMOPGroupProfileRow* ATMOPPersonRegistryDirector::FindGroupRow(
    const FName GroupId) const
{
    if (!HasValidGroupTable() || GroupId.IsNone()) return nullptr;
    return GroupDefinitionTable->FindRow<FTMOPGroupProfileRow>(
        GroupId, TEXT("TMOPGroupLookup"), false);
}

bool ATMOPPersonRegistryDirector::IsGroupLeader(
    const FTMOPPersonProfileRow& Profile,
    const ATMOPHistoricalAgent* Agent) const
{
    const FName GroupId = IsValid(Agent)
        ? Agent->SocialGroupId : Profile.SocialGroupId;
    if (GroupId.IsNone()) return false;
    if (const ATMOPGroupDirector* Groups = FindGroupDirector())
    {
        bool bFound = false;
        const FTMOPGroupSnapshot Snapshot =
            Groups->GetGroupSnapshot(GroupId, bFound);
        if (bFound)
            return Snapshot.LeaderEntityId ==
                Profile.EntityId;
    }
    if (const FTMOPGroupProfileRow* Group = FindGroupRow(GroupId))
        return Group->LeaderEntityId == Profile.EntityId;
    return Profile.GroupLeaderEntityId == Profile.EntityId;
}

bool ATMOPPersonRegistryDirector::ShouldFollowGroupLeader(
    const FTMOPPersonProfileRow& Profile,
    const ATMOPHistoricalAgent* Agent) const
{
    const FName GroupId = IsValid(Agent)
        ? Agent->SocialGroupId : Profile.SocialGroupId;
    if (GroupId.IsNone()) return false;
    FName LeaderEntityId = Profile.GroupLeaderEntityId;
    if (const ATMOPGroupDirector* Groups = FindGroupDirector())
    {
        bool bFound = false;
        const FTMOPGroupSnapshot Snapshot =
            Groups->GetGroupSnapshot(GroupId, bFound);
        if (bFound)
            LeaderEntityId = Snapshot.LeaderEntityId;
    }
    if (const FTMOPGroupProfileRow* Group = FindGroupRow(GroupId))
        return Group->bUseLeaderTimeline &&
            LeaderEntityId != Profile.EntityId;
    return Profile.bFollowGroupLeaderSchedule &&
        LeaderEntityId != Profile.EntityId;
}

void ATMOPPersonRegistryDirector::ApplyGroupTableMemberships()
{
    if (!HasValidGroupTable()) return;
    for (const FName RowName : GroupDefinitionTable->GetRowNames())
    {
        const FTMOPGroupProfileRow* Group =
            GroupDefinitionTable->FindRow<FTMOPGroupProfileRow>(
                RowName, TEXT("TMOPApplyGroupMemberships"), false);
        if (Group == nullptr || !Group->bCreateAtScenarioStart) continue;
        for (const FName MemberId : Group->MemberEntityIds)
        {
            FPersonRuntime* Runtime = RuntimePeople.Find(MemberId);
            if (Runtime == nullptr) continue;
            Runtime->Profile.SocialGroupId = Group->GroupId;
            Runtime->Profile.GroupLeaderEntityId = Group->LeaderEntityId;
            Runtime->Profile.GroupFormation = Group->Formation;
            Runtime->Profile.GroupFormationSpacingCm =
                Group->FormationSpacingCm;
            Runtime->Profile.bFollowGroupLeaderSchedule =
                Group->bUseLeaderTimeline &&
                MemberId != Group->LeaderEntityId;
        }
    }
}

void ATMOPPersonRegistryDirector::RebuildGroupsFromGroupTable()
{
    ATMOPGroupDirector* Groups = FindGroupDirector();
    if (!IsValid(Groups))
    {
        UE_LOG(LogTemp, Warning, TEXT(
            "TMOP People: DT_TMOP_Groups is assigned but no "
            "TMOPGroupDirector is in the level."));
        return;
    }

    TArray<FString> Errors;
    if (!ValidateGroupTable(Errors))
    {
        for (const FString& Error : Errors)
            UE_LOG(LogTemp, Error, TEXT("TMOP Groups: %s"), *Error);
        return;
    }

    for (const FName RowName : GroupDefinitionTable->GetRowNames())
    {
        const FTMOPGroupProfileRow* Row =
            GroupDefinitionTable->FindRow<FTMOPGroupProfileRow>(
                RowName, TEXT("TMOPCreateGroups"), false);
        if (Row == nullptr || !Row->bCreateAtScenarioStart ||
            Groups->DoesGroupExist(Row->GroupId)) continue;
        FTMOPGroupDefinition Definition;
        Definition.GroupId = Row->GroupId;
        Definition.MemberEntityIds = Row->MemberEntityIds;
        Definition.LeaderEntityId = Row->LeaderEntityId;
        Definition.Formation = Row->Formation;
        Definition.FormationSpacing = Row->FormationSpacingCm;
        Groups->CreateGroup(Definition);
    }
    Groups->RefreshWaitingGroups();
}

void ATMOPPersonRegistryDirector::RebuildGroupsFromPeople()
{
    ATMOPGroupDirector* Groups = FindGroupDirector();
    if (!IsValid(Groups))
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP People: group memberships exist but no TMOPGroupDirector is in the level."));
        return;
    }
    TMap<FName, FTMOPGroupDefinition> Definitions;
    for (const TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
    {
        const FTMOPPersonProfileRow& Profile = Pair.Value.Profile;
        if (Profile.SocialGroupId.IsNone()) continue;
        FTMOPGroupDefinition& Definition = Definitions.FindOrAdd(Profile.SocialGroupId);
        Definition.GroupId = Profile.SocialGroupId;
        Definition.MemberEntityIds.AddUnique(Profile.EntityId);
        Definition.LeaderEntityId = Profile.GroupLeaderEntityId;
        Definition.Formation = Profile.GroupFormation;
        Definition.FormationSpacing = Profile.GroupFormationSpacingCm;
    }
    for (const TPair<FName, FTMOPGroupDefinition>& Pair : Definitions)
        if (!Groups->DoesGroupExist(Pair.Key)) Groups->CreateGroup(Pair.Value);
    Groups->RefreshWaitingGroups();
}

bool ATMOPPersonRegistryDirector::IsAgentBusy(const ATMOPHistoricalAgent* Agent) const
{
    if (!IsValid(Agent)) return false;
    if (IsValid(Agent->ActionExecutor) &&
        Agent->ActionExecutor->IsExecutingAction())
        return true;
    // GroupDirector navigation does not run through ActionExecutor. Without
    // this check the next overdue timeline entry could replace a group move
    // before the leader and followers reached their anchor.
    const AAIController* AI = Cast<AAIController>(Agent->GetController());
    return IsValid(AI) &&
        AI->GetMoveStatus() != EPathFollowingStatus::Idle;
}

ATMOPVehicleBase* ATMOPPersonRegistryDirector::FindVehicle(const FName VehicleId) const
{
    if (VehicleId.IsNone()) return nullptr;
    UTMOPWorldSubsystem* WorldRegistry = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>() : nullptr;
    if (WorldRegistry != nullptr)
        if (ATMOPVehicleBase* Registered =
            Cast<ATMOPVehicleBase>(WorldRegistry->FindWorldObject(VehicleId))) return Registered;
    UWorld* World = GetWorld();
    if (World == nullptr) return nullptr;
    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
        if (It->VehicleId == VehicleId) return *It;
    return nullptr;
}

ATMOPHistoricalAgent* ATMOPPersonRegistryDirector::FindSpawnedPerson(const FName EntityId) const
{
    const FPersonRuntime* Runtime = RuntimePeople.Find(EntityId);
    return Runtime != nullptr ? Runtime->Agent.Get() : nullptr;
}

FText ATMOPPersonRegistryDirector::GetPersonDialog(
    const FName EntityId, const bool bAfterShot) const
{
    if (const FPersonRuntime* Runtime = RuntimePeople.Find(EntityId))
        return bAfterShot
            ? Runtime->Profile.Dialog.AfterShot
            : Runtime->Profile.Dialog.BeforeShot;
    if (IsValid(PersonProfileTable))
        for (const FName RowName : PersonProfileTable->GetRowNames())
            if (const FTMOPPersonProfileRow* Row =
                PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
                    RowName, TEXT("TMOPPersonDialog"), false))
                if (Row->EntityId == EntityId)
                    return bAfterShot
                        ? Row->Dialog.AfterShot
                        : Row->Dialog.BeforeShot;
    return FText::GetEmpty();
}

bool ATMOPPersonRegistryDirector::ValidatePeopleTable(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!IsValid(PersonProfileTable) ||
        PersonProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct())
    {
        OutErrors.Add(TEXT("Person Profile Table is missing or has the wrong row structure."));
        return false;
    }

    TSet<FName> EntityIds;
    for (const FName RowName : PersonProfileTable->GetRowNames())
    {
        const FTMOPPersonProfileRow* Row = PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
            RowName, TEXT("ValidatePeople"), false);
        if (Row == nullptr) continue;
        const FString Prefix = FString::Printf(TEXT("Row '%s'"), *RowName.ToString());
        if (Row->EntityId.IsNone()) OutErrors.Add(Prefix + TEXT(" has no EntityId."));
        if (RowName != Row->EntityId) OutErrors.Add(Prefix + TEXT(" Row Name must equal EntityId."));
        if (EntityIds.Contains(Row->EntityId)) OutErrors.Add(Prefix + TEXT(" duplicates EntityId."));
        EntityIds.Add(Row->EntityId);
        if (Row->bSpawnInSimulation && Row->Timeline.IsEmpty())
            OutErrors.Add(Prefix + TEXT(" is enabled for simulation but Timeline is empty."));
        TSet<FName> EntryIds;
        int32 PreviousSecond = INDEX_NONE;
        const UTMOPAnchorSubsystem* AnchorRegistry =
            GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>()
            : nullptr;
        for (int32 Index = 0; Index < Row->Timeline.Num(); ++Index)
        {
            const FTMOPPersonTimelineEntry& Entry = Row->Timeline[Index];
            if (Entry.EntryId.IsNone() || EntryIds.Contains(Entry.EntryId))
                OutErrors.Add(Prefix + FString::Printf(TEXT(" has missing/duplicate EntryId at Timeline[%d]."), Index));
            EntryIds.Add(Entry.EntryId);

            if (AnchorRegistry != nullptr)
            {
                if (!Entry.TargetAnchorId.IsNone() &&
                    AnchorRegistry->FindAnchor(Entry.TargetAnchorId) == nullptr)
                    OutErrors.Add(Prefix + FString::Printf(
                        TEXT(" Timeline[%d] references missing TargetAnchorId '%s'."),
                        Index, *Entry.TargetAnchorId.ToString()));
                for (const FName PassAnchorId : Entry.PassAnchorIds)
                    if (!PassAnchorId.IsNone() &&
                        AnchorRegistry->FindAnchor(PassAnchorId) == nullptr)
                        OutErrors.Add(Prefix + FString::Printf(
                            TEXT(" Timeline[%d] references missing PassAnchorId '%s'."),
                            Index, *PassAnchorId.ToString()));
                if (!Entry.DrivingDestinationAnchorId.IsNone() &&
                    AnchorRegistry->FindAnchor(
                        Entry.DrivingDestinationAnchorId) == nullptr)
                    OutErrors.Add(Prefix + FString::Printf(
                        TEXT(" Timeline[%d] references missing DrivingDestinationAnchorId '%s'."),
                        Index, *Entry.DrivingDestinationAnchorId.ToString()));
            }

            if (Index > 0)
            {
                const FTMOPPersonTimelineEntry& Previous =
                    Row->Timeline[Index - 1];
                const bool bSameAbsoluteTime =
                    Entry.TimingMode == ETMOPEventTimingMode::Absolute &&
                    Previous.TimingMode == ETMOPEventTimingMode::Absolute &&
                    Entry.Time.ToSecondsFromMidnight() ==
                        Previous.Time.ToSecondsFromMidnight();
                const bool bSameSharedEventTime =
                    Entry.TimingMode == ETMOPEventTimingMode::Relative &&
                    Previous.TimingMode == ETMOPEventTimingMode::Relative &&
                    Entry.SharedEventId == Previous.SharedEventId &&
                    Entry.EventOffsetSeconds == Previous.EventOffsetSeconds;
                const bool bTwoMovements =
                    Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor &&
                    Previous.Action == ETMOPPersonTimelineAction::MoveToAnchor;
                if ((bSameAbsoluteTime || bSameSharedEventTime) &&
                    bTwoMovements && !Previous.bTimeIsArrival)
                    OutErrors.Add(Prefix + FString::Printf(
                        TEXT(" Timeline[%d] and Timeline[%d] start two movements at the same resolved time; the second destination needs a later time."),
                        Index - 1, Index));
            }
            if (Entry.TimingMode == ETMOPEventTimingMode::Relative &&
                Entry.SharedEventId.IsNone())
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] uses Relative timing but has no SharedEventId."), Index));
            if (Entry.TimingMode ==
                    ETMOPEventTimingMode::RelativeToPreviousEntry &&
                Index == 0)
                OutErrors.Add(Prefix + TEXT(
                    " Timeline[0] cannot be Relative to Previous Entry."));
            if (Entry.bTimeIsArrival &&
                Entry.Action != ETMOPPersonTimelineAction::MoveToAnchor)
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] marks arrival time but is not MoveToAnchor."), Index));
            if ((Entry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
                 Entry.Action == ETMOPPersonTimelineAction::ExitVehicle ||
                 Entry.Action == ETMOPPersonTimelineAction::BeginDriving) &&
                Entry.TargetEntityId.IsNone())
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] requires a target Vehicle ID."), Index));
            if ((Entry.Action == ETMOPPersonTimelineAction::JoinGroup ||
                 Entry.Action == ETMOPPersonTimelineAction::SplitGroup ||
                 Entry.Action == ETMOPPersonTimelineAction::DissolveGroup ||
                 Entry.Action == ETMOPPersonTimelineAction::SetGroupLeader) &&
                Entry.TargetGroupId.IsNone())
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] requires a target Group ID."), Index));
            if (Entry.Action == ETMOPPersonTimelineAction::CreateGroup &&
                (Entry.GroupDefinition.GroupId.IsNone() ||
                 Entry.GroupDefinition.MemberEntityIds.IsEmpty()))
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] requires a complete Group Definition."), Index));
            if (Entry.Action == ETMOPPersonTimelineAction::SplitGroup &&
                Entry.SplitGroupDefinitions.Num() < 2)
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] Split Group requires at least two child groups."), Index));
            if (Entry.Action == ETMOPPersonTimelineAction::SetGroupLeader &&
                Entry.NewGroupLeaderEntityId.IsNone())
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" Timeline[%d] requires a new Group Leader Entity ID."), Index));
            const int32 Second = Entry.Time.ToSecondsFromMidnight();
            if (Entry.TimingMode == ETMOPEventTimingMode::Absolute &&
                PreviousSecond > Second)
                OutErrors.Add(Prefix + TEXT(" absolute Timeline entries are not chronological; array order is authoritative."));
            if (Entry.TimingMode == ETMOPEventTimingMode::Absolute) PreviousSecond = Second;
        }
        int32 FirstSimulationIndex = INDEX_NONE;
        for (int32 Index = 0; Index < Row->Timeline.Num(); ++Index)
            if (Row->Timeline[Index].Usage != ETMOPPersonTimelineUsage::DocumentationOnly)
            {
                FirstSimulationIndex = Index;
                break;
            }
        if (Row->bSpawnInSimulation && FirstSimulationIndex == INDEX_NONE)
            OutErrors.Add(Prefix + TEXT(" is enabled for simulation but has only documentation entries."));
        if (FirstSimulationIndex != INDEX_NONE &&
            Row->Timeline[FirstSimulationIndex].Action != ETMOPPersonTimelineAction::InitialPlacement &&
            Row->Timeline[FirstSimulationIndex].Action != ETMOPPersonTimelineAction::Spawn)
            OutErrors.Add(Prefix + FString::Printf(
                TEXT(" Timeline[%d] is the first simulation entry and must be InitialPlacement or Spawn."),
                FirstSimulationIndex));
        TSet<FName> SpeechLineIds;
        for (int32 Index = 0; Index < Row->AutomaticSpeech.Num(); ++Index)
        {
            const FTMOPTimedSpeechLine& Line = Row->AutomaticSpeech[Index];
            if (Line.LineId.IsNone() || SpeechLineIds.Contains(Line.LineId))
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" has missing/duplicate LineId at AutomaticSpeech[%d]."),
                    Index));
            SpeechLineIds.Add(Line.LineId);
            if (Line.Text.IsEmpty())
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" AutomaticSpeech[%d] has no Spoken Text."), Index));
            if (Line.TimingMode ==
                    ETMOPSpeechTimingMode::RelativeToSharedEvent &&
                Line.SharedEventId.IsNone())
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" AutomaticSpeech[%d] is relative but has no Shared Event ID."),
                    Index));
            if (Line.TimingMode ==
                    ETMOPSpeechTimingMode::RelativeToPreviousLine &&
                Index == 0)
                OutErrors.Add(Prefix + TEXT(
                    " AutomaticSpeech[0] cannot be Relative to Previous Line."));
        }
        if (!HasValidGroupTable() && !Row->SocialGroupId.IsNone() &&
            Row->GroupLeaderEntityId.IsNone())
            OutErrors.Add(Prefix + TEXT(" belongs to a group but has no GroupLeaderEntityId."));
    }
    return OutErrors.IsEmpty();
}

bool ATMOPPersonRegistryDirector::ValidateAppearanceAssetTable(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!IsValid(AppearanceAssetTable) ||
        AppearanceAssetTable->GetRowStruct() !=
            FTMOPAppearanceAssetRow::StaticStruct())
    {
        OutErrors.Add(TEXT(
            "Appearance Asset Table is missing or has the wrong row structure."));
        return false;
    }
    TSet<FName> CatalogIds;
    for (const FName RowName : AppearanceAssetTable->GetRowNames())
    {
        const FTMOPAppearanceAssetRow* Row =
            AppearanceAssetTable->FindRow<FTMOPAppearanceAssetRow>(
                RowName, TEXT("ValidateAppearanceAssets"), false);
        if (Row == nullptr) continue;
        const FString Prefix = FString::Printf(
            TEXT("Appearance row '%s'"), *RowName.ToString());
        if (Row->CatalogId.IsNone())
            OutErrors.Add(Prefix + TEXT(" has no CatalogId."));
        if (Row->CatalogId != RowName)
            OutErrors.Add(Prefix + TEXT(" Row Name must equal CatalogId."));
        if (CatalogIds.Contains(Row->CatalogId))
            OutErrors.Add(Prefix + TEXT(" duplicates CatalogId."));
        CatalogIds.Add(Row->CatalogId);
        if (Row->Mesh.IsNull())
            OutErrors.Add(Prefix + TEXT(" has no Skeletal Mesh."));
        if (Row->EarliestYear > 1986 || Row->LatestYear < 1986)
            OutErrors.Add(Prefix + TEXT(" is not valid for 1986."));
        if (Row->MaximumAge > 0 && Row->MaximumAge < Row->MinimumAge)
            OutErrors.Add(Prefix + TEXT(" has MaximumAge below MinimumAge."));
    }
    return OutErrors.IsEmpty();
}

void ATMOPPersonRegistryDirector::ValidateAppearanceAssetTableInEditor()
{
    TArray<FString> Errors;
    if (ValidateAppearanceAssetTable(Errors))
    {
        const int32 RowCount = IsValid(AppearanceAssetTable)
            ? AppearanceAssetTable->GetRowNames().Num() : 0;
        UE_LOG(LogTemp, Display, TEXT(
            "TMOP Appearance Asset Table validation passed: %d rows."), RowCount);
        return;
    }

    UE_LOG(LogTemp, Error, TEXT(
        "TMOP Appearance Asset Table validation failed with %d error(s):"),
        Errors.Num());
    for (const FString& Error : Errors)
        UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
}

bool ATMOPPersonRegistryDirector::ValidateGroupTable(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!HasValidGroupTable())
    {
        OutErrors.Add(TEXT(
            "Group Definition Table is missing or has the wrong row structure."));
        return false;
    }

    TSet<FName> GroupIds;
    TMap<FName, FName> InitialMemberships;
    TSet<FName> KnownPersonIds;
    if (IsValid(PersonProfileTable) &&
        PersonProfileTable->GetRowStruct() ==
            FTMOPPersonProfileRow::StaticStruct())
    {
        for (const FName PersonRowName : PersonProfileTable->GetRowNames())
            if (const FTMOPPersonProfileRow* Person =
                PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
                    PersonRowName, TEXT("ValidateGroupPeople"), false))
                KnownPersonIds.Add(Person->EntityId);
    }
    for (const FName RowName : GroupDefinitionTable->GetRowNames())
    {
        const FTMOPGroupProfileRow* Row =
            GroupDefinitionTable->FindRow<FTMOPGroupProfileRow>(
                RowName, TEXT("ValidateGroups"), false);
        if (Row == nullptr) continue;
        const FString Prefix = FString::Printf(
            TEXT("Row '%s'"), *RowName.ToString());
        if (Row->GroupId.IsNone())
            OutErrors.Add(Prefix + TEXT(" has no GroupId."));
        if (RowName != Row->GroupId)
            OutErrors.Add(Prefix + TEXT(" Row Name must equal GroupId."));
        if (GroupIds.Contains(Row->GroupId))
            OutErrors.Add(Prefix + TEXT(" duplicates GroupId."));
        GroupIds.Add(Row->GroupId);
        if (Row->MemberEntityIds.IsEmpty())
            OutErrors.Add(Prefix + TEXT(" has no members."));
        if (Row->LeaderEntityId.IsNone())
            OutErrors.Add(Prefix + TEXT(" has no LeaderEntityId."));
        if (!Row->MemberEntityIds.Contains(Row->LeaderEntityId))
            OutErrors.Add(Prefix + TEXT(
                " LeaderEntityId is not included in MemberEntityIds."));

        TSet<FName> MembersInRow;
        for (const FName MemberId : Row->MemberEntityIds)
        {
            if (MemberId.IsNone() || MembersInRow.Contains(MemberId))
                OutErrors.Add(Prefix + TEXT(
                    " has an empty or duplicate MemberEntityId."));
            MembersInRow.Add(MemberId);
            if (!KnownPersonIds.Contains(MemberId))
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" references missing person '%s'."),
                    *MemberId.ToString()));
            if (!Row->bCreateAtScenarioStart) continue;
            if (const FName* Existing = InitialMemberships.Find(MemberId))
                OutErrors.Add(Prefix + FString::Printf(
                    TEXT(" person '%s' is already in initial group '%s'."),
                    *MemberId.ToString(), *Existing->ToString()));
            else
                InitialMemberships.Add(MemberId, Row->GroupId);
        }
    }
    return OutErrors.IsEmpty();
}
