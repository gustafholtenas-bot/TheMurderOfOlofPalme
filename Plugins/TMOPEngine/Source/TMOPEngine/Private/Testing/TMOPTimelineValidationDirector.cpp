#include "Testing/TMOPTimelineValidationDirector.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "AIController.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "People/TMOPPersonProfileTypes.h"
#include "Time/TMOPClockSubsystem.h"

namespace
{
FString CsvEscape(const FString& Value)
{
    FString Escaped = Value;
    Escaped.ReplaceInline(TEXT("\""), TEXT("\"\""));
    return FString::Printf(TEXT("\"%s\""), *Escaped);
}

FString SeverityText(const ETMOPTimelineValidationSeverity Severity)
{
    switch (Severity)
    {
    case ETMOPTimelineValidationSeverity::Passed: return TEXT("Passed");
    case ETMOPTimelineValidationSeverity::Warning: return TEXT("Warning");
    default: return TEXT("Error");
    }
}
}

ATMOPTimelineValidationDirector::ATMOPTimelineValidationDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;
}

void ATMOPTimelineValidationDirector::BeginPlay()
{
    Super::BeginPlay();
    for (TActorIterator<ATMOPPersonRegistryDirector> It(GetWorld()); It; ++It)
    {
        PeopleDirector = *It;
        PeopleDirector->OnTimelineEntryApplied.AddUObject(
            this,
            &ATMOPTimelineValidationDirector::HandlePersonTimelineApplied);
        break;
    }
    if (bStartAutomatically) StartValidation();
}

void ATMOPTimelineValidationDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    const bool bWasActive = bValidationActive;
    bValidationActive = false;
    if (bExportOnEndPlay && bWasActive) ExportReports();
    if (PeopleDirector.IsValid())
        PeopleDirector->OnTimelineEntryApplied.RemoveAll(this);
    for (TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
        if (Pair.Value.Executor.IsValid())
            Pair.Value.Executor->OnActionValidationEvent.RemoveAll(this);
    TrackedAgents.Reset();
    Super::EndPlay(EndPlayReason);
}

void ATMOPTimelineValidationDirector::StartValidation()
{
    Records.Reset();
    TrackedAgents.Reset();
    SampleAccumulator = 0.0f;
    bValidationActive = true;
    DiscoverAgents();
    UE_LOG(LogTemp, Display, TEXT("TMOP timeline validation started."));
}

void ATMOPTimelineValidationDirector::StopValidation(const bool bExportReports)
{
    bValidationActive = false;
    if (bExportReports) ExportReports();
}

void ATMOPTimelineValidationDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bValidationActive) return;
    SampleAccumulator += DeltaSeconds;
    if (SampleAccumulator < SampleIntervalSeconds) return;
    const float SampleDelta = SampleAccumulator;
    SampleAccumulator = 0.0f;
    DiscoverAgents();
    SampleAgents(SampleDelta);
}

void ATMOPTimelineValidationDirector::DiscoverAgents()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPHistoricalAgent> It(World); It; ++It)
    {
        ATMOPHistoricalAgent* Agent = *It;
        if (!IsValid(Agent) || !IsValid(Agent->ActionExecutor) ||
            !IsValid(Agent->EntityIdentity)) continue;
        const FName EntityId = Agent->EntityIdentity->GetEntityId();
        if (EntityId.IsNone() || TrackedAgents.Contains(EntityId)) continue;

        FTrackedAgent Tracked;
        Tracked.Agent = Agent;
        Tracked.Executor = Agent->ActionExecutor;
        Tracked.LastLocation = Agent->GetActorLocation();
        Tracked.ActiveEntryId =
            Agent->ActionExecutor->GetCurrentEntryId();
        Agent->ActionExecutor->OnActionValidationEvent.AddUObject(
            this, &ATMOPTimelineValidationDirector::HandleActionValidation);
        TrackedAgents.Add(EntityId, MoveTemp(Tracked));

        FTMOPTimelineValidationRecord Spawn;
        Spawn.EntityId = EntityId;
        Spawn.Event = TEXT("Spawned");
        Spawn.ActualSecond = GetSimulationSecond();
        Spawn.ActualLocation = Agent->GetActorLocation();
        Spawn.Message = TEXT("Agent discovered in simulation.");
        AddRecord(Spawn);
    }
}

void ATMOPTimelineValidationDirector::SampleAgents(const float DeltaSeconds)
{
    for (TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
    {
        FTrackedAgent& Tracked = Pair.Value;
        ATMOPHistoricalAgent* Agent = Tracked.Agent.Get();
        UTMOPActionExecutorComponent* Executor = Tracked.Executor.Get();
        if (!IsValid(Agent) || !IsValid(Executor)) continue;

        FVector Target;
        bool bMoving = Executor->TryGetActiveMoveTarget(Target);
        if (!bMoving && Tracked.bRegistryManagedMove &&
            Agent->ActivityState == ETMOPAgentActivityState::Walking)
        {
            if (const AAIController* Controller =
                Cast<AAIController>(Agent->GetController()))
            {
                Target = Controller->GetImmediateMoveDestination();
                bMoving = !Target.IsNearlyZero();
            }
        }
        if (!bMoving)
        {
            Tracked.StationarySeconds = 0.0f;
            Tracked.bStuckReportedForCurrentMove = false;
            Tracked.LastLocation = Agent->GetActorLocation();
            continue;
        }

        const float MovedCm = FVector::Dist2D(
            Tracked.LastLocation, Agent->GetActorLocation());
        Tracked.LastLocation = Agent->GetActorLocation();
        if (MovedCm <= StationaryDistanceCm)
            Tracked.StationarySeconds += DeltaSeconds;
        else
            Tracked.StationarySeconds = 0.0f;

        if (Tracked.StationarySeconds >= StuckAfterSeconds &&
            !Tracked.bStuckReportedForCurrentMove)
        {
            Tracked.bStuckReportedForCurrentMove = true;
            FTMOPTimelineValidationRecord Record;
            Record.EntityId = Pair.Key;
            Record.EntryId = Tracked.ActiveEntryId;
            Record.Event = TEXT("Stuck");
            Record.ActualSecond = GetSimulationSecond();
            Record.ActualLocation = Agent->GetActorLocation();
            Record.DistanceToTargetCm = FVector::Dist2D(
                Agent->GetActorLocation(), Target);
            Record.Severity = ETMOPTimelineValidationSeverity::Error;
            Record.Message = FString::Printf(
                TEXT("Moved no more than %.0f cm per sample for %.1f seconds."),
                StationaryDistanceCm, Tracked.StationarySeconds);
            AddRecord(Record);
        }
    }
}

void ATMOPTimelineValidationDirector::HandleActionValidation(
    UTMOPActionExecutorComponent* Executor,
    const FTMOPScheduleEntry& Entry,
    const FTMOPTime ScheduledTime,
    const ETMOPActionExecutionState State)
{
    if (!bValidationActive || !IsValid(Executor)) return;
    const FName EntityId = GetEntityId(Executor);
    FTrackedAgent* Tracked = TrackedAgents.Find(EntityId);
    AActor* OwnerActor = Executor->GetOwner();

    FTMOPTimelineValidationRecord Record;
    Record.EntityId = EntityId;
    Record.EntryId = Entry.EntryId;
    Record.TargetAnchorId = Entry.TargetAnchorId;
    Record.PlannedSecond = ScheduledTime.ToSecondsFromMidnight();
    Record.ActualSecond = GetSimulationSecond();
    Record.TimeDeviationSeconds =
        Record.PlannedSecond > 0
        ? Record.ActualSecond - Record.PlannedSecond : 0.0f;
    Record.ActualLocation = IsValid(OwnerActor)
        ? OwnerActor->GetActorLocation() : FVector::ZeroVector;

    if (State == ETMOPActionExecutionState::Executing ||
        State == ETMOPActionExecutionState::WaitingForArrival)
    {
        if (Tracked != nullptr)
        {
            Tracked->ActiveEntryId = Entry.EntryId;
            Tracked->ActiveTargetAnchorId = Entry.TargetAnchorId;
            Tracked->ActivePlannedSecond = Record.PlannedSecond;
            Tracked->bRegistryManagedMove = false;
            Tracked->StationarySeconds = 0.0f;
            Tracked->bStuckReportedForCurrentMove = false;
        }
        if (State == ETMOPActionExecutionState::Executing)
        {
            Record.Event = TEXT("Started");
            const float AbsDeviation = FMath::Abs(Record.TimeDeviationSeconds);
            if (AbsDeviation > TimingErrorSeconds)
                Record.Severity = ETMOPTimelineValidationSeverity::Error;
            else if (AbsDeviation > TimingWarningSeconds)
                Record.Severity = ETMOPTimelineValidationSeverity::Warning;
            Record.Message = TEXT("Timeline action started.");
            AddRecord(Record);
        }
        return;
    }

    if (State != ETMOPActionExecutionState::Completed &&
        State != ETMOPActionExecutionState::Failed) return;

    Record.Event = State == ETMOPActionExecutionState::Completed
        ? TEXT("Completed") : TEXT("Failed");
    if (!Entry.TargetAnchorId.IsNone() && IsValid(OwnerActor))
    {
        UGameInstance* GI = GetGameInstance();
        UTMOPAnchorSubsystem* Anchors = GI != nullptr
            ? GI->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
        const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Entry.TargetAnchorId) : nullptr;
        if (IsValid(Anchor))
            Record.DistanceToTargetCm = FVector::Dist2D(
                OwnerActor->GetActorLocation(), Anchor->GetAnchorLocation());
        else
        {
            Record.Severity = ETMOPTimelineValidationSeverity::Error;
            Record.Message = TEXT("Target anchor does not exist.");
        }
    }

    if (State == ETMOPActionExecutionState::Failed)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Error;
        Record.Message = TEXT("Action executor reported failure.");
    }
    else if (Record.DistanceToTargetCm > ArrivalWarningDistanceCm)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Warning;
        Record.Message = TEXT("Action completed outside the expected anchor radius.");
    }
    else if (Record.Message.IsEmpty())
        Record.Message = TEXT("Action completed successfully.");

    AddRecord(Record);
    if (Tracked != nullptr)
    {
        Tracked->ActiveEntryId = NAME_None;
        Tracked->ActiveTargetAnchorId = NAME_None;
        Tracked->ActivePlannedSecond = INDEX_NONE;
        Tracked->bRegistryManagedMove = false;
        Tracked->StationarySeconds = 0.0f;
    }
}

void ATMOPTimelineValidationDirector::HandlePersonTimelineApplied(
    const FName EntityId,
    const FTMOPPersonTimelineEntry& Entry,
    const int32 ResolvedSecond,
    const bool bSuccessful,
    const bool bCatchUp)
{
    if (!bValidationActive || bCatchUp) return;

    FTrackedAgent* Tracked = TrackedAgents.Find(EntityId);
    if (Tracked == nullptr)
    {
        DiscoverAgents();
        Tracked = TrackedAgents.Find(EntityId);
    }

    if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor &&
        Tracked != nullptr && Tracked->Executor.IsValid() &&
        Tracked->Executor->GetCurrentEntryId() == Entry.EntryId)
        return;

    FTMOPTimelineValidationRecord Record;
    Record.EntityId = EntityId;
    Record.EntryId = Entry.EntryId;
    Record.TargetAnchorId = Entry.TargetAnchorId;
    Record.PlannedSecond = ResolvedSecond;
    Record.ActualSecond = GetSimulationSecond();
    Record.TimeDeviationSeconds =
        Record.ActualSecond - Record.PlannedSecond;
    if (Tracked != nullptr && Tracked->Agent.IsValid())
        Record.ActualLocation = Tracked->Agent->GetActorLocation();

    if (!bSuccessful)
    {
        Record.Event = TEXT("Failed");
        Record.Severity = ETMOPTimelineValidationSeverity::Error;
        Record.Message = TEXT("People timeline entry could not be applied.");
    }
    else if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor)
    {
        Record.Event = TEXT("Started");
        Record.Message = TEXT("Group-managed movement started.");
        if (Tracked != nullptr)
        {
            Tracked->ActiveEntryId = Entry.EntryId;
            Tracked->ActiveTargetAnchorId = Entry.TargetAnchorId;
            Tracked->ActivePlannedSecond = ResolvedSecond;
            Tracked->bRegistryManagedMove = true;
            Tracked->StationarySeconds = 0.0f;
            Tracked->bStuckReportedForCurrentMove = false;
        }
    }
    else
    {
        Record.Event = TEXT("Applied");
        Record.Message = TEXT("People timeline entry applied successfully.");
    }
    AddRecord(Record);
}

void ATMOPTimelineValidationDirector::AddRecord(
    const FTMOPTimelineValidationRecord& Record)
{
    Records.Add(Record);
    if (Record.Severity != ETMOPTimelineValidationSeverity::Passed)
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP validation %s %s/%s: %s"),
            *SeverityText(Record.Severity),
            *Record.EntityId.ToString(), *Record.EntryId.ToString(),
            *Record.Message);
}

int32 ATMOPTimelineValidationDirector::GetSimulationSecond() const
{
    const UGameInstance* GI = GetGameInstance();
    const UTMOPClockSubsystem* Clock = GI != nullptr
        ? GI->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    return Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight() : INDEX_NONE;
}

FName ATMOPTimelineValidationDirector::GetEntityId(
    const UTMOPActionExecutorComponent* Executor) const
{
    const ATMOPHistoricalAgent* Agent = IsValid(Executor)
        ? Cast<ATMOPHistoricalAgent>(Executor->GetOwner()) : nullptr;
    return IsValid(Agent) && IsValid(Agent->EntityIdentity)
        ? Agent->EntityIdentity->GetEntityId() : NAME_None;
}

bool ATMOPTimelineValidationDirector::ExportReports()
{
    const FString Directory = FPaths::Combine(
        FPaths::ProjectSavedDir(), TEXT("TMOP"), TEXT("Validation"));
    IFileManager::Get().MakeDirectory(*Directory, true);
    const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    const FString Base = FPaths::Combine(
        Directory, FString::Printf(TEXT("TimelineValidation_%s"), *Stamp));

    FString Csv = TEXT("EntityId,EntryId,Event,TargetAnchorId,PlannedSecond,ActualSecond,TimeDeviationSeconds,DistanceToTargetCm,X,Y,Z,Severity,Message\n");
    TArray<TSharedPtr<FJsonValue>> JsonRecords;
    for (const FTMOPTimelineValidationRecord& R : Records)
    {
        Csv += FString::Printf(TEXT("%s,%s,%s,%s,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%s,%s\n"),
            *CsvEscape(R.EntityId.ToString()), *CsvEscape(R.EntryId.ToString()),
            *CsvEscape(R.Event), *CsvEscape(R.TargetAnchorId.ToString()),
            R.PlannedSecond, R.ActualSecond, R.TimeDeviationSeconds,
            R.DistanceToTargetCm, R.ActualLocation.X, R.ActualLocation.Y,
            R.ActualLocation.Z, *CsvEscape(SeverityText(R.Severity)),
            *CsvEscape(R.Message));

        TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("entityId"), R.EntityId.ToString());
        O->SetStringField(TEXT("entryId"), R.EntryId.ToString());
        O->SetStringField(TEXT("event"), R.Event);
        O->SetStringField(TEXT("targetAnchorId"), R.TargetAnchorId.ToString());
        O->SetNumberField(TEXT("plannedSecond"), R.PlannedSecond);
        O->SetNumberField(TEXT("actualSecond"), R.ActualSecond);
        O->SetNumberField(TEXT("timeDeviationSeconds"), R.TimeDeviationSeconds);
        O->SetNumberField(TEXT("distanceToTargetCm"), R.DistanceToTargetCm);
        O->SetStringField(TEXT("severity"), SeverityText(R.Severity));
        O->SetStringField(TEXT("message"), R.Message);
        JsonRecords.Add(MakeShared<FJsonValueObject>(O));
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("trackedAgents"), TrackedAgents.Num());
    Root->SetNumberField(TEXT("recordCount"), Records.Num());
    Root->SetArrayField(TEXT("records"), JsonRecords);
    FString Json;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
    FJsonSerializer::Serialize(Root, Writer);

    const bool bCsv = FFileHelper::SaveStringToFile(
        Csv, *(Base + TEXT(".csv")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bJson = FFileHelper::SaveStringToFile(
        Json, *(Base + TEXT(".json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP validation exported %d records to %s"),
        Records.Num(), *Directory);
    return bCsv && bJson;
}
