#include "Vehicles/TMOPVehicleRoutePlan.h"
#include "Vehicles/TMOPVehicleRouteMath.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "JsonObjectConverter.h"
#include "Misc/Crc.h"

void FTMOPVehicleRoutePlan::AddSample(const FTransform& Pose)
{
    if (!Samples.IsEmpty()) LengthCm += FVector::Distance(
        Samples.Last().GetLocation(), Pose.GetLocation());
    Samples.Add(Pose);
    SampleDistances.Add(LengthCm);
}

FTransform FTMOPVehicleRoutePlan::Sample(double DistanceCm) const
{
    if (Samples.IsEmpty()) return FTransform::Identity;
    if (DistanceCm <= 0.0) return Samples[0];
    if (DistanceCm >= LengthCm) return Samples.Last();
    int32 Low = 1, High = SampleDistances.Num() - 1;
    while (Low < High)
    {
        const int32 Mid = (Low + High) / 2;
        if (SampleDistances[Mid] < DistanceCm) Low = Mid + 1;
        else High = Mid;
    }
    const double Span = SampleDistances[Low] - SampleDistances[Low - 1];
    const float Alpha = Span > UE_SMALL_NUMBER
        ? float((DistanceCm - SampleDistances[Low - 1]) / Span) : 1.0f;
    FTransform Pose;
    Pose.Blend(Samples[Low - 1], Samples[Low], Alpha);
    return Pose;
}

namespace TMOPVehicleRoute
{
bool IsDriving(ETMOPHistoricalVehicleAction Action)
{
    return Action == ETMOPHistoricalVehicleAction::BeginDriving ||
        Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
}
bool IsStop(ETMOPHistoricalVehicleAction Action)
{
    return Action == ETMOPHistoricalVehicleAction::Stop ||
        Action == ETMOPHistoricalVehicleAction::Park ||
        Action == ETMOPHistoricalVehicleAction::ExitTrafficRoute;
}
bool HasPlacement(ETMOPHistoricalVehicleAction Action)
{
    return IsStop(Action) || Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
        Action == ETMOPHistoricalVehicleAction::Spawn ||
        Action == ETMOPHistoricalVehicleAction::OffscreenTransfer;
}
int32 CompletionDelay(const FTMOPHistoricalVehicleTimelineEntry& Entry)
{
    if (Entry.Action == ETMOPHistoricalVehicleAction::OffscreenTransfer)
        return FMath::Max(0, Entry.OffscreenTransferDurationSeconds);
    return IsStop(Entry.Action) && Entry.bUseStopDuration
        ? FMath::Max(0, Entry.StopDurationSeconds) : 0;
}
FName Driver(const FTMOPHistoricalVehicleRow& Row,
    const FTMOPHistoricalVehicleTimelineEntry& Entry)
{
    if (!Entry.DriverEntityId.IsNone()) return Entry.DriverEntityId;
    // Use row identity, not pointer arithmetic: editors can pass an entry copy.
    // Never inherit from a future row or across a despawn/new vehicle life.
    int32 EntryIndex = INDEX_NONE;
    for (int32 Index = 0; Index < Row.Timeline.Num(); ++Index)
        if (&Row.Timeline[Index] == &Entry) { EntryIndex = Index; break; }
    if (EntryIndex == INDEX_NONE && !Entry.EntryId.IsNone())
        EntryIndex = Row.Timeline.IndexOfByPredicate([&Entry](const auto& Candidate)
            { return Candidate.EntryId == Entry.EntryId; });
    for (int32 Index = EntryIndex - 1; Index >= 0; --Index)
    {
        const auto& Previous = Row.Timeline[Index];
        if (Previous.Action == ETMOPHistoricalVehicleAction::Despawn) break;
        if (!Previous.DriverEntityId.IsNone()) return Previous.DriverEntityId;
        if (Previous.Action == ETMOPHistoricalVehicleAction::Spawn ||
            Previous.Action == ETMOPHistoricalVehicleAction::InitialPlacement) break;
    }
    return Row.KnownDriverEntityId;
}

void BuildManeuver(const TArray<FTransform>& Anchors, float Strength,
    bool bReverse, ETMOPVehicleManeuverTurn Turn, float RadiusCm,
    FTMOPVehicleRoutePlan& Out)
{
    Out = FTMOPVehicleRoutePlan();
    Out.bAnchorManeuver = true;
    Out.Anchors = Anchors;
    if (Anchors.Num() < 2) return;
    Out.bHasDestination = true;
    Out.Destination = Anchors.Last();
    const float Direction = bReverse ? -1.0f : 1.0f;
    for (int32 Segment = 0; Segment + 1 < Anchors.Num(); ++Segment)
    {
        const FTransform& From = Anchors[Segment];
        const FTransform& To = Anchors[Segment + 1];
        const FVector P0 = From.GetLocation(), P3 = To.GetLocation();
        const double Chord = FVector::Distance(P0, P3);
        const double Handle = RadiusCm > 0.0f ? RadiusCm * 1.333333333
            : FMath::Max(10.0, Chord * double(FMath::Clamp(Strength, 0.05f, 2.0f)));
        const FVector Forward = From.GetRotation().GetForwardVector() * Direction;
        const FVector P1 = P0 + Forward * Handle;
        const FVector P2 = P3 - To.GetRotation().GetForwardVector() * Direction * Handle;
        // A smooth lateral bump selects the side of a tight turnaround. Its
        // derivative vanishes at each end, preserving the anchor headings.
        const double TurnSign = Turn == ETMOPVehicleManeuverTurn::Left ? -1.0 :
            Turn == ETMOPVehicleManeuverTurn::Right ? 1.0 : 0.0;
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
        const FVector Bias = Side * TurnSign * FMath::Max(Chord * 0.5, double(RadiusCm));
        const int32 Steps = FMath::Clamp(FMath::CeilToInt((Chord + Handle * 2.0) / 20.0), 32, 2048);
        for (int32 Step = Segment == 0 ? 0 : 1; Step <= Steps; ++Step)
        {
            const double T = double(Step) / Steps;
            const auto Curve = TMOPVehicleRouteMath::Cubic(P0, P1, P2, P3, Bias, T);
            const FVector Position = Curve.Position;
            FQuat Rotation = (Curve.Tangent * Direction).Rotation().Quaternion();
            if (Step == 0) Rotation = From.GetRotation();
            if (Step == Steps) Rotation = To.GetRotation();
            Out.AddSample(FTransform(Rotation, Position, FVector::OneVector));
        }
        Out.ViaDistances.Add(Out.LengthCm);
    }
}

double DistanceAtTime(const FTMOPVehicleRoutePlan& Plan, double Alpha, bool bStopAtViaAnchors)
{
    Alpha = FMath::Clamp(Alpha, 0.0, 1.0);
    if (!Plan.bAnchorManeuver) return Plan.LengthCm * Alpha;
    // Ease once over the whole maneuver. Via anchors are pass-through unless
    // the author explicitly requests a brief zero-speed touch at every via.
    double Start = 0.0, End = Plan.LengthCm;
    if (bStopAtViaAnchors)
    {
        const double RawDistance = Alpha * Plan.LengthCm;
        for (double Via : Plan.ViaDistances)
        {
            End = Via;
            if (RawDistance <= Via) break;
            Start = Via;
        }
        Alpha = End > Start ? (RawDistance - Start) / (End - Start) : 1.0;
    }
    return Start + (End - Start) * TMOPVehicleRouteMath::SmoothProgress(Alpha);
}

bool Build(UWorld* World, const FTMOPHistoricalVehicleRow& Row, int32 Index,
    FTMOPVehicleRoutePlan& Out, FString& Failure, bool bRebuildManual)
{
    Out = FTMOPVehicleRoutePlan(); Failure.Reset();
    if (!World || !Row.Timeline.IsValidIndex(Index) || !IsDriving(Row.Timeline[Index].Action))
    { Failure = TEXT("Select a driving row and open its level."); return false; }
    const auto& Entry = Row.Timeline[Index];
    TMap<FName, FTransform> Anchors;
    TMap<FName, UTMOPTrafficLaneComponent*> Lanes;
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        if (!It->GetAnchorId().IsNone()) Anchors.Add(It->GetAnchorId(), FTransform(
            It->GetAnchorRotation(), It->GetAnchorLocation(), FVector::OneVector));
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Components;
        It->GetComponents(Components);
        for (auto* Lane : Components)
            if (IsValid(Lane) && !Lane->LaneId.IsNone()) Lanes.Add(Lane->LaneId, Lane);
    }
    auto Placement = [&Anchors](const FTMOPHistoricalVehicleTimelineEntry& Item, FTransform& Pose)
    {
        if (Item.PlacementMode == ETMOPHistoricalVehiclePlacementMode::WorldTransform)
        { Pose = Item.WorldTransform; return true; }
        if (const FTransform* Anchor = Anchors.Find(Item.PlacementAnchorId))
        { Pose = Item.AnchorLocalOffset * *Anchor; return true; }
        return false;
    };
    auto ResolveAnchor = [&Anchors, &Failure](FName Id, FTransform& Pose)
    {
        if (const FTransform* Found = Anchors.Find(Id)) { Pose = *Found; return true; }
        Failure = FString::Printf(TEXT("Anchor '%s' is missing in the open level."), *Id.ToString());
        return false;
    };
    FTransform Start, End;
    bool bStart = false, bEnd = false;
    Out.StartAnchorId = Entry.RouteStartAnchorId;
    Out.EndAnchorId = Entry.RouteDestinationAnchorId;
    if (!Out.StartAnchorId.IsNone())
    { if (!ResolveAnchor(Out.StartAnchorId, Start)) return false; bStart = true; }
    else for (int32 Previous = Index - 1; Previous >= 0; --Previous)
    {
        if (IsDriving(Row.Timeline[Previous].Action))
        {
            const FName PreviousEnd = Row.Timeline[Previous].RouteDestinationAnchorId;
            if (!PreviousEnd.IsNone())
            { bStart = ResolveAnchor(PreviousEnd, Start); Out.StartAnchorId = PreviousEnd; }
            break;
        }
        if (!HasPlacement(Row.Timeline[Previous].Action)) continue;
        bStart = Placement(Row.Timeline[Previous], Start);
        Out.StartAnchorId = Row.Timeline[Previous].PlacementAnchorId;
        break;
    }
    if (!Out.EndAnchorId.IsNone())
    { if (!ResolveAnchor(Out.EndAnchorId, End)) return false; bEnd = true; }
    // A matching stop may contain a local parking offset. Use it consistently
    // in the route, the estimate, the preview and the final arrival check.
    for (int32 Next = Index + 1; Next < Row.Timeline.Num(); ++Next)
    {
        const auto& Stop = Row.Timeline[Next];
        if (IsDriving(Stop.Action) || Stop.Action == ETMOPHistoricalVehicleAction::Despawn) break;
        if (!IsStop(Stop.Action)) continue;
        if (Out.EndAnchorId.IsNone() || Out.EndAnchorId == Stop.PlacementAnchorId)
        {
            if (!Placement(Stop, End))
            { Failure = TEXT("The following stop has a missing placement anchor."); return false; }
            Out.EndAnchorId = Stop.PlacementAnchorId; bEnd = true;
        }
        break;
    }
    if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::AnchorManeuver)
    {
        if (!bStart || !bEnd)
        { Failure = TEXT("Free maneuver needs valid start and end anchors (or adjoining placements)."); return false; }
        TArray<FTransform> Points; Points.Add(Start);
        for (FName Id : Entry.RouteViaAnchorIds)
        { FTransform Via; if (!ResolveAnchor(Id, Via)) return false; Points.Add(Via); }
        Points.Add(End);
        const FName StartId = Out.StartAnchorId, EndId = Out.EndAnchorId;
        BuildManeuver(Points, Entry.AnchorManeuverCurveStrength, Entry.bAnchorManeuverReverse,
            Entry.AnchorManeuverTurn, Entry.AnchorManeuverRadiusCm, Out);
        Out.StartAnchorId = StartId; Out.EndAnchorId = EndId;
        if (Out.LengthCm < 1.0) { Failure = TEXT("The maneuver has no travel distance."); return false; }
        return true;
    }
    auto Nearest = [&Lanes](const FVector& Point)
    {
        FName BestId; double Best = TNumericLimits<double>::Max();
        for (const auto& Pair : Lanes)
        {
            const FVector Projected = Pair.Value->FindLocationClosestToWorldLocation(Point, ESplineCoordinateSpace::World);
            const double Distance = FVector::DistSquared(Projected, Point);
            if (Distance < Best || (Distance == Best && Pair.Key.LexicalLess(BestId)))
            { Best = Distance; BestId = Pair.Key; }
        }
        return BestId;
    };
    auto Connected = [&Lanes, &Entry](FName From, FName To)
    {
        auto* const* Lane = Lanes.Find(From);
        if (!Lane || !Lanes.Contains(To)) return false;
        for (const auto& Connection : (*Lane)->NextLanes)
            if (Connection.TargetLaneId == To && (Connection.bAllowed || Entry.bIgnoreOneWayRestrictions)) return true;
        return false;
    };
    auto Shortest = [&Lanes, &Entry](FName From, FName To, TArray<FName>& Result)
    {
        if (!Lanes.Contains(From) || !Lanes.Contains(To)) return false;
        TMap<FName, double> Distances; TMap<FName, FName> Previous;
        TSet<FName> Open; Open.Add(From); Distances.Add(From, 0.0);
        while (!Open.IsEmpty())
        {
            FName Current; double Best = TNumericLimits<double>::Max();
            for (FName Candidate : Open)
            {
                const double Cost = Distances.FindChecked(Candidate);
                if (Cost < Best || (Cost == Best && Candidate.LexicalLess(Current)))
                { Best = Cost; Current = Candidate; }
            }
            Open.Remove(Current);
            if (Current == To)
            {
                for (FName At = To; ; At = Previous.FindChecked(At))
                { Result.Insert(At, 0); if (At == From) return true; }
            }
            for (const auto& Link : Lanes.FindChecked(Current)->NextLanes)
            {
                if ((!Link.bAllowed && !Entry.bIgnoreOneWayRestrictions) || !Lanes.Contains(Link.TargetLaneId)) continue;
                const double Cost = Best + FMath::Max(1.0f, Lanes.FindChecked(Link.TargetLaneId)->GetSplineLength());
                const double* Existing = Distances.Find(Link.TargetLaneId);
                if (!Existing || Cost < *Existing)
                { Distances.Add(Link.TargetLaneId, Cost); Previous.Add(Link.TargetLaneId, Current); Open.Add(Link.TargetLaneId); }
            }
        }
        return false;
    };
    FName StartLane = Entry.RouteStartLaneId, EndLane = Entry.RouteDestinationLaneId;
    if (StartLane.IsNone()) StartLane = bStart ? Nearest(Start.GetLocation()) :
        Entry.OrderedLaneIds.IsEmpty() ? NAME_None : Entry.OrderedLaneIds[0];
    if (EndLane.IsNone()) EndLane = bEnd ? Nearest(End.GetLocation()) :
        Entry.OrderedLaneIds.IsEmpty() ? NAME_None : Entry.OrderedLaneIds.Last();
    if (!Lanes.Contains(StartLane) || !Lanes.Contains(EndLane))
    { Failure = TEXT("Set valid start and destination anchors or lanes."); return false; }
    if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute && !bRebuildManual)
    {
        Out.LaneIds = Entry.OrderedLaneIds;
        if (Out.LaneIds.IsEmpty()) { Failure = TEXT("No lane route. Choose Recalculate route."); return false; }
        // In a stored manual route the first/last lane are authoritative unless
        // the author explicitly selected a different lane in the editor.
        if ((!Entry.RouteStartLaneId.IsNone() && Out.LaneIds[0] != StartLane) ||
            (!Entry.RouteDestinationLaneId.IsNone() && Out.LaneIds.Last() != EndLane))
        { Failure = TEXT("Stored lanes do not match the selected endpoints. Recalculate route."); return false; }
    }
    else
    {
        if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::ManualThenAutomatic && !bRebuildManual)
            Out.LaneIds = Entry.OrderedLaneIds;
        if (Out.LaneIds.IsEmpty()) Out.LaneIds.Add(StartLane);
        TArray<FName> Waypoints;
        for (FName Id : Entry.RouteViaAnchorIds)
        { FTransform Via; if (!ResolveAnchor(Id, Via)) return false; Waypoints.Add(Nearest(Via.GetLocation())); }
        Waypoints.Append(Entry.RouteViaLaneIds); Waypoints.Add(EndLane);
        for (FName To : Waypoints)
        {
            TArray<FName> Segment;
            if (!Shortest(Out.LaneIds.Last(), To, Segment))
            { Failure = FString::Printf(TEXT("No connected route from '%s' to '%s'. Use a free maneuver for an off-lane turn."), *Out.LaneIds.Last().ToString(), *To.ToString()); return false; }
            Segment.RemoveAt(0); Out.LaneIds.Append(Segment);
        }
    }
    for (int32 LaneIndex = 0; LaneIndex < Out.LaneIds.Num(); ++LaneIndex)
    {
        if (!Lanes.Contains(Out.LaneIds[LaneIndex]))
        { Failure = TEXT("A stored lane is missing from the level."); return false; }
        if (LaneIndex && !Connected(Out.LaneIds[LaneIndex - 1], Out.LaneIds[LaneIndex]))
        { Failure = TEXT("Stored route contains disconnected or restricted lanes. Recalculate route."); return false; }
    }
    if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute && !bRebuildManual)
    {
        TArray<FName> RequiredVia;
        for (FName Id : Entry.RouteViaAnchorIds)
        { FTransform Via; if (!ResolveAnchor(Id, Via)) return false; RequiredVia.Add(Nearest(Via.GetLocation())); }
        RequiredVia.Append(Entry.RouteViaLaneIds);
        int32 Cursor = 0;
        for (FName Id : RequiredVia)
        {
            while (Cursor < Out.LaneIds.Num() && Out.LaneIds[Cursor] != Id) ++Cursor;
            if (Cursor == Out.LaneIds.Num())
            { Failure = TEXT("The stored route does not pass the selected via points in order. Recalculate route."); return false; }
        }
    }
    auto* First = Lanes.FindChecked(Out.LaneIds[0]);
    auto* Last = Lanes.FindChecked(Out.LaneIds.Last());
    Out.StartDistanceCm = bStart ? First->GetDistanceAlongSplineAtSplineInputKey(
        First->FindInputKeyClosestToWorldLocation(Start.GetLocation())) : Entry.RouteStartDistanceAlongFirstLaneCm;
    if (Entry.RouteStartDistanceAlongFirstLaneCm > 0.0f) Out.StartDistanceCm = Entry.RouteStartDistanceAlongFirstLaneCm;
    Out.StartDistanceCm = FMath::Clamp(Out.StartDistanceCm, 0.0f, First->GetSplineLength());
    if (bStart && FVector::Distance(Start.GetLocation(), First->GetLaneLocationAtDistance(Out.StartDistanceCm)) > 300.0)
    { Failure = TEXT("The starting placement is more than 3 m from the chosen lane. Choose its lane or add a free maneuver to enter traffic."); return false; }
    Out.EndDistanceCm = bEnd ? Last->GetDistanceAlongSplineAtSplineInputKey(
        Last->FindInputKeyClosestToWorldLocation(End.GetLocation())) : Last->GetSplineLength();
    if (Out.LaneIds.Num() == 1 && Out.EndDistanceCm < Out.StartDistanceCm - 1.0f)
    { Failure = TEXT("Destination is behind the start on the same lane. Use a U-turn/free maneuver."); return false; }
    for (int32 LaneIndex = 0; LaneIndex < Out.LaneIds.Num(); ++LaneIndex)
    {
        auto* Lane = Lanes.FindChecked(Out.LaneIds[LaneIndex]);
        const float Begin = LaneIndex == 0 ? Out.StartDistanceCm : 0.0f;
        const float Finish = LaneIndex == Out.LaneIds.Num() - 1 ? Out.EndDistanceCm : Lane->GetSplineLength();
        const int32 Steps = FMath::Max(1, FMath::CeilToInt((Finish - Begin) / 50.0f));
        for (int32 Step = 0; Step <= Steps; ++Step)
            Out.AddSample(Lane->GetLaneTransformAtDistance(FMath::Lerp(Begin, Finish, float(Step) / Steps)));
    }
    Out.bHasDestination = bEnd;
    Out.Destination = bEnd ? End : Out.Samples.Last();
    if (bEnd) Out.AddSample(End);
    return true;
}

FString Fingerprint(const FTMOPHistoricalVehicleRow& Row, const FTMOPVehicleRoutePlan& Plan,
    int32 Departure, int32 Arrival)
{
    FString Serialized;
    FJsonObjectConverter::UStructToJsonObjectString(Row, Serialized);
    Serialized += FString::Printf(TEXT("|route-v2|%d|%d"), Departure, Arrival);
    for (const auto& Pose : Plan.Samples) Serialized += Pose.ToString();
    return FString::Printf(TEXT("%08x"), FCrc::StrCrc32(*Serialized));
}
FName UniqueEntryId(const FTMOPHistoricalVehicleRow& Row, const FString& Base)
{
    TSet<FName> Used;
    for (const auto& Entry : Row.Timeline) Used.Add(Entry.EntryId);
    for (int32 Suffix = 1; ; ++Suffix)
    {
        const FName Candidate(*FString::Printf(TEXT("%s_%03d"), *Base, Suffix));
        if (!Used.Contains(Candidate)) return Candidate;
    }
}
bool IsStaticSceneryHit(const FHitResult& Hit)
{
    const UPrimitiveComponent* Component = Hit.GetComponent();
    if (!IsValid(Component)) return false;
    // Test the component, not the actor name: Blueprint scenery may also be static.
    return Component->GetCollisionObjectType() == ECC_WorldStatic ||
        (Component->IsA<UStaticMeshComponent>() &&
         Component->Mobility != EComponentMobility::Movable);
}

bool FindObstacle(UWorld* World, const FTMOPVehicleRoutePlan& Plan,
    FVector HalfExtent, FHitResult& Hit, const AActor* IgnoreActor,
    bool bIgnoreStaticScenery)
{
    if (!World) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPManeuverPreview), false, IgnoreActor);
    FCollisionObjectQueryParams Objects;
    if (!bIgnoreStaticScenery) Objects.AddObjectTypesToQuery(ECC_WorldStatic);
    if (IgnoreActor)
    {
        Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
        Objects.AddObjectTypesToQuery(ECC_Vehicle);
        Objects.AddObjectTypesToQuery(ECC_Pawn);
        Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
        TArray<AActor*> Attached;
        IgnoreActor->GetAttachedActors(Attached, true, true);
        Params.AddIgnoredActors(Attached);
    }
    // Lift the sensor above the road so road surfaces are not obstacles.
    const FVector Lift(0.0, 0.0, HalfExtent.Z + 30.0);
    for (int32 Index = 1; Index < Plan.Samples.Num(); ++Index)
    {
        TArray<FHitResult> Hits;
        World->SweepMultiByObjectType(Hits, Plan.Samples[Index - 1].GetLocation() + Lift,
            Plan.Samples[Index].GetLocation() + Lift, Plan.Samples[Index].GetRotation(),
            Objects, FCollisionShape::MakeBox(HalfExtent), Params);
        for (const FHitResult& Candidate : Hits)
        {
            if (bIgnoreStaticScenery && IsStaticSceneryHit(Candidate)) continue;
            Hit = Candidate;
            return true;
        }
    }
    return false;
}
}
