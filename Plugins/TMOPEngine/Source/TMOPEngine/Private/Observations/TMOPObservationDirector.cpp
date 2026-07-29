#include "Observations/TMOPObservationDirector.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Time/TMOPClockSubsystem.h"
#include "Vehicles/TMOPVehicleBase.h"

ATMOPObservationDirector::ATMOPObservationDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPObservationDirector::BeginPlay()
{
    Super::BeginPlay();

    ReloadObservationData();

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
        {
            Clock->OnSecondChanged.AddDynamic(
                this, &ATMOPObservationDirector::HandleSecondChanged);
            Clock->OnLoopRestarted.AddDynamic(
                this, &ATMOPObservationDirector::HandleLoopRestarted);
        }
    }

    ResolveCanonicalTimes();
}

void ATMOPObservationDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
        {
            Clock->OnSecondChanged.RemoveDynamic(
                this, &ATMOPObservationDirector::HandleSecondChanged);
            Clock->OnLoopRestarted.RemoveDynamic(
                this, &ATMOPObservationDirector::HandleLoopRestarted);
        }
    }

    Super::EndPlay(EndPlayReason);
}

int32 ATMOPObservationDirector::ReloadObservationData()
{
    LoadedObservations.Reset();
    LoadedLinks.Reset();

    if (IsValid(ObservationTable) &&
        ObservationTable->GetRowStruct() ==
            FTMOPObservationDefinition::StaticStruct())
    {
        static const FString Context(TEXT("TMOPObservationDirector"));
        TArray<FTMOPObservationDefinition*> Rows;
        ObservationTable->GetAllRows(Context, Rows);
        for (const FTMOPObservationDefinition* Row : Rows)
        {
            if (Row != nullptr && !Row->ObservationId.IsNone())
            {
                LoadedObservations.Add(Row->ObservationId, *Row);
            }
        }
    }

    if (IsValid(ObservationLinkTable) &&
        ObservationLinkTable->GetRowStruct() ==
            FTMOPObservationLinkDefinition::StaticStruct())
    {
        static const FString Context(TEXT("TMOPObservationLinks"));
        TArray<FTMOPObservationLinkDefinition*> Rows;
        ObservationLinkTable->GetAllRows(Context, Rows);
        for (const FTMOPObservationLinkDefinition* Row : Rows)
        {
            if (Row != nullptr && !Row->LinkId.IsNone())
            {
                LoadedLinks.Add(Row->LinkId, *Row);
            }
        }
    }

    for (const FTMOPObservationDefinition& Definition :
        ObservationDefinitions)
    {
        if (!Definition.ObservationId.IsNone())
        {
            LoadedObservations.Add(Definition.ObservationId, Definition);
        }
    }
    for (const FTMOPObservationLinkDefinition& Link : ObservationLinks)
    {
        if (!Link.LinkId.IsNone())
        {
            LoadedLinks.Add(Link.LinkId, Link);
        }
    }
    ResetObservationRuntime();

    TArray<FString> Errors;
    ValidateObservationData(Errors);
    for (const FString& Error : Errors)
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP observation data: %s"), *Error);
    }

    UE_LOG(LogTemp, Display,
        TEXT("TMOP observation director loaded %d observations and %d links."),
        LoadedObservations.Num(), LoadedLinks.Num());

    return LoadedObservations.Num();
}

int32 ATMOPObservationDirector::ResolveCanonicalTimes()
{
    int32 ResolvedCount = 0;
    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        LoadedObservations)
    {
        if (ResolveCanonicalTime(Pair.Key))
        {
            ++ResolvedCount;
        }
    }
    return ResolvedCount;
}

void ATMOPObservationDirector::ResetObservationRuntime()
{
    RuntimeObservations.Reset();
    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        LoadedObservations)
    {
        FTMOPObservationRuntime Runtime;
        Runtime.ObservationId = Pair.Key;
        Runtime.State = ETMOPObservationRuntimeState::Pending;
        if (!Pair.Value.bEnabled)
        {
            Runtime.State = ETMOPObservationRuntimeState::Invalid;
            Runtime.Diagnostic = TEXT("Observation is disabled.");
        }
        RuntimeObservations.Add(Pair.Key, Runtime);
    }
}

int32 ATMOPObservationDirector::ApplyBakedObservationRuntime(
    const TArray<FTMOPObservationRuntime>& BakedRuntime)
{
    int32 Applied = 0;
    for (const FTMOPObservationRuntime& Baked : BakedRuntime)
    {
        if (FTMOPObservationRuntime* Runtime =
            RuntimeObservations.Find(Baked.ObservationId))
        {
            *Runtime = Baked;
            ++Applied;
        }
    }
    return Applied;
}

bool ATMOPObservationDirector::ResolveCanonicalTime(
    const FName ObservationId)
{
    const FTMOPObservationDefinition* Definition =
        LoadedObservations.Find(ObservationId);
    FTMOPObservationRuntime* Runtime =
        RuntimeObservations.Find(ObservationId);

    if (Definition == nullptr || Runtime == nullptr ||
        !Definition->bEnabled)
    {
        return false;
    }

    int32 StartSecond = 0;
    if (Definition->TimingMode == ETMOPObservationTimingMode::Absolute)
    {
        StartSecond = Definition->CanonicalTime.ToSecondsFromMidnight();
    }
    else
    {
        UGameInstance* GameInstance = GetGameInstance();
        UTMOPHistoricalEventSubsystem* Events = GameInstance != nullptr
            ? GameInstance->GetSubsystem<UTMOPHistoricalEventSubsystem>()
            : nullptr;

        FTMOPHistoricalEventRuntime EventRuntime;
        if (Events == nullptr ||
            Definition->ReferenceSharedEventId.IsNone() ||
            !Events->TryGetEventRuntime(
                Definition->ReferenceSharedEventId, EventRuntime) ||
            !EventRuntime.bHasResolvedTime)
        {
            Runtime->Diagnostic = FString::Printf(
                TEXT("Waiting for shared event '%s' to resolve canonical time."),
                *Definition->ReferenceSharedEventId.ToString());
            return false;
        }

        StartSecond =
            EventRuntime.ResolvedTime.ToSecondsFromMidnight() +
            Definition->ReferenceOffsetSeconds;
    }

    Runtime->ResolvedCanonicalStartTime =
        FTMOPTime::FromSecondsFromMidnight(StartSecond);
    Runtime->ResolvedCanonicalEndTime =
        FTMOPTime::FromSecondsFromMidnight(
            StartSecond + FMath::Max(1, Definition->ObservationDurationSeconds));
    Runtime->bHasResolvedCanonicalTime = true;
    Runtime->Diagnostic = TEXT("Canonical time resolved.");
    return true;
}

void ATMOPObservationDirector::HandleSecondChanged(const FTMOPTime NewTime)
{
    ResolveCanonicalTimes();
    const int32 CurrentSecond = NewTime.ToSecondsFromMidnight();

    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        LoadedObservations)
    {
        const FTMOPObservationDefinition& Definition = Pair.Value;
        FTMOPObservationRuntime* Runtime =
            RuntimeObservations.Find(Pair.Key);

        if (Runtime == nullptr ||
            Runtime->State != ETMOPObservationRuntimeState::Pending ||
            !Runtime->bHasResolvedCanonicalTime)
        {
            continue;
        }

        const int32 StartSecond =
            Runtime->ResolvedCanonicalStartTime.ToSecondsFromMidnight();
        const int32 EndSecond =
            Runtime->ResolvedCanonicalEndTime.ToSecondsFromMidnight();

        if (CurrentSecond >= StartSecond && CurrentSecond <= EndSecond)
        {
            EvaluateObservationNow(Pair.Key);
        }
        else if (CurrentSecond > EndSecond)
        {
            Runtime->State = ETMOPObservationRuntimeState::Missed;
            Runtime->Diagnostic =
                TEXT("Canonical window ended without a valid observation.");
            OnObservationEvaluated.Broadcast(Pair.Key, Runtime->State);
        }
    }
}

void ATMOPObservationDirector::HandleLoopRestarted(
    const int32 NewLoopNumber, const FTMOPTime RestartTime)
{
    ResetObservationRuntime();
    ResolveCanonicalTimes();
}

bool ATMOPObservationDirector::EvaluateObservationNow(
    const FName ObservationId)
{
    const FTMOPObservationDefinition* Definition =
        LoadedObservations.Find(ObservationId);
    FTMOPObservationRuntime* Runtime =
        RuntimeObservations.Find(ObservationId);

    if (Definition == nullptr || Runtime == nullptr ||
        Runtime->State != ETMOPObservationRuntimeState::Pending)
    {
        return false;
    }

    if (EvaluateGeometry(*Definition, *Runtime))
    {
        Runtime->State = ETMOPObservationRuntimeState::Observed;
        Runtime->Diagnostic =
            TEXT("Observer and observed entity satisfied the canonical observation.");
        OnObservationEvaluated.Broadcast(ObservationId, Runtime->State);
        return true;
    }

    return false;
}

bool ATMOPObservationDirector::EvaluateGeometry(
    const FTMOPObservationDefinition& Definition,
    FTMOPObservationRuntime& Runtime) const
{
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPAnchorSubsystem* Anchors = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPAnchorSubsystem>()
        : nullptr;
    ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
        ? Anchors->FindAnchor(Definition.ObservationAnchorId)
        : nullptr;
    AActor* ObservedActor = FindEntityActor(Definition.ObservedEntityId);

    if (!IsValid(Anchor))
    {
        Runtime.Diagnostic = FString::Printf(
            TEXT("Observation anchor '%s' is unavailable."),
            *Definition.ObservationAnchorId.ToString());
        return false;
    }
    if (!IsValid(ObservedActor))
    {
        Runtime.Diagnostic = FString::Printf(
            TEXT("Observed entity '%s' is not active."),
            *Definition.ObservedEntityId.ToString());
        return false;
    }

    const FVector AnchorLocation = Anchor->GetAnchorLocation();
    Runtime.ObservedDistanceToAnchorCm = FVector::Dist(
        ObservedActor->GetActorLocation(), AnchorLocation);

    if (Definition.bRequireObservedEntityNearAnchor &&
        Runtime.ObservedDistanceToAnchorCm > Definition.ObservationRadiusCm)
    {
        Runtime.Diagnostic = TEXT("Observed entity is outside the observation area.");
        return false;
    }

    for (const FName ObserverId : Definition.ObserverEntityIds)
    {
        AActor* ObserverActor = FindEntityActor(ObserverId);
        if (!IsValid(ObserverActor))
        {
            continue;
        }

        const float ObserverDistance = FVector::Dist(
            ObserverActor->GetActorLocation(), AnchorLocation);
        if (Definition.bRequireObserverNearAnchor &&
            ObserverDistance > Definition.ObservationRadiusCm)
        {
            continue;
        }

        if (Definition.bRequiresLineOfSight && GetWorld() != nullptr)
        {
            FCollisionQueryParams QueryParams(
                SCENE_QUERY_STAT(TMOPObservationLineOfSight), false);
            QueryParams.AddIgnoredActor(ObserverActor);
            QueryParams.AddIgnoredActor(ObservedActor);

            FHitResult Hit;
            if (GetWorld()->LineTraceSingleByChannel(
                Hit,
                ObserverActor->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f),
                ObservedActor->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f),
                ECC_Visibility,
                QueryParams))
            {
                continue;
            }
        }

        Runtime.SuccessfulObserverEntityId = ObserverId;
        Runtime.ObserverDistanceToAnchorCm = ObserverDistance;
        return true;
    }

    Runtime.Diagnostic =
        TEXT("No configured observer currently satisfies distance and line-of-sight.");
    return false;
}

AActor* ATMOPObservationDirector::FindEntityActor(
    const FName EntityId) const
{
    if (EntityId.IsNone() || GetWorld() == nullptr)
    {
        return nullptr;
    }

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (const ATMOPVehicleBase* Vehicle =
            Cast<ATMOPVehicleBase>(Actor))
        {
            if (Vehicle->VehicleId == EntityId)
            {
                return Actor;
            }
        }

        const UTMOPWorldEntityComponent* Identity =
            IsValid(Actor)
                ? Actor->FindComponentByClass<UTMOPWorldEntityComponent>()
                : nullptr;
        if (Identity != nullptr && Identity->EntityId == EntityId)
        {
            return Actor;
        }
    }

    return nullptr;
}

bool ATMOPObservationDirector::TryGetObservationDefinition(
    const FName ObservationId,
    FTMOPObservationDefinition& OutDefinition) const
{
    if (const FTMOPObservationDefinition* Found =
        LoadedObservations.Find(ObservationId))
    {
        OutDefinition = *Found;
        return true;
    }
    OutDefinition = FTMOPObservationDefinition();
    return false;
}

bool ATMOPObservationDirector::TryGetObservationRuntime(
    const FName ObservationId,
    FTMOPObservationRuntime& OutRuntime) const
{
    if (const FTMOPObservationRuntime* Found =
        RuntimeObservations.Find(ObservationId))
    {
        OutRuntime = *Found;
        return true;
    }
    OutRuntime = FTMOPObservationRuntime();
    return false;
}

TArray<FTMOPObservationRuntime>
ATMOPObservationDirector::GetAllObservationRuntime() const
{
    TArray<FTMOPObservationRuntime> Results;
    RuntimeObservations.GenerateValueArray(Results);
    Results.Sort([](
        const FTMOPObservationRuntime& A,
        const FTMOPObservationRuntime& B)
    {
        return FNameLexicalLess()(A.ObservationId, B.ObservationId);
    });
    return Results;
}

bool ATMOPObservationDirector::ValidateObservationData(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();

    if (IsValid(ObservationTable) &&
        ObservationTable->GetRowStruct() !=
            FTMOPObservationDefinition::StaticStruct())
    {
        OutErrors.Add(TEXT(
            "ObservationTable has the wrong row structure."));
    }
    if (IsValid(ObservationLinkTable) &&
        ObservationLinkTable->GetRowStruct() !=
            FTMOPObservationLinkDefinition::StaticStruct())
    {
        OutErrors.Add(TEXT(
            "ObservationLinkTable has the wrong row structure."));
    }
    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        LoadedObservations)
    {
        const FTMOPObservationDefinition& Definition = Pair.Value;
        if (Definition.ObserverEntityIds.IsEmpty())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' has no observers."),
                *Pair.Key.ToString()));
        }
        if (Definition.ObservedEntityId.IsNone())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' has no observed entity."),
                *Pair.Key.ToString()));
        }
        if (Definition.ObservationAnchorId.IsNone())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' has no observation anchor."),
                *Pair.Key.ToString()));
        }
        if (Definition.TimingMode ==
                ETMOPObservationTimingMode::RelativeToSharedEvent &&
            Definition.ReferenceSharedEventId.IsNone())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' has no reference shared event."),
                *Pair.Key.ToString()));
        }
    }

    for (const TPair<FName, FTMOPObservationLinkDefinition>& Pair :
        LoadedLinks)
    {
        const FTMOPObservationLinkDefinition& Link = Pair.Value;
        if (!LoadedObservations.Contains(Link.FromObservationId) ||
            !LoadedObservations.Contains(Link.ToObservationId))
        {
            OutErrors.Add(FString::Printf(
                TEXT("Link '%s' references an unknown observation."),
                *Pair.Key.ToString()));
        }
        for (const FTMOPObservationRouteAlternative& Alternative :
            Link.AlternativeRoutes)
        {
            if (Alternative.RouteId.IsNone())
            {
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' has an alternative route without RouteId."),
                    *Pair.Key.ToString()));
            }
        }
    }

    return OutErrors.IsEmpty();
}
