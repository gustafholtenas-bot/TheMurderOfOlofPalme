#include "Groups/TMOPGroupDirector.h"

#include "AIController.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Time/TMOPClockSubsystem.h"

ATMOPGroupDirector::ATMOPGroupDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPGroupDirector::BeginPlay()
{
    Super::BeginPlay();
    if (bCreateInitialGroupsOnBeginPlay)
        for (const FTMOPGroupDefinition& Definition : InitialGroups) CreateGroup(Definition);
}

void ATMOPGroupDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    for (FRuntimeGroup& Group : RuntimeGroups)
    {
        if (Group.State == ETMOPGroupState::WaitingForMembers) RefreshMembers(Group);
        if (Group.State == ETMOPGroupState::Conversing) UpdateConversation(Group, DeltaSeconds);
        else if (Group.State == ETMOPGroupState::Moving) UpdateMovement(Group);
        if (bEnableSocialPresentation) UpdateSocialPresentation(Group, DeltaSeconds);
    }
}

void ATMOPGroupDirector::RefreshMembers(FRuntimeGroup& Group)
{
    for (const FName EntityId : Group.Definition.MemberEntityIds)
    {
        bool bAlreadyPresent = false;
        for (const TWeakObjectPtr<ATMOPHistoricalAgent>& Existing : Group.Members)
            if (const ATMOPHistoricalAgent* Agent = Existing.Get())
                if (Agent->EntityIdentity != nullptr &&
                    Agent->EntityIdentity->EntityId == EntityId)
                { bAlreadyPresent = true; break; }
        if (bAlreadyPresent) continue;

        if (ATMOPHistoricalAgent* Agent = FindAgent(EntityId))
        {
            Agent->SocialGroupId = Group.Definition.GroupId;
            Agent->KnownCompanionIds = Group.Definition.MemberEntityIds;
            Agent->KnownCompanionIds.Remove(EntityId);
            Group.Members.Add(Agent);
        }
    }
    Group.Members.RemoveAll([](const TWeakObjectPtr<ATMOPHistoricalAgent>& Agent)
        { return !Agent.IsValid(); });
    if (Group.Members.Num() == Group.Definition.MemberEntityIds.Num() &&
        Group.State == ETMOPGroupState::WaitingForMembers)
        SetState(Group, ETMOPGroupState::Idle);
}

void ATMOPGroupDirector::RefreshCompanionLists(FRuntimeGroup& Group)
{
    for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
        if (ATMOPHistoricalAgent* Agent = Member.Get())
        {
            Agent->SocialGroupId = Group.Definition.GroupId;
            Agent->KnownCompanionIds = Group.Definition.MemberEntityIds;
            if (Agent->EntityIdentity != nullptr)
                Agent->KnownCompanionIds.Remove(
                    Agent->EntityIdentity->EntityId);
        }
}

int32 ATMOPGroupDirector::RefreshWaitingGroups()
{
    int32 Ready = 0;
    for (FRuntimeGroup& Group : RuntimeGroups)
    {
        RefreshMembers(Group);
        Ready += Group.Members.Num() == Group.Definition.MemberEntityIds.Num() ? 1 : 0;
    }
    return Ready;
}

bool ATMOPGroupDirector::CreateGroup(const FTMOPGroupDefinition& Definition)
{
    if (Definition.GroupId.IsNone() || DoesGroupExist(Definition.GroupId) ||
        Definition.MemberEntityIds.IsEmpty()) return false;
    FRuntimeGroup Group;
    Group.Definition = Definition;
    for (const FName EntityId : Definition.MemberEntityIds)
    {
        ATMOPHistoricalAgent* Agent = FindAgent(EntityId);
        if (!IsValid(Agent))
        {
            // This is expected for agents whose spawn time is later than the
            // group's creation. RefreshMembers attaches them after spawning.
            UE_LOG(LogTemp, Display, TEXT("TMOP group '%s' is waiting for later-spawning agent '%s'."),
                *Definition.GroupId.ToString(), *EntityId.ToString());
            continue;
        }
        Agent->SocialGroupId = Definition.GroupId;
        Agent->KnownCompanionIds = Definition.MemberEntityIds;
        Agent->KnownCompanionIds.Remove(EntityId);
        Group.Members.Add(Agent);
    }
    Group.State = Group.Members.Num() == Definition.MemberEntityIds.Num()
        ? ETMOPGroupState::Idle : ETMOPGroupState::WaitingForMembers;
    RuntimeGroups.Add(MoveTemp(Group));
    OnGroupStateChanged.Broadcast(Definition.GroupId, RuntimeGroups.Last().State);
    return true;
}

bool ATMOPGroupDirector::DissolveGroup(const FName GroupId)
{
    for (int32 Index = 0; Index < RuntimeGroups.Num(); ++Index)
    {
        if (RuntimeGroups[Index].Definition.GroupId != GroupId) continue;
        for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : RuntimeGroups[Index].Members)
        {
            if (ATMOPHistoricalAgent* Agent = Member.Get())
            {
                if (AAIController* Controller = Cast<AAIController>(Agent->GetController())) Controller->StopMovement();
                Agent->SocialGroupId = NAME_None;
                Agent->KnownCompanionIds.Reset();
            }
        }
        OnGroupStateChanged.Broadcast(GroupId, ETMOPGroupState::Dissolved);
        RuntimeGroups.RemoveAt(Index);
        return true;
    }
    return false;
}

bool ATMOPGroupDirector::AddMember(const FName GroupId,
    const FName EntityId)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr || EntityId.IsNone() ||
        Group->Definition.MemberEntityIds.Contains(EntityId))
        return false;
    ATMOPHistoricalAgent* Agent = FindAgent(EntityId);
    if (!IsValid(Agent) ||
        (!Agent->SocialGroupId.IsNone() &&
         Agent->SocialGroupId != GroupId))
        return false;
    Group->Definition.MemberEntityIds.Add(EntityId);
    Group->Members.AddUnique(Agent);
    RefreshCompanionLists(*Group);
    if (Group->Members.Num() == Group->Definition.MemberEntityIds.Num() &&
        Group->State == ETMOPGroupState::WaitingForMembers)
        SetState(*Group, ETMOPGroupState::Idle);
    else
        OnGroupStateChanged.Broadcast(GroupId, Group->State);
    return true;
}

bool ATMOPGroupDirector::RemoveMember(const FName GroupId,
    const FName EntityId)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr ||
        !Group->Definition.MemberEntityIds.Contains(EntityId))
        return false;

    if (ATMOPHistoricalAgent* Agent = FindAgent(EntityId))
    {
        if (AAIController* Controller =
            Cast<AAIController>(Agent->GetController()))
            Controller->StopMovement();
        Agent->SocialGroupId = NAME_None;
        Agent->KnownCompanionIds.Reset();
    }
    Group->Definition.MemberEntityIds.Remove(EntityId);
    Group->Members.RemoveAll(
        [EntityId](const TWeakObjectPtr<ATMOPHistoricalAgent>& Member)
        {
            const ATMOPHistoricalAgent* Agent = Member.Get();
            return !IsValid(Agent) ||
                (Agent->EntityIdentity != nullptr &&
                 Agent->EntityIdentity->EntityId == EntityId);
        });

    if (Group->Definition.MemberEntityIds.IsEmpty())
        return DissolveGroup(GroupId);
    if (Group->Definition.LeaderEntityId == EntityId)
        Group->Definition.LeaderEntityId =
            Group->Definition.MemberEntityIds[0];
    RefreshCompanionLists(*Group);
    OnGroupStateChanged.Broadcast(GroupId, Group->State);
    return true;
}

bool ATMOPGroupDirector::SetGroupLeader(const FName GroupId,
    const FName NewLeaderEntityId)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr ||
        !Group->Definition.MemberEntityIds.Contains(NewLeaderEntityId))
        return false;
    Group->Definition.LeaderEntityId = NewLeaderEntityId;
    OnGroupStateChanged.Broadcast(GroupId, Group->State);
    return true;
}

bool ATMOPGroupDirector::MergeGroups(const FName NewGroupId,
    const TArray<FName>& SourceGroupIds, const FName NewLeaderEntityId,
    const ETMOPGroupFormation Formation, const float FormationSpacing)
{
    if (NewGroupId.IsNone() || DoesGroupExist(NewGroupId) || SourceGroupIds.Num() < 2) return false;
    FTMOPGroupDefinition Merged;
    Merged.GroupId = NewGroupId;
    Merged.LeaderEntityId = NewLeaderEntityId;
    Merged.Formation = Formation;
    Merged.FormationSpacing = FMath::Max(30.0f, FormationSpacing);
    for (const FName SourceId : SourceGroupIds)
    {
        const FRuntimeGroup* Source = FindGroup(SourceId);
        if (Source == nullptr) return false;
        for (const FName MemberId : Source->Definition.MemberEntityIds) Merged.MemberEntityIds.AddUnique(MemberId);
    }
    for (const FName SourceId : SourceGroupIds) DissolveGroup(SourceId);
    return CreateGroup(Merged);
}

bool ATMOPGroupDirector::SplitGroup(const FName SourceGroupId,
    const TArray<FTMOPGroupDefinition>& NewGroups)
{
    const FRuntimeGroup* Source = FindGroup(SourceGroupId);
    if (Source == nullptr || NewGroups.Num() < 2) return false;
    TSet<FName> SourceMembers;
    for (const FName MemberId : Source->Definition.MemberEntityIds) SourceMembers.Add(MemberId);
    TSet<FName> Assigned;
    for (const FTMOPGroupDefinition& Definition : NewGroups)
        for (const FName MemberId : Definition.MemberEntityIds)
        {
            if (!SourceMembers.Contains(MemberId) || Assigned.Contains(MemberId)) return false;
            Assigned.Add(MemberId);
        }
    if (Assigned.Num() != SourceMembers.Num()) return false;
    DissolveGroup(SourceGroupId);
    bool bSuccess = true;
    for (const FTMOPGroupDefinition& Definition : NewGroups) bSuccess &= CreateGroup(Definition);
    return bSuccess;
}

bool ATMOPGroupDirector::StartConversation(const FName GroupId,
    const float MinSeconds, const float MaxSeconds, const int32 Seed)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr) return false;
    StopGroup(GroupId);
    Group->bConversationHasNoAutomaticEnd = MaxSeconds < 0.0f;
    if (Group->bConversationHasNoAutomaticEnd) Group->RemainingConversationSeconds = -1.0f;
    else
    {
        FRandomStream Random(Seed);
        const float MinValue = FMath::Max(0.0f, MinSeconds);
        Group->RemainingConversationSeconds = Random.FRandRange(MinValue, FMath::Max(MinValue, MaxSeconds));
    }
    for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group->Members)
        if (ATMOPHistoricalAgent* Agent = Member.Get()) Agent->SetActivityState(ETMOPAgentActivityState::Interacting);
    SetState(*Group, ETMOPGroupState::Conversing);
    return true;
}

bool ATMOPGroupDirector::EndConversation(const FName GroupId)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr || Group->State != ETMOPGroupState::Conversing) return false;
    for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group->Members)
        if (ATMOPHistoricalAgent* Agent = Member.Get()) Agent->SetActivityState(ETMOPAgentActivityState::Standing);
    Group->RemainingConversationSeconds = 0.0f;
    Group->bConversationHasNoAutomaticEnd = false;
    SetState(*Group, ETMOPGroupState::Idle);
    return true;
}

bool ATMOPGroupDirector::FocusAgentOnActor(const FName AgentEntityId,
    AActor* Target, const float DurationSeconds, const bool bTalking)
{
    ATMOPHistoricalAgent* Agent = FindAgent(AgentEntityId);
    if (!IsValid(Agent) || !IsValid(Target)) return false;
    Agent->SetSocialFocus(Target, DurationSeconds, bTalking);
    if (!Agent->IsDialogueFocused() && Agent->GetVelocity().Size2D() < 10.0f)
    {
        const FVector Delta = Target->GetActorLocation() - Agent->GetActorLocation();
        if (!Delta.IsNearlyZero())
            Agent->SetActorRotation(FRotator(0.0f, Delta.Rotation().Yaw, 0.0f));
    }
    return true;
}

bool ATMOPGroupDirector::MoveGroupToLocation(const FName GroupId,
    const FVector TargetLocation, const float AcceptanceRadius)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    ATMOPHistoricalAgent* Leader = Group != nullptr ? FindLeader(*Group) : nullptr;
    if (Group == nullptr || !IsValid(Leader)) return false;
    Group->RouteLocations.Reset();
    Group->RouteLocationIndex = INDEX_NONE;
    Group->TargetLocation = TargetLocation;
    Group->AcceptanceRadius = FMath::Max(20.0f, AcceptanceRadius);
    Group->ExpectedArrivalSecond = INDEX_NONE;
    SetState(*Group, ETMOPGroupState::Moving);
    UpdateMovement(*Group);
    return true;
}

bool ATMOPGroupDirector::MoveGroupThroughLocations(const FName GroupId,
    const TArray<FVector>& RouteLocations, const float AcceptanceRadius)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    ATMOPHistoricalAgent* Leader = Group != nullptr ? FindLeader(*Group) : nullptr;
    if (Group == nullptr || !IsValid(Leader) || RouteLocations.IsEmpty()) return false;
    Group->RouteLocations = RouteLocations;
    Group->RouteLocationIndex = 0;
    Group->TargetLocation = Group->RouteLocations[0];
    Group->AcceptanceRadius = FMath::Max(20.0f, AcceptanceRadius);
    Group->ExpectedArrivalSecond = INDEX_NONE;
    SetState(*Group, ETMOPGroupState::Moving);
    UpdateMovement(*Group);
    return true;
}

bool ATMOPGroupDirector::MoveGroupThroughLocationsTimed(const FName GroupId,
    const TArray<FVector>& RouteLocations, const float AcceptanceRadius,
    const int32 ExpectedArrivalSecond,
    const float MinimumSpeedCmPerSecond,
    const float MaximumSpeedCmPerSecond)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    ATMOPHistoricalAgent* Leader = Group != nullptr
        ? FindLeader(*Group) : nullptr;
    if (Group == nullptr || !IsValid(Leader) || RouteLocations.IsEmpty())
        return false;
    Group->RouteLocations = RouteLocations;
    Group->RouteLocationIndex = 0;
    Group->TargetLocation = Group->RouteLocations[0];
    Group->AcceptanceRadius = FMath::Max(20.0f, AcceptanceRadius);
    Group->ExpectedArrivalSecond = ExpectedArrivalSecond;
    Group->TimedMinimumSpeedCmPerSecond =
        FMath::Max(1.0f, MinimumSpeedCmPerSecond);
    Group->TimedMaximumSpeedCmPerSecond = FMath::Max(
        Group->TimedMinimumSpeedCmPerSecond,
        MaximumSpeedCmPerSecond);
    SetState(*Group, ETMOPGroupState::Moving);
    UpdateMovement(*Group);
    return true;
}

bool ATMOPGroupDirector::StopGroup(const FName GroupId)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr) return false;
    Group->ExpectedArrivalSecond = INDEX_NONE;
    for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group->Members)
        if (ATMOPHistoricalAgent* Agent = Member.Get())
        {
            if (AAIController* Controller = Cast<AAIController>(Agent->GetController())) Controller->StopMovement();
            Agent->SetActivityState(ETMOPAgentActivityState::Standing);
        }
    Group->RouteLocations.Reset();
    Group->RouteLocationIndex = INDEX_NONE;
    if (Group->State == ETMOPGroupState::Moving) SetState(*Group, ETMOPGroupState::Idle);
    return true;
}

void ATMOPGroupDirector::ResetAllGroups()
{
    while (!RuntimeGroups.IsEmpty())
        DissolveGroup(RuntimeGroups.Last().Definition.GroupId);
}

int32 ATMOPGroupDirector::RecreateInitialGroups()
{
    ResetAllGroups();
    int32 Created = 0;
    for (const FTMOPGroupDefinition& Definition : InitialGroups)
        Created += CreateGroup(Definition) ? 1 : 0;
    return Created;
}

void ATMOPGroupDirector::UpdateConversation(FRuntimeGroup& Group, const float DeltaSeconds)
{
    if (Group.bConversationHasNoAutomaticEnd) return;
    Group.RemainingConversationSeconds -= DeltaSeconds;
    if (Group.RemainingConversationSeconds <= 0.0f) EndConversation(Group.Definition.GroupId);
}

void ATMOPGroupDirector::UpdateSocialPresentation(
    FRuntimeGroup& Group, const float DeltaSeconds)
{
    Group.Members.RemoveAll([](const TWeakObjectPtr<ATMOPHistoricalAgent>& Member)
        { return !Member.IsValid(); });
    if (Group.Members.Num() < 2 ||
        !IsGroupCloseEnoughForSocialPresentation(Group)) return;

    Group.SocialElapsedSeconds += DeltaSeconds;
    if (Group.NextSpeakerChangeSeconds <= Group.SocialElapsedSeconds)
        UpdateSocialSpeaker(Group);

    const bool bWaiting = IsGroupWaiting(Group);
    const FVector Center = GetGroupCenter(Group);
    if (bWaiting && bArrangeWaitingGroupsInCircle)
        ArrangeWaitingCircle(Group, Center);
    else
        Group.bWaitingCircleInitialized = false;

    for (int32 Index = 0; Index < Group.Members.Num(); ++Index)
    {
        ATMOPHistoricalAgent* Agent = Group.Members[Index].Get();
        if (!IsValid(Agent)) continue;
        if (Agent->IsDialogueFocused()) continue;
        ATMOPHistoricalAgent* Partner =
            Group.Members[(Index + 1 + FMath::Max(0, Group.ActiveSpeakerIndex))
                % Group.Members.Num()].Get();
        if (!IsValid(Partner) || Partner == Agent)
            Partner = Group.Members[(Index + 1) % Group.Members.Num()].Get();
        if (IsValid(Partner))
            Agent->SetSocialFocus(Partner,
                FMath::Max(0.5f, Group.NextSpeakerChangeSeconds -
                    Group.SocialElapsedSeconds + 0.25f),
                Index == Group.ActiveSpeakerIndex);

        if (bWaiting)
        {
            FVector TowardCenter = Center - Agent->GetActorLocation();
            TowardCenter.Z = 0.0f;
            if (!TowardCenter.IsNearlyZero())
            {
                const FRotator Desired(0.0f, TowardCenter.Rotation().Yaw, 0.0f);
                Agent->SetActorRotation(FMath::RInterpTo(
                    Agent->GetActorRotation(), Desired, DeltaSeconds,
                    SocialFacingInterpolationSpeed));
            }
        }
    }
}

void ATMOPGroupDirector::UpdateSocialSpeaker(FRuntimeGroup& Group)
{
    if (Group.Members.IsEmpty()) return;
    const int32 PreviousSpeaker = Group.ActiveSpeakerIndex;
    const uint32 StableHash =
        GetTypeHash(Group.Definition.GroupId) +
        static_cast<uint32>(Group.SocialElapsedSeconds * 10.0f) * 2654435761u;
    FRandomStream Random(static_cast<int32>(StableHash));
    Group.ActiveSpeakerIndex = Random.RandRange(0, Group.Members.Num() - 1);
    if (Group.Members.Num() > 1 &&
        Group.ActiveSpeakerIndex == PreviousSpeaker)
        Group.ActiveSpeakerIndex =
            (Group.ActiveSpeakerIndex + 1) % Group.Members.Num();
    const float Duration = Random.FRandRange(
        FMath::Max(1.0f, MinimumSpeakerSeconds),
        FMath::Max(MinimumSpeakerSeconds, MaximumSpeakerSeconds));
    Group.NextSpeakerChangeSeconds = Group.SocialElapsedSeconds + Duration;

    for (int32 Index = 0; Index < Group.Members.Num(); ++Index)
        if (ATMOPHistoricalAgent* Agent = Group.Members[Index].Get())
        {
            if (Agent->IsDialogueFocused()) continue;
            if (UTMOPAnimationStateComponent* Animation =
                Agent->FindComponentByClass<UTMOPAnimationStateComponent>())
            {
                if (Index == Group.ActiveSpeakerIndex)
                {
                    if (Animation->Overlay == ETMOPAnimOverlay::None ||
                        Animation->Overlay == ETMOPAnimOverlay::Talking)
                        Animation->SetOverlay(ETMOPAnimOverlay::Talking);
                }
                else if (Animation->Overlay == ETMOPAnimOverlay::Talking)
                    Animation->SetOverlay(ETMOPAnimOverlay::None);
            }
        }
}

bool ATMOPGroupDirector::IsGroupCloseEnoughForSocialPresentation(
    const FRuntimeGroup& Group) const
{
    const FVector Center = GetGroupCenter(Group);
    const float MaximumSquared = FMath::Square(MaximumSocialGroupDistanceCm);
    for (const TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
        if (const ATMOPHistoricalAgent* Agent = Member.Get())
            if (FVector::DistSquared2D(Agent->GetActorLocation(), Center) >
                MaximumSquared) return false;
    return true;
}

bool ATMOPGroupDirector::IsGroupWaiting(const FRuntimeGroup& Group) const
{
    if (Group.State == ETMOPGroupState::Moving ||
        Group.State == ETMOPGroupState::WaitingForMembers ||
        Group.State == ETMOPGroupState::Dissolved) return false;
    if (Group.bWaitingCircleInitialized) return true;
    for (const TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
    {
        const ATMOPHistoricalAgent* Agent = Member.Get();
        if (!IsValid(Agent) || Agent->GetVelocity().Size2D() > 20.0f)
            return false;
        if (Agent->ActivityState != ETMOPAgentActivityState::Standing &&
            Agent->ActivityState != ETMOPAgentActivityState::Interacting &&
            Agent->ActivityState != ETMOPAgentActivityState::Idle)
            return false;
    }
    return true;
}

FVector ATMOPGroupDirector::GetGroupCenter(const FRuntimeGroup& Group) const
{
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    for (const TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
        if (const ATMOPHistoricalAgent* Agent = Member.Get())
        {
            Center += Agent->GetActorLocation();
            ++Count;
        }
    return Count > 0 ? Center / static_cast<float>(Count) : FVector::ZeroVector;
}

void ATMOPGroupDirector::ArrangeWaitingCircle(
    FRuntimeGroup& Group, const FVector& Center)
{
    const int32 MemberCount = Group.Members.Num();
    if (MemberCount < 2) return;
    const float Radius = FMath::Max(
        MinimumConversationCircleRadiusCm,
        Group.Definition.FormationSpacing * MemberCount / (2.0f * PI));
    bool bAllPlaced = true;
    for (int32 Index = 0; Index < MemberCount; ++Index)
    {
        ATMOPHistoricalAgent* Agent = Group.Members[Index].Get();
        AAIController* Controller =
            IsValid(Agent) ? Cast<AAIController>(Agent->GetController()) : nullptr;
        if (!IsValid(Agent) || Controller == nullptr) continue;
        if (Agent->IsDialogueFocused())
        {
            Controller->StopMovement();
            continue;
        }
        const float Angle = 2.0f * PI * Index / MemberCount;
        const FVector Desired = Center +
            FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius;
        if (FVector::DistSquared2D(Agent->GetActorLocation(), Desired) >
            FMath::Square(35.0f))
        {
            bAllPlaced = false;
            Agent->SetActivityState(ETMOPAgentActivityState::Walking);
            Controller->MoveToLocation(Desired, 28.0f, true, true, false, true);
        }
        else if (Agent->ActivityState == ETMOPAgentActivityState::Walking)
        {
            Controller->StopMovement();
            Agent->SetActivityState(
                Group.State == ETMOPGroupState::Conversing
                ? ETMOPAgentActivityState::Interacting
                : ETMOPAgentActivityState::Standing);
        }
    }
    Group.bWaitingCircleInitialized = !bAllPlaced;
}

void ATMOPGroupDirector::UpdateMovement(FRuntimeGroup& Group)
{
    ATMOPHistoricalAgent* Leader = FindLeader(Group);
    if (!IsValid(Leader)) { SetState(Group, ETMOPGroupState::WaitingForMembers); return; }

    const UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const double Now = Clock != nullptr
        ? Clock->GetCurrentTimeSecondsExact() : 0.0;
    if (Group.ExpectedArrivalSecond != INDEX_NONE)
    {
        if (Now >= static_cast<double>(Group.ExpectedArrivalSecond))
        {
            const FVector FinalTarget = Group.RouteLocations.IsEmpty()
                ? Group.TargetLocation : Group.RouteLocations.Last();
            Leader->SetActorLocation(
                FinalTarget, false, nullptr, ETeleportType::TeleportPhysics);
            for (int32 Index = 0; Index < Group.Members.Num(); ++Index)
            {
                ATMOPHistoricalAgent* Agent = Group.Members[Index].Get();
                if (!IsValid(Agent) || Agent == Leader) continue;
                Agent->SetActorLocation(
                    Leader->GetActorTransform().TransformPosition(
                        GetFormationOffset(Group, Index)),
                    false, nullptr, ETeleportType::TeleportPhysics);
            }
            Group.ExpectedArrivalSecond = INDEX_NONE;
            StopGroup(Group.Definition.GroupId);
            SetState(Group, ETMOPGroupState::Arrived);
            return;
        }

        double RemainingPathCm = FVector::Dist2D(
            Leader->GetActorLocation(), Group.TargetLocation);
        for (int32 Index = Group.RouteLocationIndex + 1;
            Index < Group.RouteLocations.Num(); ++Index)
            RemainingPathCm += FVector::Dist2D(
                Group.RouteLocations[Index - 1], Group.RouteLocations[Index]);
        const double RemainingSeconds =
            static_cast<double>(Group.ExpectedArrivalSecond) - Now;
        const float SimulationRate = Clock != nullptr
            ? FMath::Max(0.0f, Clock->GetTimeScale()) : 1.0f;
        const float RequiredSpeed = RemainingSeconds > KINDA_SMALL_NUMBER
            ? static_cast<float>(RemainingPathCm / RemainingSeconds) *
                SimulationRate
            : Group.TimedMaximumSpeedCmPerSecond;
        const float ChosenSpeed = FMath::Clamp(RequiredSpeed,
            Group.TimedMinimumSpeedCmPerSecond * SimulationRate,
            Group.TimedMaximumSpeedCmPerSecond * SimulationRate);
        for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
            if (ATMOPHistoricalAgent* Agent = Member.Get())
                if (UCharacterMovementComponent* Movement =
                    Agent->GetCharacterMovement())
                    Movement->MaxWalkSpeed = ChosenSpeed;
    }
    if (FVector::DistSquared2D(Leader->GetActorLocation(), Group.TargetLocation) <= FMath::Square(Group.AcceptanceRadius))
    {
        // Reaching the leader target is not the same as the group arriving.
        // Previously this immediately stopped every member, which could strand
        // Ingrid (or any other follower) far behind a fast leader.
        bool bAllMembersInFormation = true;
        if (AAIController* LeaderController =
            Cast<AAIController>(Leader->GetController()))
            LeaderController->StopMovement();
        Leader->SetActivityState(ETMOPAgentActivityState::Standing);

        const float MemberTolerance = FMath::Max(
            30.0f, MemberArrivalToleranceCm);
        for (int32 Index = 0; Index < Group.Members.Num(); ++Index)
        {
            ATMOPHistoricalAgent* Agent = Group.Members[Index].Get();
            if (!IsValid(Agent) || Agent == Leader) continue;
            const FVector FormationTarget =
                Leader->GetActorTransform().TransformPosition(
                    GetFormationOffset(Group, Index));
            if (FVector::DistSquared2D(
                Agent->GetActorLocation(), FormationTarget) <=
                FMath::Square(MemberTolerance))
            {
                if (AAIController* Controller =
                    Cast<AAIController>(Agent->GetController()))
                    Controller->StopMovement();
                Agent->SetActivityState(ETMOPAgentActivityState::Standing);
                continue;
            }

            bAllMembersInFormation = false;
            AAIController* Controller =
                Cast<AAIController>(Agent->GetController());
            if (Controller != nullptr)
            {
                Agent->SetActivityState(ETMOPAgentActivityState::Walking);
                Controller->MoveToLocation(FormationTarget,
                    MemberTolerance, true, true, false, true);
            }
        }
        if (!bAllMembersInFormation) return;

        if (Group.RouteLocations.IsValidIndex(Group.RouteLocationIndex + 1))
        {
            ++Group.RouteLocationIndex;
            Group.TargetLocation = Group.RouteLocations[Group.RouteLocationIndex];
            UpdateMovement(Group);
            return;
        }
        if (Group.ExpectedArrivalSecond != INDEX_NONE &&
            Now < static_cast<double>(Group.ExpectedArrivalSecond))
        {
            for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
                if (ATMOPHistoricalAgent* Agent = Member.Get())
                    if (AAIController* Controller =
                        Cast<AAIController>(Agent->GetController()))
                        Controller->StopMovement();
            return;
        }
        Group.ExpectedArrivalSecond = INDEX_NONE;
        StopGroup(Group.Definition.GroupId);
        SetState(Group, ETMOPGroupState::Arrived);
        return;
    }
    for (int32 Index = 0; Index < Group.Members.Num(); ++Index)
    {
        ATMOPHistoricalAgent* Agent = Group.Members[Index].Get();
        AAIController* Controller = IsValid(Agent) ? Cast<AAIController>(Agent->GetController()) : nullptr;
        if (!IsValid(Agent) || Controller == nullptr) continue;
        FVector Target = Group.TargetLocation;
        if (Agent != Leader) Target = Leader->GetActorTransform().TransformPosition(GetFormationOffset(Group, Index));
        Agent->SetActivityState(ETMOPAgentActivityState::Walking);
        Controller->MoveToLocation(Target, Group.AcceptanceRadius, true, true, false, true);
    }
}

FVector ATMOPGroupDirector::GetFormationOffset(const FRuntimeGroup& Group, const int32 MemberIndex) const
{
    const float Spacing = FMath::Max(30.0f, Group.Definition.FormationSpacing);
    if (Group.Definition.Formation == ETMOPGroupFormation::FollowLeader)
        return FVector(-Spacing * MemberIndex, 0.0f, 0.0f);
    if (Group.Definition.Formation == ETMOPGroupFormation::SideBySide)
    {
        const float Center = (Group.Members.Num() - 1) * 0.5f;
        return FVector(0.0f, (MemberIndex - Center) * Spacing, 0.0f);
    }
    const float Angle = Group.Members.Num() > 0 ? 2.0f * PI * MemberIndex / Group.Members.Num() : 0.0f;
    return FVector(FMath::Cos(Angle) * Spacing, FMath::Sin(Angle) * Spacing, 0.0f);
}

ATMOPHistoricalAgent* ATMOPGroupDirector::FindAgent(const FName EntityId) const
{
    UWorld* World = GetWorld();
    if (World == nullptr || EntityId.IsNone()) return nullptr;
    for (TActorIterator<ATMOPHistoricalAgent> It(World); It; ++It)
        if (It->EntityIdentity != nullptr && It->EntityIdentity->EntityId == EntityId) return *It;
    return nullptr;
}

ATMOPHistoricalAgent* ATMOPGroupDirector::FindLeader(const FRuntimeGroup& Group) const
{
    if (!Group.Definition.LeaderEntityId.IsNone())
        for (const TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group.Members)
            if (ATMOPHistoricalAgent* Agent = Member.Get())
                if (Agent->EntityIdentity != nullptr &&
                    Agent->EntityIdentity->EntityId == Group.Definition.LeaderEntityId) return Agent;
    return Group.Members.IsEmpty() ? nullptr : Group.Members[0].Get();
}

ATMOPGroupDirector::FRuntimeGroup* ATMOPGroupDirector::FindGroup(const FName GroupId)
{
    for (FRuntimeGroup& Group : RuntimeGroups) if (Group.Definition.GroupId == GroupId) return &Group;
    return nullptr;
}
const ATMOPGroupDirector::FRuntimeGroup* ATMOPGroupDirector::FindGroup(const FName GroupId) const
{
    for (const FRuntimeGroup& Group : RuntimeGroups) if (Group.Definition.GroupId == GroupId) return &Group;
    return nullptr;
}

void ATMOPGroupDirector::SetState(FRuntimeGroup& Group, const ETMOPGroupState NewState)
{
    if (Group.State == NewState) return;
    Group.State = NewState;
    OnGroupStateChanged.Broadcast(Group.Definition.GroupId, NewState);
}

bool ATMOPGroupDirector::DoesGroupExist(const FName GroupId) const { return FindGroup(GroupId) != nullptr; }

FTMOPGroupSnapshot ATMOPGroupDirector::MakeSnapshot(const FRuntimeGroup& Group) const
{
    FTMOPGroupSnapshot Result;
    Result.GroupId = Group.Definition.GroupId;
    Result.MemberEntityIds = Group.Definition.MemberEntityIds;
    Result.LeaderEntityId = Group.Definition.LeaderEntityId;
    Result.State = Group.State;
    Result.Formation = Group.Definition.Formation;
    Result.RemainingConversationSeconds = Group.RemainingConversationSeconds;
    Result.bConversationHasNoAutomaticEnd = Group.bConversationHasNoAutomaticEnd;
    Result.TargetLocation = Group.TargetLocation;
    Result.AcceptanceRadius = Group.AcceptanceRadius;
    return Result;
}

FTMOPGroupSnapshot ATMOPGroupDirector::GetGroupSnapshot(const FName GroupId, bool& bFound) const
{
    const FRuntimeGroup* Group = FindGroup(GroupId);
    bFound = Group != nullptr;
    return Group != nullptr ? MakeSnapshot(*Group) : FTMOPGroupSnapshot();
}

TArray<FTMOPGroupSnapshot> ATMOPGroupDirector::GetAllGroupSnapshots() const
{
    TArray<FTMOPGroupSnapshot> Result;
    for (const FRuntimeGroup& Group : RuntimeGroups) Result.Add(MakeSnapshot(Group));
    return Result;
}

bool ATMOPGroupDirector::AreAllMembersWithinRadius(const FName GroupId,
    const FVector Location, const float Radius) const
{
    const FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr || Group->Members.IsEmpty()) return false;
    const float RadiusSquared = FMath::Square(FMath::Max(0.0f, Radius));
    for (const TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group->Members)
    {
        const ATMOPHistoricalAgent* Agent = Member.Get();
        if (!IsValid(Agent) || FVector::DistSquared2D(Agent->GetActorLocation(), Location) > RadiusSquared) return false;
    }
    return true;
}

bool ATMOPGroupDirector::ValidateDefinitions(const TArray<FTMOPGroupDefinition>& Definitions,
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TSet<FName> GroupIds;
    for (const FTMOPGroupDefinition& Definition : Definitions)
    {
        if (Definition.GroupId.IsNone()) OutErrors.Add(TEXT("A group has no GroupId."));
        else if (GroupIds.Contains(Definition.GroupId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate GroupId '%s'."), *Definition.GroupId.ToString()));
        GroupIds.Add(Definition.GroupId);
        if (Definition.MemberEntityIds.Num() < 2)
            OutErrors.Add(FString::Printf(TEXT("Group '%s' needs at least two members."), *Definition.GroupId.ToString()));
        TSet<FName> Members;
        for (const FName MemberId : Definition.MemberEntityIds)
        {
            if (MemberId.IsNone() || Members.Contains(MemberId))
                OutErrors.Add(FString::Printf(TEXT("Group '%s' has an invalid or duplicate member."), *Definition.GroupId.ToString()));
            Members.Add(MemberId);
        }
        if (!Definition.LeaderEntityId.IsNone() && !Members.Contains(Definition.LeaderEntityId))
            OutErrors.Add(FString::Printf(TEXT("Leader is not a member of group '%s'."), *Definition.GroupId.ToString()));
    }
    return OutErrors.IsEmpty();
}
