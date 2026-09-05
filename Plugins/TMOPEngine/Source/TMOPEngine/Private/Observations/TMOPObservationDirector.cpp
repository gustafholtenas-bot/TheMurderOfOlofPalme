#include "Observations/TMOPObservationDirector.h"
#include "Observations/TMOPObservationSignalementLibrary.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Components/ActorComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Time/TMOPClockSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"

namespace
{
const TCHAR* WorldGuideSettingsSection = TEXT("TMOP.WorldGuideSettings");
const TCHAR* ShowOlofLineKey = TEXT("ShowOlofLocationLine");
const TCHAR* ShowObservationLinesKey = TEXT("ShowActiveObservationLines");

bool ReadWorldGuideSetting(const TCHAR* Key, const bool DefaultValue)
{
    bool Value = DefaultValue;
    if (GConfig != nullptr)
        GConfig->GetBool(WorldGuideSettingsSection, Key, Value,
            GGameUserSettingsIni);
    return Value;
}

void WriteWorldGuideSetting(const TCHAR* Key, const bool Value)
{
    if (GConfig == nullptr) return;
    GConfig->SetBool(WorldGuideSettingsSection, Key, Value,
        GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}
}

ATMOPObservationDirector::ATMOPObservationDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void ATMOPObservationDirector::BeginPlay()
{
    Super::BeginPlay();

    bShowOlofLocationLine = GetSavedShowOlofLocationLine();
    bShowActiveObservationLines = GetSavedShowActiveObservationLines();

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
    const UGameInstance* GameInstance = GetGameInstance();
    const UTMOPClockSubsystem* Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const double CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTimeSecondsExact() : -1.0;
    if (bEnableLinkedTrackSimulation && CurrentSecond >= 0.0)
        UpdateObservationTracks(CurrentSecond);
    DrawWorldGuideLines(CurrentSecond >= 0.0
        ? FMath::FloorToInt(CurrentSecond) : INDEX_NONE);
}

bool ATMOPObservationDirector::GetSavedShowOlofLocationLine()
{
    return ReadWorldGuideSetting(ShowOlofLineKey, true);
}

bool ATMOPObservationDirector::GetSavedShowActiveObservationLines()
{
    return ReadWorldGuideSetting(ShowObservationLinesKey, true);
}

void ATMOPObservationDirector::SaveShowOlofLocationLine(const bool bShow)
{
    WriteWorldGuideSetting(ShowOlofLineKey, bShow);
}

void ATMOPObservationDirector::SaveShowActiveObservationLines(const bool bShow)
{
    WriteWorldGuideSetting(ShowObservationLinesKey, bShow);
}

void ATMOPObservationDirector::SetShowOlofLocationLine(const bool bShow)
{
    bShowOlofLocationLine = bShow;
    SaveShowOlofLocationLine(bShow);
}

void ATMOPObservationDirector::SetShowActiveObservationLines(const bool bShow)
{
    bShowActiveObservationLines = bShow;
    SaveShowActiveObservationLines(bShow);
}

void ATMOPObservationDirector::DrawWorldGuideLines(
    const int32 CurrentSecond) const
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;

    if (bShowOlofLocationLine)
    {
        if (AActor* Olof = FindEntityActor(OlofPalmeEntityId);
            IsValid(Olof))
        {
            const FVector Start = Olof->GetActorLocation();
            DrawDebugLine(World, Start,
                Start + FVector::UpVector * OlofLocationLineHeightCm,
                OlofLocationLineColor.ToFColor(true), false, 0.0f, 0,
                WorldGuideLineThickness);
        }
    }

    if (!bShowActiveObservationLines || CurrentSecond == INDEX_NONE) return;
    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        LoadedObservations)
    {
        const FTMOPObservationDefinition& Definition = Pair.Value;
        const FTMOPObservationRuntime* Runtime =
            RuntimeObservations.Find(Pair.Key);
        if (!Definition.bEnabled || Runtime == nullptr ||
            !Runtime->bHasResolvedCanonicalTime ||
            Definition.ObservedEntityId.IsNone())
            continue;

        const int32 StartSecond =
            Runtime->ResolvedCanonicalStartTime.ToSecondsFromMidnight();
        const int32 EndSecond =
            Runtime->ResolvedCanonicalEndTime.ToSecondsFromMidnight();
        if (CurrentSecond < StartSecond || CurrentSecond > EndSecond)
            continue;

        AActor* Observed = FindObservedActorForObservation(Definition);
        if (!IsValid(Observed)) continue;
        const FVector ObservedPoint =
            Observed->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
        for (const FName ObserverId : Definition.ObserverEntityIds)
        {
            AActor* Observer = FindEntityActor(ObserverId);
            if (!IsValid(Observer)) continue;
            const FVector ObserverPoint =
                Observer->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
            const float ArrowSize = FMath::Min(ObservationArrowSizeCm,
                FVector::Distance(ObserverPoint, ObservedPoint) * 0.25f);
            DrawDebugDirectionalArrow(World, ObserverPoint, ObservedPoint,
                FMath::Max(5.0f, ArrowSize),
                ObservationLineColor.ToFColor(true), false, 0.0f, 0,
                ObservationLineThickness);
        }
    }
}

int32 ATMOPObservationDirector::ReloadObservationData()
{
    LoadedObservations.Reset();
    LoadedLinks.Reset();
    RuntimeTracks.Reset();
    ResolvedTracks.Reset();
    ObservationPlaybackEntities.Reset();
    EntityActorCache.Reset();

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
    ObservationPlaybackEntities.Reset();

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
        bool bHasOverlappingWindows = false;
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
            Segment.TravelEndSecond = To.StartSecond;
            Segment.PolylinePoints.Add(From.Location);

            const bool bSamePlace = From.AnchorId == To.AnchorId ||
                From.Location.Equals(To.Location, 1.0f);
            if (To.StartSecond < From.EndSecond && !bSamePlace)
                bHasOverlappingWindows = true;

            // Multiple witnesses can report the same subject at the same
            // place during overlapping windows. That is one stationary
            // interval, not an impossible movement leg.
            if (To.StartSecond <= From.EndSecond && bSamePlace)
                continue;

            const FTMOPObservationTrackSegment* Authored =
                Link.TrackSegments.FindByPredicate(
                    [&From, &To](const FTMOPObservationTrackSegment& Candidate)
                    {
                        return Candidate.FromObservationId == From.ObservationId &&
                            Candidate.ToObservationId == To.ObservationId;
                    });
            if (Authored != nullptr)
            {
                Segment.MovementMode = Authored->MovementMode;
                Segment.VehicleEntityId = Authored->VehicleEntityId;
                Segment.VehicleSeatId = Authored->VehicleSeatId;
                Segment.MaximumBoardingDistanceCm =
                    Authored->MaximumBoardingDistanceCm;
                Segment.DriverEntityId = Authored->DriverEntityId;
                Segment.OrderedLaneIds = Authored->OrderedLaneIds;
                Segment.VehiclePassAnchorIds =
                    Authored->VehiclePassAnchorIds;
                Segment.VehicleRouteMode = Authored->VehicleRouteMode;
                Segment.VehicleDestinationAnchorId =
                    Authored->VehicleDestinationAnchorId;
                Segment.VehicleStartDistanceAlongFirstLaneCm =
                    Authored->VehicleStartDistanceAlongFirstLaneCm;
                Segment.VehicleCruiseSpeedKmh =
                    Authored->VehicleCruiseSpeedKmh;
                Segment.bIgnoreOneWayRestrictions =
                    Authored->bIgnoreOneWayRestrictions;
                Segment.bRunRedLights = Authored->bRunRedLights;
                if ((Authored->MovementMode ==
                        ETMOPObservationSegmentMovementMode::WalkToVehicleAndBoard ||
                     Authored->MovementMode ==
                        ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard) &&
                    Authored->BoardingLeadSeconds > 0.0f)
                    Segment.TravelEndSecond = FMath::Max(
                        static_cast<double>(From.EndSecond),
                        Segment.TravelEndSecond -
                            static_cast<double>(Authored->BoardingLeadSeconds));
            }
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

            const ETMOPObservedEntityType SegmentEntityType =
                Link.LinkedEntityType != ETMOPObservedEntityType::Unknown
                ? Link.LinkedEntityType
                : LoadedObservations.FindChecked(From.ObservationId)
                    .ObservedEntityType;
            if (Segment.MovementMode ==
                ETMOPObservationSegmentMovementMode::Automatic)
            {
                if (SegmentEntityType == ETMOPObservedEntityType::Vehicle)
                    Segment.MovementMode = Authored != nullptr &&
                            (!Authored->OrderedLaneIds.IsEmpty() ||
                             !Authored->VehicleDestinationAnchorId.IsNone())
                        ? ETMOPObservationSegmentMovementMode::VehicleLaneRoute
                        : ETMOPObservationSegmentMovementMode::VehicleDirectInterpolation;
                else
                    Segment.MovementMode =
                        ETMOPObservationSegmentMovementMode::Walk;
            }
            const bool bPedestrianPath =
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::Walk ||
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::Run ||
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::Sprint ||
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::WalkToVehicleAndBoard ||
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard ||
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::ExitVehicleThenWalk ||
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::ExitVehicleThenRun;
            if ((RouteIds == nullptr || RouteIds->IsEmpty()) &&
                bPedestrianPath &&
                Link.bUseNavigationPathForPeople)
            {
                if (UNavigationPath* NavigationPath =
                    UNavigationSystemV1::FindPathToLocationSynchronously(
                        GetWorld(), From.Location, To.Location))
                {
                    if (NavigationPath->IsValid() &&
                        NavigationPath->PathPoints.Num() >= 2)
                        Segment.PolylinePoints = NavigationPath->PathPoints;
                }
            }

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
            const double AvailableTravelSeconds =
                Segment.TravelEndSecond - static_cast<double>(From.EndSecond);
            const float MinimumRequiredSpeed = AvailableTravelSeconds > 0.0
                ? Segment.DistanceCm /
                    static_cast<float>(AvailableTravelSeconds)
                : TNumericLimits<float>::Max();
            const float ModeDefaultSpeed =
                Segment.MovementMode == ETMOPObservationSegmentMovementMode::Sprint
                    ? 600.0f
                : Segment.MovementMode == ETMOPObservationSegmentMovementMode::Run ||
                  Segment.MovementMode == ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard ||
                  Segment.MovementMode == ETMOPObservationSegmentMovementMode::ExitVehicleThenRun
                    ? 350.0f
                : Segment.MovementMode == ETMOPObservationSegmentMovementMode::VehicleLaneRoute ||
                  Segment.MovementMode == ETMOPObservationSegmentMovementMode::VehicleDirectInterpolation ||
                  Segment.MovementMode == ETMOPObservationSegmentMovementMode::RideInVehicle
                    ? 1200.0f : 140.0f;
            const float PreferredSpeed = Authored != nullptr &&
                    Authored->PreferredTravelSpeedCmPerSecond > 0.0f
                ? Authored->PreferredTravelSpeedCmPerSecond
                : Link.PreferredTravelSpeedCmPerSecond > 0.0f
                ? Link.PreferredTravelSpeedCmPerSecond : ModeDefaultSpeed;
            Segment.TravelStartSecond = From.EndSecond;
            if (Link.TravelTimingMode ==
                    ETMOPObservationTravelTimingMode::ArriveAtPreferredSpeed &&
                AvailableTravelSeconds > 0.0 && PreferredSpeed > 0.0f)
            {
                const double PreferredDuration =
                    Segment.DistanceCm / PreferredSpeed;
                Segment.TravelStartSecond = FMath::Max(
                    static_cast<double>(From.EndSecond),
                    Segment.TravelEndSecond - PreferredDuration);
            }
            const double PlannedTravelSeconds =
                Segment.TravelEndSecond - Segment.TravelStartSecond;
            Segment.RequiredSpeedCmPerSecond = PlannedTravelSeconds > 0.0
                ? Segment.DistanceCm /
                    static_cast<float>(PlannedTravelSeconds)
                : TNumericLimits<float>::Max();
            Runtime.MaximumRequiredSpeedCmPerSecond = FMath::Max(
                Runtime.MaximumRequiredSpeedCmPerSecond,
                MinimumRequiredSpeed);
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
        else if (bInvalid || Track.Points.Num() < 2 || bHasOverlappingWindows)
        {
            Runtime.State = ETMOPObservationTrackRuntimeState::Invalid;
            Runtime.Diagnostic = bHasOverlappingWindows
                ? TEXT("Consecutive observation windows overlap; split the hypothesis or remove duplicate sightings before interpolation.")
                : TEXT("Track needs at least two valid, timed observations with anchors.");
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

        if (Link.SimulationMode ==
                ETMOPObservationTrackSimulationMode::InterpolateExistingActor &&
            !Runtime.LinkedEntityId.IsNone())
            for (const FResolvedTrackPoint& Point : Track.Points)
                ObservationPlaybackEntities.FindOrAdd(
                    Point.ObservationId, Runtime.LinkedEntityId);

        RuntimeTracks.Add(Pair.Key, Runtime);
        ResolvedTracks.Add(Pair.Key, MoveTemp(Track));
    }
    return BuiltCount;
}

bool ATMOPObservationDirector::ApplySegmentTransition(
    const FTMOPObservationLinkDefinition& /*Link*/,
    FResolvedTrackSegment& Segment,
    FTMOPObservationTrackRuntime& Runtime,
    const double CurrentSecond)
{
    AActor* PlaybackActor = FindEntityActor(Runtime.LinkedEntityId);
    ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(PlaybackActor);
    const FName VehicleId = !Segment.VehicleEntityId.IsNone()
        ? Segment.VehicleEntityId
        : Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::VehicleLaneRoute
        ? Runtime.LinkedEntityId : NAME_None;
    ATMOPVehicleBase* Vehicle = Cast<ATMOPVehicleBase>(
        FindEntityActor(VehicleId));

    auto IsAgentSeatedInVehicle = [](ATMOPHistoricalAgent* Candidate,
        ATMOPVehicleBase* CandidateVehicle)
    {
        if (!IsValid(Candidate) || !IsValid(CandidateVehicle)) return false;
        for (const UTMOPVehicleSeatComponent* Seat :
            CandidateVehicle->GetVehicleSeats())
            if (IsValid(Seat) && Seat->GetOccupant() == Candidate) return true;
        return false;
    };

    const bool bExitAtStart =
        Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::ExitVehicleThenWalk ||
        Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::ExitVehicleThenRun;
    const bool bSeatAtStart =
        Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::RideInVehicle;
    const bool bSeatAtEnd =
        Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::WalkToVehicleAndBoard ||
        Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard;

    if (CurrentSecond >= Segment.TravelStartSecond &&
        !Segment.bStartTransitionApplied)
    {
        if (bExitAtStart)
        {
            if (!IsValid(Agent) || !IsValid(Vehicle))
            {
                Runtime.Diagnostic = TEXT(
                    "Exit-vehicle segment is waiting for its person and vehicle actors.");
                return false;
            }
            Segment.bStartTransitionApplied =
                !IsAgentSeatedInVehicle(Agent, Vehicle) ||
                Vehicle->ExitVehicle(Agent);
        }
        else if (bSeatAtStart)
        {
            if (!IsValid(Agent) || !IsValid(Vehicle))
            {
                Runtime.Diagnostic = TEXT(
                    "Ride segment is waiting for its person and vehicle actors.");
                return false;
            }
            if (!IsAgentSeatedInVehicle(Agent, Vehicle))
            {
                const float BoardingDistanceCm = FVector::Dist(
                    Agent->GetActorLocation(), Vehicle->GetActorLocation());
                if (Segment.MaximumBoardingDistanceCm > 0.0f &&
                    BoardingDistanceCm > Segment.MaximumBoardingDistanceCm)
                {
                    Runtime.Diagnostic = FString::Printf(
                        TEXT("Ride refused: '%s' is %.0f cm from '%s' (maximum %.0f cm)."),
                        *Runtime.LinkedEntityId.ToString(), BoardingDistanceCm,
                        *VehicleId.ToString(),
                        Segment.MaximumBoardingDistanceCm);
                    return false;
                }
            }
            Segment.bStartTransitionApplied =
                IsAgentSeatedInVehicle(Agent, Vehicle) ||
                Vehicle->EnterVehicle(Agent, Segment.VehicleSeatId);
        }
        else
        {
            Segment.bStartTransitionApplied = true;
        }
    }

    if (Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::VehicleLaneRoute &&
        CurrentSecond >= Segment.TravelStartSecond &&
        CurrentSecond <= Segment.TravelEndSecond &&
        !Segment.bVehicleRouteStarted)
    {
        if (!IsValid(Vehicle))
        {
            Runtime.Diagnostic = TEXT(
                "Vehicle lane segment is waiting for the linked vehicle actor.");
            return false;
        }
        if (!Segment.DriverEntityId.IsNone() &&
            !IsValid(Vehicle->GetDriverAgent()))
            if (ATMOPHistoricalAgent* Driver = Cast<ATMOPHistoricalAgent>(
                FindEntityActor(Segment.DriverEntityId)))
                Vehicle->EnterDriverSeat(Driver);

        ATMOPHistoricalVehicleDirector* VehicleDirector = nullptr;
        if (GetWorld() != nullptr)
            for (TActorIterator<ATMOPHistoricalVehicleDirector> It(GetWorld());
                It; ++It)
            {
                VehicleDirector = *It;
                break;
            }
        if (!IsValid(VehicleDirector) ||
            !VehicleDirector->BeginDrivingVehicle(
                VehicleId, Segment.DriverEntityId,
                Segment.OrderedLaneIds, Segment.VehiclePassAnchorIds,
                Segment.VehicleRouteMode,
                Segment.VehicleDestinationAnchorId,
                Segment.VehicleStartDistanceAlongFirstLaneCm,
                false))
        {
            Runtime.Diagnostic = TEXT(
                "Vehicle lane route could not start; check driver seat, lanes and destination anchor.");
            return false;
        }
        if (UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<
                UTMOPTrafficVehicleMovementComponent>())
        {
            Movement->bDespawnAtRouteEnd = false;
            Movement->bIgnoreOneWayRestrictions =
                Segment.bIgnoreOneWayRestrictions;
            Movement->bRunRedLights = Segment.bRunRedLights;
            if (Segment.VehicleCruiseSpeedKmh > 0.0f)
                Movement->DesiredCruiseSpeedKmh =
                    Segment.VehicleCruiseSpeedKmh;
            Movement->ConfigureTimedArrival(
                FMath::RoundToInt(Segment.TravelEndSecond));
        }
        Segment.bVehicleRouteStarted = true;
    }

    if (bSeatAtEnd && CurrentSecond >= Segment.TravelEndSecond &&
        !Segment.bEndTransitionApplied)
    {
        if (!IsValid(Agent) || !IsValid(Vehicle))
        {
            Runtime.Diagnostic = TEXT(
                "Boarding segment reached its deadline but person or vehicle is unavailable.");
            return false;
        }
        const float BoardingDistanceCm = FVector::Dist(
            Agent->GetActorLocation(), Vehicle->GetActorLocation());
        if (Segment.MaximumBoardingDistanceCm > 0.0f &&
            BoardingDistanceCm > Segment.MaximumBoardingDistanceCm)
        {
            Runtime.Diagnostic = FString::Printf(
                TEXT("Boarding refused: '%s' is %.0f cm from '%s' (maximum %.0f cm)."),
                *Runtime.LinkedEntityId.ToString(), BoardingDistanceCm,
                *VehicleId.ToString(), Segment.MaximumBoardingDistanceCm);
            return false;
        }
        Segment.bEndTransitionApplied =
            IsAgentSeatedInVehicle(Agent, Vehicle) ||
            Vehicle->EnterVehicle(Agent, Segment.VehicleSeatId);
        if (!Segment.bEndTransitionApplied)
        {
            Runtime.Diagnostic = FString::Printf(
                TEXT("Could not board '%s' into seat '%s' of '%s'."),
                *Runtime.LinkedEntityId.ToString(),
                *Segment.VehicleSeatId.ToString(), *VehicleId.ToString());
            return false;
        }
    }

    Runtime.bPlaybackActorSeated =
        IsAgentSeatedInVehicle(Agent, Vehicle);
    return true;
}

void ATMOPObservationDirector::UpdateObservationTracks(
    const double CurrentSecond)
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
        Runtime->CurrentMovementMode =
            ETMOPObservationSegmentMovementMode::Automatic;
        Runtime->CurrentVehicleEntityId = NAME_None;
        Runtime->bPlaybackActorSeated = false;

        if (CurrentSecond < Track->Points[0].StartSecond)
        {
            Runtime->State =
                ETMOPObservationTrackRuntimeState::WaitingForFirstObservation;
            Runtime->InferredLocation = Track->Points[0].Location;
            continue;
        }

        if (Link.SimulationMode ==
            ETMOPObservationTrackSimulationMode::InterpolateExistingActor)
            for (FResolvedTrackSegment& Segment : Track->Segments)
                if (CurrentSecond >= Segment.TravelStartSecond)
                    ApplySegmentTransition(
                        Link, Segment, *Runtime, CurrentSecond);

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
                FResolvedTrackSegment& Segment = Track->Segments[Index];
                if (CurrentSecond < Segment.TravelStartSecond ||
                    CurrentSecond > Segment.TravelEndSecond)
                    continue;
                const double Duration = FMath::Max(
                    0.001, Segment.TravelEndSecond - Segment.TravelStartSecond);
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
                Runtime->CurrentMovementMode = Segment.MovementMode;
                Runtime->CurrentVehicleEntityId =
                    !Segment.VehicleEntityId.IsNone()
                    ? Segment.VehicleEntityId
                    : Segment.MovementMode ==
                            ETMOPObservationSegmentMovementMode::VehicleLaneRoute ||
                      Segment.MovementMode ==
                            ETMOPObservationSegmentMovementMode::VehicleDirectInterpolation
                    ? Runtime->LinkedEntityId : NAME_None;
                if (Segment.MovementMode ==
                        ETMOPObservationSegmentMovementMode::RideInVehicle ||
                    Segment.MovementMode ==
                        ETMOPObservationSegmentMovementMode::VehicleLaneRoute)
                    if (AActor* Controlled = FindEntityActor(
                        Runtime->LinkedEntityId))
                        TargetLocation = Controlled->GetActorLocation();
                if (Segment.MovementMode ==
                    ETMOPObservationSegmentMovementMode::HoldPosition)
                    TargetLocation = Segment.PolylinePoints[0];
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
                if (!IsValid(
                    Track->ControlledActor->GetAttachParentActor()))
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
        const bool bAttachedToVehicle =
            IsValid(Cast<ATMOPVehicleBase>(Actor->GetAttachParentActor()));
        const bool bMovementOwnedByVehicle =
            Runtime->CurrentMovementMode ==
                ETMOPObservationSegmentMovementMode::VehicleLaneRoute ||
            Runtime->CurrentMovementMode ==
                ETMOPObservationSegmentMovementMode::RideInVehicle ||
            bAttachedToVehicle;
        Runtime->bPlaybackActorSeated = bAttachedToVehicle;
        if (bMovementOwnedByVehicle)
        {
            Runtime->InferredLocation = Actor->GetActorLocation();
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
        if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(Actor))
        {
            ETMOPAgentActivityState Activity =
                ETMOPAgentActivityState::Standing;
            if (Runtime->State ==
                ETMOPObservationTrackRuntimeState::Interpolating)
                Activity = Runtime->CurrentMovementMode ==
                        ETMOPObservationSegmentMovementMode::Sprint
                    ? ETMOPAgentActivityState::Sprinting
                    : Runtime->CurrentMovementMode ==
                            ETMOPObservationSegmentMovementMode::Run ||
                      Runtime->CurrentMovementMode ==
                            ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard ||
                      Runtime->CurrentMovementMode ==
                            ETMOPObservationSegmentMovementMode::ExitVehicleThenRun
                    ? ETMOPAgentActivityState::Running
                    : ETMOPAgentActivityState::Walking;
            Agent->SetActivityState(Activity);
        }
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
    AActor* ObservedActor = FindObservedActorForObservation(Definition);

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

AActor* ATMOPObservationDirector::FindObservedActorForObservation(
    const FTMOPObservationDefinition& Definition) const
{
    if (const FName* PlaybackEntity =
        ObservationPlaybackEntities.Find(Definition.ObservationId))
        if (AActor* Actor = FindEntityActor(*PlaybackEntity); IsValid(Actor))
            return Actor;
    return FindEntityActor(Definition.ObservedEntityId);
}

AActor* ATMOPObservationDirector::FindEntityActor(
    const FName EntityId) const
{
    if (EntityId.IsNone() || GetWorld() == nullptr)
    {
        return nullptr;
    }

    if (const TWeakObjectPtr<AActor>* Cached =
        EntityActorCache.Find(EntityId))
    {
        if (Cached->IsValid()) return Cached->Get();
        EntityActorCache.Remove(EntityId);
    }

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (const ATMOPVehicleBase* Vehicle =
            Cast<ATMOPVehicleBase>(Actor))
        {
            if (Vehicle->VehicleId == EntityId)
            {
                EntityActorCache.Add(EntityId, Actor);
                return Actor;
            }
        }

        const UTMOPWorldEntityComponent* Identity =
            IsValid(Actor)
                ? Actor->FindComponentByClass<UTMOPWorldEntityComponent>()
                : nullptr;
        if (Identity != nullptr && Identity->EntityId == EntityId)
        {
            EntityActorCache.Add(EntityId, Actor);
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
    TMap<FName, FName> InterpolatedObservationOwners;
    TMap<FName, FName> InterpolatedEntityOwners;

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
        if (Definition.ObserverEntityIds.IsEmpty() &&
            !Definition.bAllowUnattributedObservation)
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
        if (Definition.ObservedPersonIdentityStatus !=
                ETMOPObservedPersonIdentityStatus::Unclassified &&
            Definition.ObservedEntityType != ETMOPObservedEntityType::Person)
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' has a person identity classification but is not a Person observation."),
                *Pair.Key.ToString()));
        if (Definition.bNoFurtherSignalementInSource &&
            !Definition.bSignalementSourceReviewed)
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' says no further signalement exists, but its source is not marked reviewed."),
                *Pair.Key.ToString()));
        if (Definition.bNoFurtherSignalementInSource &&
            !Definition.WitnessSignalements.IsEmpty())
            OutErrors.Add(FString::Printf(
                TEXT("Observation '%s' both contains signalement and says none exists in the source."),
                *Pair.Key.ToString()));
        for (const FTMOPObservationWitnessSignalement& Signalement :
            Definition.WitnessSignalements)
        {
            if (!Signalement.ObserverEntityId.IsNone() &&
                !Definition.ObserverEntityIds.Contains(
                    Signalement.ObserverEntityId))
                OutErrors.Add(FString::Printf(
                    TEXT("Observation '%s' signalement is attributed to '%s', who is not in ObserverEntityIds."),
                    *Pair.Key.ToString(),
                    *Signalement.ObserverEntityId.ToString()));
            const bool bPartialAgeRange =
                (Signalement.EstimatedAgeMinimum > 0) !=
                    (Signalement.EstimatedAgeMaximum > 0);
            if (bPartialAgeRange ||
                Signalement.EstimatedAgeMaximum <
                    Signalement.EstimatedAgeMinimum)
                OutErrors.Add(FString::Printf(
                    TEXT("Observation '%s' has an invalid signalement age range."),
                    *Pair.Key.ToString()));
            const bool bPartialHeightRange =
                (Signalement.EstimatedHeightMinimumCm > 0.0f) !=
                    (Signalement.EstimatedHeightMaximumCm > 0.0f);
            if (bPartialHeightRange ||
                Signalement.EstimatedHeightMaximumCm <
                    Signalement.EstimatedHeightMinimumCm)
                OutErrors.Add(FString::Printf(
                    TEXT("Observation '%s' has an invalid signalement height range."),
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
        if (Link.SimulationMode ==
                ETMOPObservationTrackSimulationMode::InterpolateExistingActor &&
            Link.LinkedEntityId.IsNone())
            OutErrors.Add(FString::Printf(
                TEXT("Interpolated link '%s' needs a LinkedEntityId."),
                *Pair.Key.ToString()));
        else if (Link.SimulationMode ==
                ETMOPObservationTrackSimulationMode::InterpolateExistingActor)
        {
            if (const FName* ExistingLink =
                InterpolatedEntityOwners.Find(Link.LinkedEntityId))
                OutErrors.Add(FString::Printf(
                    TEXT("Entity '%s' is controlled by both interpolated links '%s' and '%s'."),
                    *Link.LinkedEntityId.ToString(),
                    *ExistingLink->ToString(), *Pair.Key.ToString()));
            else
                InterpolatedEntityOwners.Add(Link.LinkedEntityId, Pair.Key);
        }

        ETMOPObservedEntityType FirstType = ETMOPObservedEntityType::Unknown;
        for (const FName ObservationId : MemberIds)
        {
            if (Link.SimulationMode ==
                ETMOPObservationTrackSimulationMode::InterpolateExistingActor)
            {
                if (const FName* ExistingOwner =
                    InterpolatedObservationOwners.Find(ObservationId))
                    OutErrors.Add(FString::Printf(
                        TEXT("Observation '%s' is controlled by both interpolated links '%s' and '%s'."),
                        *ObservationId.ToString(), *ExistingOwner->ToString(),
                        *Pair.Key.ToString()));
                else
                    InterpolatedObservationOwners.Add(ObservationId, Pair.Key);
            }
            const FTMOPObservationDefinition* Observation =
                LoadedObservations.Find(ObservationId);
            if (Observation == nullptr)
            {
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' references unknown observation '%s'."),
                    *Pair.Key.ToString(), *ObservationId.ToString()));
                continue;
            }
            if (Observation->ObservedPersonIdentityStatus ==
                ETMOPObservedPersonIdentityStatus::KnownPerson)
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' contains known-person observation '%s'; known people must be associated directly and cannot use ObservationLinks."),
                    *Pair.Key.ToString(), *ObservationId.ToString()));
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

        if (Link.bRequireSignalementCompatibility &&
            FirstType == ETMOPObservedEntityType::Person)
        {
            for (int32 MemberIndex = 0;
                MemberIndex + 1 < MemberIds.Num(); ++MemberIndex)
            {
                const FTMOPObservationDefinition* FirstObservation =
                    LoadedObservations.Find(MemberIds[MemberIndex]);
                const FTMOPObservationDefinition* SecondObservation =
                    LoadedObservations.Find(MemberIds[MemberIndex + 1]);
                if (FirstObservation == nullptr || SecondObservation == nullptr)
                    continue;
                const FTMOPSignalementComparison Comparison =
                    UTMOPObservationSignalementLibrary::CompareSignalements(
                        *FirstObservation, *SecondObservation);
                if (!Comparison.bHasComparableEvidence)
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' requires signalement matching, but '%s' and '%s' have no comparable structured evidence."),
                        *Pair.Key.ToString(),
                        *MemberIds[MemberIndex].ToString(),
                        *MemberIds[MemberIndex + 1].ToString()));
                else if (Comparison.CompatibilityScore <
                    Link.MinimumSignalementCompatibility)
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' signalement mismatch between '%s' and '%s': %.0f%% is below %.0f%%."),
                        *Pair.Key.ToString(),
                        *MemberIds[MemberIndex].ToString(),
                        *MemberIds[MemberIndex + 1].ToString(),
                        Comparison.CompatibilityScore * 100.0f,
                        Link.MinimumSignalementCompatibility * 100.0f));
            }
        }

        for (const FTMOPObservationTrackSegment& Segment : Link.TrackSegments)
        {
            if (!MemberIds.Contains(Segment.FromObservationId) ||
                !MemberIds.Contains(Segment.ToObservationId) ||
                Segment.FromObservationId == Segment.ToObservationId)
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' has a track segment outside its ObservationIds array."),
                    *Pair.Key.ToString()));

            const bool bPersonVehicleTransition =
                Segment.MovementMode ==
                    ETMOPObservationSegmentMovementMode::WalkToVehicleAndBoard ||
                Segment.MovementMode ==
                    ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard ||
                Segment.MovementMode ==
                    ETMOPObservationSegmentMovementMode::RideInVehicle ||
                Segment.MovementMode ==
                    ETMOPObservationSegmentMovementMode::ExitVehicleThenWalk ||
                Segment.MovementMode ==
                    ETMOPObservationSegmentMovementMode::ExitVehicleThenRun;
            const ETMOPObservedEntityType EffectiveLinkType =
                Link.LinkedEntityType != ETMOPObservedEntityType::Unknown
                ? Link.LinkedEntityType : FirstType;
            if (bPersonVehicleTransition &&
                EffectiveLinkType != ETMOPObservedEntityType::Person)
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' uses a person/vehicle transition but LinkedEntityType is not Person."),
                    *Pair.Key.ToString()));
            if (bPersonVehicleTransition && Segment.VehicleEntityId.IsNone())
                OutErrors.Add(FString::Printf(
                    TEXT("Link '%s' has a boarding/riding/exiting segment without VehicleEntityId."),
                    *Pair.Key.ToString()));

            if (Segment.MovementMode ==
                ETMOPObservationSegmentMovementMode::VehicleLaneRoute)
            {
                const FName EffectiveVehicleId =
                    !Segment.VehicleEntityId.IsNone()
                    ? Segment.VehicleEntityId : Link.LinkedEntityId;
                if (EffectiveVehicleId.IsNone())
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' has a vehicle lane segment without a vehicle entity."),
                        *Pair.Key.ToString()));
                if (Segment.DriverEntityId.IsNone())
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' has a vehicle lane segment without DriverEntityId."),
                        *Pair.Key.ToString()));
                if (Segment.VehicleRouteMode ==
                        ETMOPVehicleRouteMode::ManualLaneRoute &&
                    Segment.OrderedLaneIds.IsEmpty())
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' has a manual vehicle route without OrderedLaneIds."),
                        *Pair.Key.ToString()));
                if (Segment.VehicleRouteMode !=
                        ETMOPVehicleRouteMode::ManualLaneRoute &&
                    Segment.VehicleDestinationAnchorId.IsNone())
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' has an automatic vehicle route without VehicleDestinationAnchorId."),
                        *Pair.Key.ToString()));
            }

            TSet<FName> UniqueLaneIds;
            for (const FName LaneId : Segment.OrderedLaneIds)
            {
                if (LaneId.IsNone() || UniqueLaneIds.Contains(LaneId))
                    OutErrors.Add(FString::Printf(
                        TEXT("Link '%s' has an empty or duplicate OrderedLaneId."),
                        *Pair.Key.ToString()));
                UniqueLaneIds.Add(LaneId);
            }
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
