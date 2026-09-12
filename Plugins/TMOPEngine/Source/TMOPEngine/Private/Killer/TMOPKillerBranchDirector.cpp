#include "Killer/TMOPKillerBranchDirector.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Killer/TMOPKillerBranchPoint.h"
#include "Killer/TMOPKillerGhost.h"
#include "Materials/MaterialInterface.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Time/TMOPClockSubsystem.h"
#include "TMOPKillerBranchSchedule.h"

struct FTMOPKillerBranchRuntime
{
    struct FPath { TArray<FVector> Points; TArray<double> Distances; };
    TArray<TWeakObjectPtr<ATMOPKillerBranchPoint>> Corners;
    TArray<FPath> Paths;
    std::vector<TMOPKillerBranches::Edge> Edges;
    std::vector<TMOPKillerBranches::Seed> Seeds;
    TMOPKillerBranches::Schedule Schedule;
    TMap<int32, double> ObservedArrivalTimes;
    TWeakObjectPtr<ATMOPHistoricalAgent> Original;
    FVector PreviousOriginalLocation = FVector::ZeroVector;
    double PreviousSecond = -1.0;
    double NextDiscoveryTime = 0.0;
    int32 LoopNumber = INDEX_NONE;
    bool bHavePreviousOriginal = false;
    bool bWarnedMissingAppearance = false;
};

void FTMOPKillerBranchRuntimeDeleter::operator()(FTMOPKillerBranchRuntime* Value) const
{
    delete Value;
}

ATMOPKillerBranchDirector::ATMOPKillerBranchDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    SetActorEnableCollision(false);
    Runtime.Reset(new FTMOPKillerBranchRuntime());
}
ATMOPKillerBranchDirector::~ATMOPKillerBranchDirector() = default;

void ATMOPKillerBranchDirector::BeginPlay()
{
    Super::BeginPlay();
    // One director owns a network; prevent accidental duplicate ghost populations.
    for (TActorIterator<ATMOPKillerBranchDirector> It(GetWorld()); It; ++It)
        if (*It != this && It->NetworkId == NetworkId && It->GetName() < GetName())
        {
            UE_LOG(LogTemp, Warning, TEXT("Killer branches: duplicate director '%s' disabled."), *GetName());
            bEnabled = false;
            return;
        }
    RebuildNetwork();
}

void ATMOPKillerBranchDirector::EndPlay(const EEndPlayReason::Type Reason)
{
    ResetBranches();
    Super::EndPlay(Reason);
}

void ATMOPKillerBranchDirector::ClearVisuals()
{
    for (auto& Pair : Ghosts) if (IsValid(Pair.Value)) Pair.Value->Destroy();
    Ghosts.Reset();
}

void ATMOPKillerBranchDirector::ResetBranches()
{
    ClearVisuals();
    if (IsValid(AppearanceTemplate)) AppearanceTemplate->Destroy();
    AppearanceTemplate = nullptr;
    Runtime->Seeds.clear();
    Runtime->Schedule = {};
    Runtime->ObservedArrivalTimes.Reset();
    Runtime->Original.Reset();
    Runtime->PreviousSecond = -1.0;
    Runtime->bHavePreviousOriginal = false;
}

bool ATMOPKillerBranchDirector::RebuildNetwork()
{
    ResetBranches();
    Runtime->Corners.Reset();
    Runtime->Paths.Reset();
    Runtime->Edges.clear();
    if (!IsValid(GetWorld())) return false;
    TArray<ATMOPKillerBranchPoint*> Corners;
    for (TActorIterator<ATMOPKillerBranchPoint> It(GetWorld()); It; ++It)
        if (It->NetworkId == NetworkId) Corners.Add(*It);
    Corners.Sort([](const ATMOPKillerBranchPoint& A, const ATMOPKillerBranchPoint& B)
        { return A.GetName() < B.GetName(); });
    if (Corners.Num() > 128)
    {
        UE_LOG(LogTemp, Error, TEXT("Killer branches: maximum 128 corners per network."));
        return false;
    }
    for (auto* Corner : Corners) Runtime->Corners.Add(Corner);
    auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!IsValid(Nav)) return false;
    for (int32 From = 0; From < Corners.Num(); ++From)
    {
        TSet<int32> Seen;
        for (const auto& DestinationReference : Corners[From]->NextCorners)
        {
            auto* Destination = DestinationReference.Get();
            const int32 To = Corners.IndexOfByKey(Destination);
            if (To == INDEX_NONE || To == From || Seen.Contains(To)) continue;
            Seen.Add(To);
            FNavLocation Start, End;
            const FVector Extent(80.0f, 80.0f, 160.0f);
            if (!Nav->ProjectPointToNavigation(Corners[From]->GetActorLocation(), Start, Extent) ||
                !Nav->ProjectPointToNavigation(Destination->GetActorLocation(), End, Extent)) continue;
            UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
                GetWorld(), Start.Location, End.Location);
            if (!IsValid(Path) || !Path->IsValid() || Path->IsPartial() || Path->PathPoints.Num() < 2 ||
                FVector::Dist(Path->PathPoints.Last(), End.Location) > 80.0f) continue;
            FTMOPKillerBranchRuntime::FPath Cached;
            Cached.Points = Path->PathPoints;
            Cached.Distances.Add(0.0);
            for (int32 I = 1; I < Cached.Points.Num(); ++I)
                Cached.Distances.Add(Cached.Distances.Last() + FVector::Dist(Cached.Points[I - 1], Cached.Points[I]));
            if (Cached.Distances.Last() < 10.0) continue;
            Runtime->Paths.Add(MoveTemp(Cached));
            Runtime->Edges.push_back({From, To, Runtime->Paths.Last().Distances.Last() /
                FMath::Clamp(GhostRunSpeed, 100.0f, 1000.0f)});
        }
    }
    return !Runtime->Edges.empty();
}

void ATMOPKillerBranchDirector::ValidateNetwork()
{
    RebuildNetwork();
    int32 RequestedLinks = 0;
    for (const auto& Weak : Runtime->Corners)
        if (const auto* Corner = Weak.Get())
        {
            RequestedLinks += Corner->NextCorners.Num();
            if (Corner->bBranchWhenOriginalPasses && !IsValid(Corner->OriginalNextCorner))
                UE_LOG(LogTemp, Warning, TEXT("Killer corner '%s': Original Next Corner is empty; verify this is the end of the original route."), *Corner->GetName());
        }
    UE_LOG(LogTemp, Display, TEXT("Killer branches: %d corners, %d authored links, %d complete navigation paths. Missing/duplicate/partial links are skipped. Run animation: %s, material: %s."),
        Runtime->Corners.Num(), RequestedLinks, static_cast<int32>(Runtime->Edges.size()),
        *GetNameSafe(RunAnimation), *GetNameSafe(GhostMaterial));
}

void ATMOPKillerBranchDirector::RebuildSchedule()
{
    Runtime->Schedule = TMOPKillerBranches::Build(Runtime->Edges, Runtime->Seeds,
        FMath::Clamp(MaxConcurrentGhosts, 1, 64), FMath::Clamp(MaxBranchDepth, 1, 16),
        FMath::Clamp(MaxScheduledLegs, 1, 2048));
    if (Runtime->Schedule.Pruned)
        UE_LOG(LogTemp, Display, TEXT("Killer branches: %d alternatives limited by the configured budgets."), Runtime->Schedule.Pruned);
}

void ATMOPKillerBranchDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsValid(GetWorld())) return;
    if (!GetWorld()->IsGameWorld())
    {
#if WITH_EDITOR
        if (bDrawNetworkInEditor)
            for (TActorIterator<ATMOPKillerBranchPoint> It(GetWorld()); It; ++It)
                if (It->NetworkId == NetworkId)
                    for (const auto& Next : It->NextCorners)
                        if (IsValid(Next)) DrawDebugDirectionalArrow(GetWorld(), It->GetActorLocation() + FVector(0, 0, 40),
                            Next->GetActorLocation() + FVector(0, 0, 40), 60,
                            Next == It->OriginalNextCorner ? FColor::White : FColor::Cyan, false, 0, 0, 3);
#endif
        return;
    }
    UGameInstance* GI = GetGameInstance();
    auto* Clock = IsValid(GI) ? GI->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    auto* Events = IsValid(GI) ? GI->GetSubsystem<UTMOPHistoricalEventSubsystem>() : nullptr;
    if (!bEnabled || !IsValid(Clock) || !IsValid(Events))
    {
        if (IsValid(AppearanceTemplate) || !Ghosts.IsEmpty()) ResetBranches();
        return;
    }
    const double Now = Clock->GetCurrentTimeSecondsExact();
    if (Runtime->LoopNumber != Clock->GetLoopNumber())
    {
        ResetBranches();
        Runtime->LoopNumber = Clock->GetLoopNumber();
    }
    FTMOPHistoricalEventRuntime EscapeEvent;
    if (!Events->TryGetEventRuntime(EscapeEventId, EscapeEvent) || !EscapeEvent.bHasResolvedTime ||
        EscapeEvent.State != ETMOPEventRuntimeState::Triggered ||
        Now < EscapeEvent.ResolvedTime.ToSecondsFromMidnight() + EscapeDelaySeconds)
    {
        ClearVisuals();
        Runtime->bHavePreviousOriginal = false;
        Runtime->PreviousSecond = Now;
        return;
    }
    const double RealNow = GetWorld()->GetTimeSeconds();
    if (RealNow >= Runtime->NextDiscoveryTime)
    {
        Runtime->NextDiscoveryTime = RealNow + 1.0;
        if (Runtime->Edges.empty()) RebuildNetwork();
        if (!Runtime->Original.IsValid())
            for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It)
                if (IsValid(It->EntityIdentity.Get()) && It->EntityIdentity->EntityId == FName(TEXT("THE_KILLER")))
                {
                    Runtime->Original = *It;
                    Runtime->bHavePreviousOriginal = false;
                    break;
                }
    }
    auto* Original = Runtime->Original.Get();
    if (!IsValid(AppearanceTemplate) && IsValid(Original) && !Original->IsHidden())
    {
        if (!IsValid(RunAnimation) || !IsValid(GhostMaterial))
        {
            if (!Runtime->bWarnedMissingAppearance)
                UE_LOG(LogTemp, Warning, TEXT("Killer branches: assign Run Animation and Ghost Material (setup script)."));
            Runtime->bWarnedMissingAppearance = true;
            return;
        }
        FActorSpawnParameters Params;
        Params.ObjectFlags |= RF_Transient;
        AppearanceTemplate = GetWorld()->SpawnActor<ATMOPKillerGhost>(ATMOPKillerGhost::StaticClass(), FTransform::Identity, Params);
        if (!IsValid(AppearanceTemplate) || !AppearanceTemplate->InitializeVisual(Original, RunAnimation, GhostMaterial, GhostColor))
        {
            if (IsValid(AppearanceTemplate)) AppearanceTemplate->Destroy();
            AppearanceTemplate = nullptr;
            return;
        }
        AppearanceTemplate->SetActorHiddenInGame(true);
    }
    if (Runtime->Edges.empty() || !IsValid(AppearanceTemplate)) return;

    const double Step = Now - Runtime->PreviousSecond;
    const bool bContinuous = Runtime->bHavePreviousOriginal && Step > 0.0 && Step <= 2.0;
    const FVector Position = IsValid(Original) ? Original->GetActorLocation() -
        FVector(0, 0, Original->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) : FVector::ZeroVector;
    std::vector<TMOPKillerBranches::Seed> NewSeeds;
    for (int32 I = 0; I < Runtime->Corners.Num(); ++I)
    {
        const auto* Corner = Runtime->Corners[I].Get();
        if (!IsValid(Corner) || !Corner->bBranchWhenOriginalPasses) continue;
        double Arrival = -1.0;
        if (Corner->bUseSharedEventArrival)
        {
            FTMOPHistoricalEventRuntime Event;
            if (Events->TryGetEventRuntime(Corner->ArrivalEventId, Event) && Event.bHasResolvedTime &&
                Event.State != ETMOPEventRuntimeState::Cancelled)
                Arrival = Event.ResolvedTime.ToSecondsFromMidnight() + Corner->ArrivalEventOffsetSeconds;
        }
        else if (const double* Recorded = Runtime->ObservedArrivalTimes.Find(I)) Arrival = *Recorded;
        else if (IsValid(Original) && !Original->IsHidden())
        {
            // Do not interpret a world-seek teleport as crossing all intervening corners.
            const bool bUseSegment = bContinuous && FVector::Dist(Position, Runtime->PreviousOriginalLocation) <=
                2000.0 * Step + Corner->ArrivalRadiusCm;
            const FVector Start = bUseSegment ? Runtime->PreviousOriginalLocation : Position;
            const FVector Delta = Position - Start;
            const double T = Delta.SizeSquared() > 1.0 ? FMath::Clamp(
                FVector::DotProduct(Corner->GetActorLocation() - Start, Delta) / Delta.SizeSquared(), 0.0, 1.0) : 1.0;
            if (FVector::DistSquared(Start + Delta * T, Corner->GetActorLocation()) <= FMath::Square(Corner->ArrivalRadiusCm))
            {
                Arrival = bUseSegment ? Runtime->PreviousSecond + Step * T : Now;
                Runtime->ObservedArrivalTimes.Add(I, Arrival);
            }
        }
        if (Arrival < EscapeEvent.ResolvedTime.ToSecondsFromMidnight() + EscapeDelaySeconds) continue;
        const int32 Next = Runtime->Corners.IndexOfByPredicate([Corner](const auto& W)
            { return W.Get() == Corner->OriginalNextCorner.Get(); });
        const int32 Previous = Runtime->Corners.IndexOfByPredicate([Corner](const auto& W)
            { return W.Get() == Corner->OriginalPreviousCorner.Get(); });
        NewSeeds.push_back({I, Next, Previous, Arrival});
    }
    const bool bSeedsChanged = NewSeeds.size() != Runtime->Seeds.size() ||
        !std::equal(NewSeeds.begin(), NewSeeds.end(), Runtime->Seeds.begin(), [](const auto& A, const auto& B)
        { return A.Node == B.Node && A.OriginalNext == B.OriginalNext && A.OriginalPrevious == B.OriginalPrevious &&
            FMath::Abs(A.Time - B.Time) < 0.001; });
    if (bSeedsChanged)
    {
        Runtime->Seeds = MoveTemp(NewSeeds);
        RebuildSchedule();
    }
    Runtime->PreviousSecond = Now;
    Runtime->PreviousOriginalLocation = Position;
    Runtime->bHavePreviousOriginal = IsValid(Original);

    TSet<FString> ActiveKeys;
    for (const auto& Leg : Runtime->Schedule.Legs)
    {
        if (!TMOPKillerBranches::Active(Leg, Now)) continue;
        const FString Key(UTF8_TO_TCHAR(Leg.Key.c_str()));
        ActiveKeys.Add(Key);
    }
    for (auto It = Ghosts.CreateIterator(); It; ++It)
        if (!ActiveKeys.Contains(It.Key()))
        {
            if (IsValid(It.Value())) It.Value()->Destroy();
            It.RemoveCurrent();
        }
    for (const auto& Leg : Runtime->Schedule.Legs)
    {
        if (!TMOPKillerBranches::Active(Leg, Now)) continue;
        const FString Key(UTF8_TO_TCHAR(Leg.Key.c_str()));
        auto* Ghost = Ghosts.FindRef(Key).Get();
        if (!IsValid(Ghost))
        {
            FActorSpawnParameters Params;
            Params.ObjectFlags |= RF_Transient;
            Ghost = GetWorld()->SpawnActor<ATMOPKillerGhost>(ATMOPKillerGhost::StaticClass(), FTransform::Identity, Params);
            if (!IsValid(Ghost)) continue;
            if (!Ghost->InitializeVisual(AppearanceTemplate, RunAnimation, GhostMaterial, GhostColor))
            {
                Ghost->Destroy();
                continue;
            }
            Ghosts.Add(Key, Ghost);
        }
        const auto& Path = Runtime->Paths[Leg.EdgeIndex];
        const double Distance = FMath::Clamp((Now - Leg.Start) / (Leg.End - Leg.Start), 0.0, 1.0) * Path.Distances.Last();
        int32 Segment = 1;
        while (Segment + 1 < Path.Points.Num() && Path.Distances[Segment] < Distance) ++Segment;
        const double Length = Path.Distances[Segment] - Path.Distances[Segment - 1];
        const FVector Location = FMath::Lerp(Path.Points[Segment - 1], Path.Points[Segment],
            Length > 0.0 ? (Distance - Path.Distances[Segment - 1]) / Length : 0.0);
        const FVector Direction = Path.Points[Segment] - Path.Points[Segment - 1];
        Ghost->SetActorLocationAndRotation(Location + FVector(0, 0, Ghost->GetGroundOffset()),
            FRotator(0, Direction.Rotation().Yaw, 0));
        const double Fade = FMath::Max(0.05, static_cast<double>(FadeSeconds));
        const double FadeIn = FMath::Clamp((Now - Leg.RootTime) / Fade, 0.0, 1.0);
        const double FadeOut = Leg.Terminal ? FMath::Clamp((Leg.End - Now) / Fade, 0.0, 1.0) : 1.0;
        Ghost->SampleAnimation(Now - Leg.RootTime, FMath::Clamp(AnimationPlayRate, 0.1f, 3.0f),
            FMath::Clamp(GhostOpacity, 0.01f, 0.8f) * FadeIn * FadeOut);
    }
}
