#include "World/TMOPTimedPropDirector.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Items/TMOPItemMeshSubsystem.h"
#include "Items/TMOPWorldItem.h"
#include "World/TMOPFindingActor.h"
#include "Time/TMOPClockSubsystem.h"

ATMOPTimedPropDirector::ATMOPTimedPropDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPTimedPropDirector::BeginPlay()
{
    Super::BeginPlay();
    if (UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr)
        Anchors->DiscoverAnchorsInWorld();
    RestartScheduleAtCurrentTime();
}

void ATMOPTimedPropDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllSpawnedInstances();
    Super::EndPlay(EndPlayReason);
}

void ATMOPTimedPropDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!IsValid(Clock)) return;
    const int32 CurrentSecond =
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE &&
        CurrentSecond < LastEvaluatedSecond)
    {
        RestartScheduleAtCurrentTime();
        return;
    }
    if (CurrentSecond != LastEvaluatedSecond)
    {
        EvaluateSchedule(CurrentSecond, false);
        LastEvaluatedSecond = CurrentSecond;
    }
}

void ATMOPTimedPropDirector::RestartScheduleAtCurrentTime()
{
    DestroyAllSpawnedInstances();
    NextEntryIndex = 0;
    LastResolvedEntrySecond = INDEX_NONE;
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = IsValid(Clock)
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    EvaluateSchedule(CurrentSecond, true);
    LastEvaluatedSecond = CurrentSecond;
}

void ATMOPTimedPropDirector::EvaluateSchedule(
    const int32 CurrentSecond, const bool /*bCatchUp*/)
{
    while (ScheduledEntries.IsValidIndex(NextEntryIndex))
    {
        const FTMOPTimedPropEntry& Entry =
            ScheduledEntries[NextEntryIndex];
        int32 ResolvedSecond = INDEX_NONE;
        if (!ResolveEntrySecond(Entry, ResolvedSecond) ||
            ResolvedSecond > CurrentSecond)
            break;
        if (!ApplyEntry(Entry))
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Timed Prop '%s' could not be applied (Instance '%s')."),
                *Entry.EntryId.ToString(), *Entry.InstanceId.ToString());
        LastResolvedEntrySecond = ResolvedSecond;
        ++NextEntryIndex;
    }
}

bool ATMOPTimedPropDirector::ResolveEntrySecond(
    const FTMOPTimedPropEntry& Entry, int32& OutSecond) const
{
    OutSecond = INDEX_NONE;
    if (Entry.TimingMode == ETMOPEventTimingMode::Absolute)
    {
        OutSecond = Entry.Time.ToSecondsFromMidnight();
        return true;
    }
    if (Entry.TimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
    {
        if (LastResolvedEntrySecond == INDEX_NONE) return false;
        OutSecond = LastResolvedEntrySecond + Entry.OffsetSeconds;
        return true;
    }
    if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
    {
        const UTMOPHistoricalEventSubsystem* Events =
            GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<
                UTMOPHistoricalEventSubsystem>() : nullptr;
        FTMOPHistoricalEventRuntime Runtime;
        if (!IsValid(Events) || Entry.SharedEventId.IsNone() ||
            !Events->TryGetEventRuntime(Entry.SharedEventId, Runtime) ||
            !Runtime.bHasResolvedTime)
            return false;
        OutSecond = Runtime.ResolvedTime.ToSecondsFromMidnight() +
            Entry.OffsetSeconds;
        return true;
    }
    return false;
}

bool ATMOPTimedPropDirector::ApplyEntry(
    const FTMOPTimedPropEntry& Entry)
{
    if (Entry.InstanceId.IsNone() || GetWorld() == nullptr)
        return false;
    if (Entry.Action == ETMOPTimedPropAction::Despawn)
    {
        if (TWeakObjectPtr<AActor>* Found =
            SpawnedInstances.Find(Entry.InstanceId))
            if (AActor* Existing = Found->Get())
                Existing->Destroy();
        SpawnedInstances.Remove(Entry.InstanceId);
        return true;
    }

    FTransform SpawnTransform;
    if (!ResolveSpawnTransform(Entry, SpawnTransform)) return false;
    if (TWeakObjectPtr<AActor>* Found =
        SpawnedInstances.Find(Entry.InstanceId))
        if (AActor* Existing = Found->Get())
            Existing->Destroy();

    AActor* Spawned = nullptr;
    if (Entry.PropKind == ETMOPTimedPropKind::StaticMesh)
    {
        UStaticMesh* Mesh = Entry.StaticMesh.LoadSynchronous();
        if (!IsValid(Mesh)) return false;
        AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), SpawnTransform);
        if (!IsValid(MeshActor)) return false;
        UStaticMeshComponent* Component =
            MeshActor->GetStaticMeshComponent();
        Component->SetMobility(EComponentMobility::Movable);
        Component->SetStaticMesh(Mesh);
        Component->SetCastShadow(Entry.bCastShadow);
        Component->SetCollisionEnabled(Entry.bEnableCollision
            ? ECollisionEnabled::QueryAndPhysics
            : ECollisionEnabled::NoCollision);
        Spawned = MeshActor;
    }
    else if (Entry.PropKind == ETMOPTimedPropKind::ActorClass)
    {
        if (!Entry.ActorClass) return false;
        Spawned = GetWorld()->SpawnActor<AActor>(
            Entry.ActorClass, SpawnTransform);
    }
    else if (Entry.PropKind == ETMOPTimedPropKind::Finding)
    {
        UStaticMesh* ResolvedFindingMesh = Entry.StaticMesh.LoadSynchronous();
        FVector ResolvedFindingScale = Entry.FindingScale;
        const FName FindingCatalogId = !Entry.ItemMeshId.IsNone()
            ? Entry.ItemMeshId : FName(*Entry.EvidenceId);
        if (!IsValid(ResolvedFindingMesh) && !FindingCatalogId.IsNone())
            if (UTMOPItemMeshSubsystem* Catalog = GetGameInstance() != nullptr
                ? GetGameInstance()->GetSubsystem<UTMOPItemMeshSubsystem>() : nullptr)
            {
                FTMOPItemMeshRow Definition;
                if (Catalog->FindItemMeshDefinition(FindingCatalogId, Definition))
                {
                    ResolvedFindingMesh = Definition.Mesh.LoadSynchronous();
                    ResolvedFindingScale = Definition.DefaultFindingScale;
                }
            }
        ATMOPFindingActor* Finding =
            GetWorld()->SpawnActorDeferred<ATMOPFindingActor>(
                ATMOPFindingActor::StaticClass(), SpawnTransform, this, nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!IsValid(Finding)) return false;
        Finding->ConfigureFinding(
            Entry.FindingDisplayName, Entry.EvidenceId,
            Entry.SourceReference, Entry.SourceTimeLabel,
            Entry.SourceLatitude, Entry.SourceLongitude,
            Entry.bLocationApproximate, ResolvedFindingMesh,
            ResolvedFindingScale, Entry.FindingColor);
        Finding->FinishSpawning(SpawnTransform);
        Spawned = Finding;
    }
    else if (Entry.PropKind == ETMOPTimedPropKind::PickupItem)
    {
        if (!IsValid(Entry.ItemDefinition)) return false;
        TSubclassOf<ATMOPWorldItem> Class = Entry.WorldItemClass;
        if (!Class)
            Class = ATMOPWorldItem::StaticClass();
        ATMOPWorldItem* Item =
            GetWorld()->SpawnActorDeferred<ATMOPWorldItem>(
                Class, SpawnTransform, this, nullptr,
                ESpawnActorCollisionHandlingMethod::
                    AdjustIfPossibleButAlwaysSpawn);
        if (!IsValid(Item)) return false;
        Item->ConfigureWorldItem(Entry.ItemDefinition, 1);
        Item->FinishSpawning(SpawnTransform);
        Spawned = Item;
    }

    if (!IsValid(Spawned)) return false;
    SpawnedInstances.Add(Entry.InstanceId, Spawned);
    return true;
}

bool ATMOPTimedPropDirector::ResolveSpawnTransform(
    const FTMOPTimedPropEntry& Entry, FTransform& OutTransform) const
{
    FTransform Base = Entry.WorldTransform;
    if (Entry.Placement == ETMOPTimedPropPlacement::Anchor)
    {
        const UTMOPAnchorSubsystem* Anchors =
            GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>()
            : nullptr;
        ATMOPHistoricalAnchor* Anchor = IsValid(Anchors)
            ? Anchors->FindAnchor(Entry.AnchorId) : nullptr;
        if (!IsValid(Anchor)) return false;
        Base = Anchor->GetActorTransform();
    }
    OutTransform = Entry.LocalOffset * Base;
    if (Entry.bSnapToGround && GetWorld() != nullptr)
    {
        FVector Location = OutTransform.GetLocation();
        FHitResult GroundHit;
        FCollisionQueryParams Params(
            SCENE_QUERY_STAT(TMOPTimedPropGround), false, this);
        const FVector Start = Location + FVector(0.0f, 0.0f, 10000.0f);
        const FVector End = Location - FVector(0.0f, 0.0f, 10000.0f);
        if (GetWorld()->LineTraceSingleByChannel(
            GroundHit, Start, End, ECC_Visibility, Params))
        {
            Location.Z = GroundHit.ImpactPoint.Z + Entry.GroundOffsetCm;
            OutTransform.SetLocation(Location);
        }
    }
    return true;
}

AActor* ATMOPTimedPropDirector::FindSpawnedInstance(
    const FName InstanceId) const
{
    const TWeakObjectPtr<AActor>* Found =
        SpawnedInstances.Find(InstanceId);
    return Found != nullptr ? Found->Get() : nullptr;
}

void ATMOPTimedPropDirector::DestroyAllSpawnedInstances()
{
    for (TPair<FName, TWeakObjectPtr<AActor>>& Pair : SpawnedInstances)
        if (AActor* Actor = Pair.Value.Get())
            Actor->Destroy();
    SpawnedInstances.Reset();
}
