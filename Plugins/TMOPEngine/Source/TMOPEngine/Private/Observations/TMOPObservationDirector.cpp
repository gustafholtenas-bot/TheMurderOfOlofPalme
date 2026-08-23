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
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
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
    for (TPair<FName, FResolvedTrack>& Pair : ResolvedTracks)
        if (Pair.Value.bCollisionSuppressedByTrack &&
            Pair.Value.ControlledActor.IsValid())
            Pair.Value.ControlledActor->SetActorEnableCollision(
                Pair.Value.bActorCollisionWasEnabled);
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

void ATMOPObservationDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bEnableLinkedTrackSimulation || GetGameInstance() == nullptr) return;
    const UTMOPClockSubsystem* Clock =
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    if (Clock != nullptr)
        UpdateObservationTracks(
            Clock->GetCurrentTime().ToSecondsFromMidnight());
}

int32 ATMOPObservationDirector::ReloadObservationData()
{
    LoadedObservations.Reset();
    LoadedLinks.Reset();
    RuntimeTracks.Reset();
    ResolvedTracks.Reset();

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
    bool bTrackInputsChanged = RuntimeTracks.IsEmpty();
    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        LoadedObservations)
    {
        const FTMOPObservationRuntime* Before =
            RuntimeObservations.Find(Pair.Key);
        const bool bWasResolved = Before != nullptr &&
            Before->bHasResolvedCanonicalTime;
        const int32 PreviousStart = bWasResolved
            ? Before->ResolvedCanonicalStartTime.ToSecondsFromMidnight()
            : INDEX_NONE;
        const int32 PreviousEnd = bWasResolved
            ? Before->ResolvedCanonicalEndTime.ToSecondsFromMidnight()
            : INDEX_NONE;
        if (ResolveCanonicalTime(Pair.Key))
        {
            ++ResolvedCount;
            const FTMOPObservationRuntime* After =
                RuntimeObservations.Find(Pair.Key);
            bTrackInputsChanged |= !bWasResolved || After == nullptr ||
                After->ResolvedCanonicalStartTime.ToSecondsFromMidnight() !=
                    PreviousStart ||
                After->ResolvedCanonicalEndTime.ToSecondsFromMidnight() !=
                    PreviousEnd;
        }
    }
    if (bTrackInputsChanged) RebuildObservationTracks();
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

TArray<FName> ATMOPObservationDirector::GetLinkObservationIds(
    const FTMOPObservationLinkDefinition& Link) const
{
    TArray<FName> Result;
    TSet<FName> Seen;
    for (const FName ObservationId : Link.ObservationIds)
        if (!ObservationId.IsNone() && !Seen.Contains(ObservationId))
        {
            Seen.Add(ObservationId);
            Result.Add(ObservationId);
        }

    // Seamless migration for every existing two-column link table.
    if (Result.IsEmpty())
        for (const FName ObservationId :
            {Link.FromObservationId, Link.ToObservationId})
            if (!ObservationId.IsNone() && !Seen.Contains(ObservationId))
            {
                Seen.Add(ObservationId);
                Result.Add(ObservationId);
            }
    return Result;
}

int32 ATMOPObservationDirector::RebuildObservationTracks()
{
    for (TPair<FName, FResolvedTrack>& Pair : ResolvedTracks)
        if (Pair.Value.bCollisionSuppressedByTrack &&
            Pair.Value.ControlledActor.IsValid())
            Pair.Value.ControlledActor->SetActorEnableCollision(
                Pair.Value.bActorCollisionWasEnabled);
    ResolvedTracks.Reset();
    RuntimeTracks.Reset();

    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    if (Anchors == nullptr) return 0;

    int32 BuiltCount = 0;
    for (const TPair<FName, FTMOPObservationLinkDefinition>& Pair : LoadedLinks)
    {
        const FTMOPObservationLinkDefinition& Link = Pair.Value;
        FTMOPObservationTrackRuntime Runtime;
        Runtime.LinkId = Pair.Key;
        Runtime.LinkedEntityId = Link.LinkedEntityId;
        Runtime.State = ETMOPObservationTrackRuntimeState::Unresolved;

        const TArray<FName> MemberIds = GetLinkObservationIds(Link);
        FResolvedTrack Track;
        bool bInvalid = MemberIds.Num() < 2;
        for (const FName ObservationId : MemberIds)
        {
            const FTMOPObservationDefinition* Definition =
                LoadedObservations.Find(ObservationId);
            const FTMOPObservationRuntime* ObservationRuntime =
                RuntimeObservations.Find(ObservationId);
            ATMOPHistoricalAnchor* Anchor = Definition != nullptr
                ? Anchors->FindAnchor(Definition->ObservationAnchorId) : nullptr;
            if (Definition == nullptr || ObservationRuntime == nullptr ||
                !ObservationRuntime->bHasResolvedCanonicalTime ||
                !IsValid(Anchor))
            {
                bInvalid = true;
                continue;
            }
            FResolvedTrackPoint Point;
            Point.ObservationId = ObservationId;
            Point.AnchorId = Definition->ObservationAnchorId;
            Point.StartSecond = ObservationRuntime->ResolvedCanonicalStartTime
                .ToSecondsFromMidnight();
            Point.EndSecond = ObservationRuntime->ResolvedCanonicalEndTime
                .ToSecondsFromMidnight();
            Point.Location = Anchor->GetAnchorLocation();
            Track.Points.Add(MoveTemp(Point));
        }
        Track.Points.Sort([](const FResolvedTrackPoint& A,
            const FResolvedTrackPoint& B)
        {
            if (A.StartSecond != B.StartSecond)
                return A.StartSecond < B.StartSecond;
            return A.ObservationId.LexicalLess(B.ObservationId);
        });
        for (const FResolvedTrackPoint& Point : Track.Points)
            Runtime.OrderedObservationIds.Add(Point.ObservationId);

        if (Runtime.LinkedEntityId.IsNone() && !Track.Points.IsEmpty())
        {
            FName SharedObservedEntityId = LoadedObservations.FindChecked(
                Track.Points[0].ObservationId).ObservedEntityId;
            for (const FResolvedTrackPoint& Point : Track.Points)
                if (LoadedObservations.FindChecked(Point.ObservationId)
                        .ObservedEntityId != SharedObservedEntityId)
                {
                    SharedObservedEntityId = NAME_None;
                    break;
                }
            Runtime.LinkedEntityId = SharedObservedEntityId;
        }

        for (int32 Index = 0; Index + 1 < Track.Points.Num(); ++Index)
        {
            const FResolvedTrackPoint& From = Track.Points[Index];
            const FResolvedTrackPoint& To = Track.Points[Index + 1];
            FResolvedTrackSegment Segment;
            Segment.FromObservationId = From.ObservationId;
            Segment.ToObservationId = To.ObservationId;
            Segment.TravelStartSecond = From.EndSecond;
            Segment.TravelEndSecond = To.StartSecond;
            Segment.PolylinePoints.Add(From.Location);

            const FTMOPObservationTrackSegment* Authored =
                Link.TrackSegments.FindByPredicate(
                    [&From, &To](const FTMOPObservationTrackSegment& Candidate)
                    {
                        return Candidate.FromObservationId == From.ObservationId &&
                            Candidate.ToObservationId == To.ObservationId;
                    });
            const TArray<FName>* RouteIds = Authored != nullptr
                ? &Authored->RouteAnchorIds : nullptr;
            if (RouteIds == nullptr && Index == 0 &&
                Link.FromObservationId == From.ObservationId &&
                Link.ToObservationId == To.ObservationId)
                RouteIds = &Link.RouteAnchorIds;
            if (RouteIds != nullptr)
                for (const FName AnchorId : *RouteIds)
                    if (const ATMOPHistoricalAnchor* RouteAnchor =
                        Anchors->FindAnchor(AnchorId))
                        Segment.PolylinePoints.Add(
                            RouteAnchor->GetAnchorLocation());
            Segment.PolylinePoints.Add(To.Location);

            Segment.CumulativeDistancesCm.Add(0.0f);
            for (int32 PointIndex = 1;
                PointIndex < Segment.PolylinePoints.Num(); ++PointIndex)
            {
                Segment.DistanceCm += FVector::Dist(
                    Segment.PolylinePoints[PointIndex - 1],
                    Segment.PolylinePoints[PointIndex]);
                Segment.CumulativeDistancesCm.Add(Segment.DistanceCm);
            }
            if (Authored != nullptr && Authored->AuthoredDistanceCm > 0.0f)
                Segment.DistanceCm = Authored->AuthoredDistanceCm;
            const int32 TravelSeconds =
                Segment.TravelEndSecond - Segment.TravelStartSecond;
            Segment.RequiredSpeedCmPerSecond = TravelSeconds > 0
                ? Segment.DistanceCm / static_cast<float>(TravelSeconds)
                : TNumericLimits<float>::Max();
            Runtime.MaximumRequiredSpeedCmPerSecond = FMath::Max(
                Runtime.MaximumRequiredSpeedCmPerSecond,
                Segment.RequiredSpeedCmPerSecond);
            Track.Segments.Add(MoveTemp(Segment));
        }

        const ETMOPObservedEntityType EntityType =
            Link.LinkedEntityType != ETMOPObservedEntityType::Unknown
            ? Link.LinkedEntityType
            : Track.Points.Num() > 0 &&
                LoadedObservations.Contains(Track.Points[0].ObservationId)
            ? LoadedObservations.FindChecked(
                Track.Points[0].ObservationId).ObservedEntityType
            : ETMOPObservedEntityType::Unknown;
        const float MaximumSpeed = Link.MaximumPlausibleSpeedCmPerSecond > 0.0f
            ? Link.MaximumPlausibleSpeedCmPerSecond
            : EntityType == ETMOPObservedEntityType::Vehicle
            ? 5000.0f : 800.0f;
        Runtime.bPhysicallyPlausible =
            Runtime.MaximumRequiredSpeedCmPerSecond <= MaximumSpeed;

        if (Link.Relationship == ETMOPObservationRelationship::Rejected)
        {
            Runtime.State = ETMOPObservationTrackRuntimeState::Invalid;
            Runtime.Diagnostic = TEXT("Rejected links are never interpolated.");
        }
        else if (bInvalid || Track.Points.Num() < 2)
        {
            Runtime.State = ETMOPObservationTrackRuntimeState::Invalid;
            Runtime.Diagnostic =
                TEXT("Track needs at least two valid, timed observations with anchors.");
        }
        else
        {
            Runtime.State =
                ETMOPObservationTrackRuntimeState::WaitingForFirstObservation;
            Runtime.InferredLocation = Track.Points[0].Location;
            Runtime.Diagnostic = Runtime.bPhysicallyPlausible
                ? FString::Printf(TEXT("Built %d observations and %d route segments."),
                    Track.Points.Num(), Track.Segments.Num())
                : FString::Printf(
                    TEXT("Track requires %.0f cm/s, above the configured plausible maximum %.0f cm/s."),
                    Runtime.MaximumRequiredSpeedCmPerSecond, MaximumSpeed);
            ++BuiltCount;
        }

        RuntimeTracks.Add(Pair.Key, Runtime);
        ResolvedTracks.Add(Pair.Key, MoveTemp(Track));
    }
    return BuiltCount;
}

void ATMOPObservationDirector::UpdateObservationTracks(
    const int32 CurrentSecond)
{
    for (const TPair<FName, FTMOPObservationLinkDefinition>& Pair : LoadedLinks)
    {
        const FTMOPObservationLinkDefinition& Link = Pair.Value;
        FTMOPObservationTrackRuntime* Runtime = RuntimeTracks.Find(Pair.Key);
        FResolvedTrack* Track = ResolvedTracks.Find(Pair.Key);
        if (Runtime == nullptr || Track == nullptr ||
            Runtime->State == ETMOPObservationTrackRuntimeState::Invalid ||
            Track->Points.Num() < 2)
            continue;

        FVector TargetLocation = Track->Points[0].Location;
        FVector TargetDirection = FVector::ForwardVector;
        bool bHasWorldPosition = false;
        Runtime->CurrentSegmentIndex = INDEX_NONE;
        Runtime->CurrentFromObservationId = NAME_None;
        Runtime->CurrentToObservationId = NAME_None;
        Runtime->SegmentAlpha = 0.0f;
        Runtime->CurrentRequiredSpeedCmPerSecond = 0.0f;

        if (CurrentSecond < Track->Points[0].StartSecond)
        {
            Runtime->State =
                ETMOPObservationTrackRuntimeState::WaitingForFirstObservation;
            Runtime->InferredLocation = Track->Points[0].Location;
            continue;
        }

        for (const FResolvedTrackPoint& Point : Track->Points)
            if (CurrentSecond >= Point.StartSecond &&
                CurrentSecond <= Point.EndSecond)
            {
                Runtime->State =
                    ETMOPObservationTrackRuntimeState::AtObservation;
                Runtime->CurrentFromObservationId = Point.ObservationId;
                Runtime->CurrentToObservationId = Point.ObservationId;
                TargetLocation = Point.Location;
                bHasWorldPosition = true;
                break;
            }

        if (!bHasWorldPosition)
            for (int32 Index = 0; Index < Track->Segments.Num(); ++Index)
            {
                const FResolvedTrackSegment& Segment = Track->Segments[Index];
                if (CurrentSecond < Segment.TravelStartSecond ||
                    CurrentSecond > Segment.TravelEndSecond)
                    continue;
                const int32 Duration = FMath::Max(
                    1, Segment.TravelEndSecond - Segment.TravelStartSecond);
                const float Alpha = FMath::Clamp(
                    static_cast<float>(CurrentSecond -
                        Segment.TravelStartSecond) /
                        static_cast<float>(Duration),
                    0.0f, 1.0f);
                const float GeometricDistance =
                    Segment.CumulativeDistancesCm.IsEmpty()
                    ? 0.0f : Segment.CumulativeDistancesCm.Last();
                const float DesiredDistance = GeometricDistance * Alpha;
                int32 LegIndex = 1;
                while (LegIndex < Segment.CumulativeDistancesCm.Num() &&
                    Segment.CumulativeDistancesCm[LegIndex] < DesiredDistance)
                    ++LegIndex;
                LegIndex = FMath::Clamp(
                    LegIndex, 1, Segment.PolylinePoints.Num() - 1);
                const float LegStart =
                    Segment.CumulativeDistancesCm[LegIndex - 1];
                const float LegLength = FMath::Max(1.0f,
                    Segment.CumulativeDistancesCm[LegIndex] - LegStart);
                const float LegAlpha = FMath::Clamp(
                    (DesiredDistance - LegStart) / LegLength, 0.0f, 1.0f);
                TargetLocation = FMath::Lerp(
                    Segment.PolylinePoints[LegIndex - 1],
                    Segment.PolylinePoints[LegIndex], LegAlpha);
                TargetDirection =
                    Segment.PolylinePoints[LegIndex] -
                    Segment.PolylinePoints[LegIndex - 1];
                Runtime->State =
                    ETMOPObservationTrackRuntimeState::Interpolating;
                Runtime->CurrentSegmentIndex = Index;
                Runtime->CurrentFromObservationId =
                    Segment.FromObservationId;
                Runtime->CurrentToObservationId = Segment.ToObservationId;
                Runtime->SegmentAlpha = Alpha;
                Runtime->CurrentRequiredSpeedCmPerSecond =
                    Segment.RequiredSpeedCmPerSecond;
                bHasWorldPosition = true;
                break;
            }

        if (!bHasWorldPosition &&
            CurrentSecond > Track->Points.Last().EndSecond)
        {
            Runtime->State = ETMOPObservationTrackRuntimeState::Completed;
            Runtime->InferredLocation = Track->Points.Last().Location;
            if (Track->bCollisionSuppressedByTrack &&
                Track->ControlledActor.IsValid())
            {
                Track->ControlledActor->SetActorEnableCollision(
                    Track->bActorCollisionWasEnabled);
                Track->bCollisionSuppressedByTrack = false;
            }
            continue;
        }

        Runtime->InferredLocation = TargetLocation;
        if (!bHasWorldPosition ||
            Link.SimulationMode !=
                ETMOPObservationTrackSimulationMode::InterpolateExistingActor)
            continue;

        const FString EntityText = Runtime->LinkedEntityId.ToString();
        const bool bEvidenceEntity = EntityText.StartsWith(
            TEXT("OBSERVED"), ESearchCase::IgnoreCase);
        if (!bEvidenceEntity && !Link.bAllowMovementOfKnownEntity)
        {
            Runtime->Diagnostic =
                TEXT("Known entity movement is protected; enable Allow Movement Of Known Entity only for a deliberate override.");
            continue;
        }
        AActor* Actor = Track->ControlledActor.Get();
        if (!IsValid(Actor))
        {
            Actor = FindEntityActor(Runtime->LinkedEntityId);
            Track->ControlledActor = Actor;
        }
        if (!IsValid(Actor))
        {
            Runtime->Diagnostic = FString::Printf(
                TEXT("Linked entity '%s' has no active actor. Create/spawn its People or Vehicle row first."),
                *Runtime->LinkedEntityId.ToString());
            continue;
        }
        if (Link.bDisableCollisionWhileInterpolating &&
            !Track->bCollisionSuppressedByTrack)
        {
            Track->bActorCollisionWasEnabled = Actor->GetActorEnableCollision();
            Actor->SetActorEnableCollision(false);
            Track->bCollisionSuppressedByTrack = true;
        }
        FRotator TargetRotation = Actor->GetActorRotation();
        TargetDirection.Z = 0.0f;
        if (!TargetDirection.IsNearlyZero())
            TargetRotation.Yaw = TargetDirection.Rotation().Yaw;
        Actor->SetActorLocationAndRotation(
            TargetLocation, TargetRotation, false, nullptr,
            ETeleportType::TeleportPhysics);
    }
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

    // Some legacy Blender observations preserve a documented/reconstructed
    // sighting window and signalement but not the witness identity. They may
    // drive a short observed-entity visualization without pretending that a
    // known witness was present. Sourced rows continue through the stricter
    // observer-distance and line-of-sight checks below.
    if (Definition.ObserverEntityIds.IsEmpty() &&
        Definition.bAllowUnattributedObservation)
    {
        Runtime.Diagnostic =
            TEXT("Unattributed reconstructed observation satisfied by the observed entity position.");
        return true;
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

bool ATMOPObservationDirector::TryGetObservationTrackRuntime(
    const FName LinkId,
    FTMOPObservationTrackRuntime& OutRuntime) const
{
    if (const FTMOPObservationTrackRuntime* Found =
        RuntimeTracks.Find(LinkId))
    {
        OutRuntime = *Found;
        return true;
    }
    OutRuntime = FTMOPObservationTrackRuntime();
    return false;
}

TArray<FTMOPObservationTrackRuntime>
ATMOPObservationDirector::GetAllObservationTrackRuntime() const
{
    TArray<FTMOPObservationTrackRuntime> Results;
    RuntimeTracks.GenerateValueArray(Results);
    Results.Sort([](const FTMOPObservationTrackRuntime& A,
        const FTMOPObservationTrackRuntime& B)
    {
        return A.LinkId.LexicalLess(B.LinkId);
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
        const TArray<FName> MemberIds = GetLinkObservationIds(Link);
        if (MemberIds.Num() < 2)
        {
            OutErrors.Add(FString::Printf(
                TEXT("Link '%s' needs at least two unique observations."),
                *Pair.Key.ToString()));
        }
        if (!Link.ObservationIds.IsEmpty() &&
            MemberIds.Num() != Link.ObservationIds.Num())
            OutErrors.Add(FString::Printf(
                TEXT("Link '%s' contains empty or duplicate ObservationIds."),
                *Pair.Key.ToString()));

        ETMOPObservedEntityType FirstType = ETMOPObservedEntityType::Unknown;
        for (const FName ObservationId : MemberIds)
        {
            const FTMOPObservationDefinition* Observation =
                LoadedObservations.Find(ObservationId);
            if (Observation == nullptr)
            {
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' references unknown observation '%s'."),
                    *Pair.Key.ToString(), *ObservationId.ToString()));
                continue;
            }
            if (FirstType == ETMOPObservedEntityType::Unknown)
                FirstType = Observation->ObservedEntityType;
            else if (Observation->ObservedEntityType !=
                    ETMOPObservedEntityType::Unknown &&
                FirstType != Observation->ObservedEntityType)
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' mixes person and vehicle observations."),
                    *Pair.Key.ToString()));
        }
        if (Link.LinkedEntityType != ETMOPObservedEntityType::Unknown &&
            FirstType != ETMOPObservedEntityType::Unknown &&
            Link.LinkedEntityType != FirstType)
            OutErrors.Add(FString::Printf(
                TEXT("Link '%s' LinkedEntityType contradicts its observations."),
                *Pair.Key.ToString()));

        for (const FTMOPObservationTrackSegment& Segment : Link.TrackSegments)
        {
            if (!MemberIds.Contains(Segment.FromObservationId) ||
                !MemberIds.Contains(Segment.ToObservationId) ||
                Segment.FromObservationId == Segment.ToObservationId)
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' has a track segment outside its ObservationIds array."),
                    *Pair.Key.ToString()));
            for (const FTMOPObservationRouteAlternative& Alternative :
                Segment.AlternativeRoutes)
                if (Alternative.RouteId.IsNone())
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' has a segment alternative without RouteId."),
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
