#include "Groups/TMOPGroupDirector.h"

#include "AIController.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"

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
            UE_LOG(LogTemp, Warning, TEXT("TMOP group '%s' is waiting for agent '%s'."),
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

bool ATMOPGroupDirector::MoveGroupToLocation(const FName GroupId,
    const FVector TargetLocation, const float AcceptanceRadius)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    ATMOPHistoricalAgent* Leader = Group != nullptr ? FindLeader(*Group) : nullptr;
    if (Group == nullptr || !IsValid(Leader)) return false;
    Group->TargetLocation = TargetLocation;
    Group->AcceptanceRadius = FMath::Max(20.0f, AcceptanceRadius);
    SetState(*Group, ETMOPGroupState::Moving);
    UpdateMovement(*Group);
    return true;
}

bool ATMOPGroupDirector::StopGroup(const FName GroupId)
{
    FRuntimeGroup* Group = FindGroup(GroupId);
    if (Group == nullptr) return false;
    for (TWeakObjectPtr<ATMOPHistoricalAgent>& Member : Group->Members)
        if (ATMOPHistoricalAgent* Agent = Member.Get())
        {
            if (AAIController* Controller = Cast<AAIController>(Agent->GetController())) Controller->StopMovement();
            Agent->SetActivityState(ETMOPAgentActivityState::Standing);
        }
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

void ATMOPGroupDirector::UpdateMovement(FRuntimeGroup& Group)
{
    ATMOPHistoricalAgent* Leader = FindLeader(Group);
    if (!IsValid(Leader)) { SetState(Group, ETMOPGroupState::WaitingForMembers); return; }
    if (FVector::DistSquared2D(Leader->GetActorLocation(), Group.TargetLocation) <= FMath::Square(Group.AcceptanceRadius))
    {
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
