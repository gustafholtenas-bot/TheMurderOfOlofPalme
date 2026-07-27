#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Groups/TMOPGroupTypes.h"
#include "TMOPGroupDirector.generated.h"

class ATMOPHistoricalAgent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPGroupChangedSignature, FName, GroupId, ETMOPGroupState, NewState);

/** Generic runtime groups: create, merge, converse, move and split. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGroupDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGroupDirector();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups")
    TArray<FTMOPGroupDefinition> InitialGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups")
    bool bCreateInitialGroupsOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social")
    bool bEnableSocialPresentation = true;

    /** Nearby waiting groups form a loose inward-facing circle. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social")
    bool bArrangeWaitingGroupsInCircle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social",
        meta=(ClampMin="100.0", Units="cm"))
    float MaximumSocialGroupDistanceCm = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social",
        meta=(ClampMin="50.0", Units="cm"))
    float MinimumConversationCircleRadiusCm = 95.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social",
        meta=(ClampMin="0.1"))
    float SocialFacingInterpolationSpeed = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social",
        meta=(ClampMin="1.0", Units="s"))
    float MinimumSpeakerSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups|Social",
        meta=(ClampMin="1.0", Units="s"))
    float MaximumSpeakerSeconds = 5.5f;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Groups|Events")
    FTMOPGroupChangedSignature OnGroupStateChanged;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool CreateGroup(const FTMOPGroupDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool DissolveGroup(FName GroupId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool AddMember(FName GroupId, FName EntityId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool RemoveMember(FName GroupId, FName EntityId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool SetGroupLeader(FName GroupId, FName NewLeaderEntityId);

    /** Creates NewGroup and dissolves every source group after all members are collected. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool MergeGroups(FName NewGroupId, const TArray<FName>& SourceGroupIds,
        FName NewLeaderEntityId, ETMOPGroupFormation Formation, float FormationSpacing);

    /** Dissolves SourceGroupId and creates every supplied child definition. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool SplitGroup(FName SourceGroupId, const TArray<FTMOPGroupDefinition>& NewGroups);

    /** Negative MaxSeconds means the conversation continues until EndConversation is called. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool StartConversation(FName GroupId, float MinSeconds, float MaxSeconds, int32 Seed);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool EndConversation(FName GroupId);

    /** Used by dialogue/gameplay to make an NPC face a player or another actor. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Groups|Social")
    bool FocusAgentOnActor(FName AgentEntityId, AActor* Target,
        float DurationSeconds = 4.0f, bool bTalking = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool MoveGroupToLocation(FName GroupId, FVector TargetLocation,
        float AcceptanceRadius = 100.0f);

    /** Moves the group through every supplied location in order. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool MoveGroupThroughLocations(FName GroupId,
        const TArray<FVector>& RouteLocations,
        float AcceptanceRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool StopGroup(FName GroupId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    void ResetAllGroups();

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    int32 RecreateInitialGroups();

    /** Adds agents that spawned after a group was created. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    int32 RefreshWaitingGroups();

    UFUNCTION(BlueprintPure, Category="TMOP|Groups")
    bool DoesGroupExist(FName GroupId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Groups")
    FTMOPGroupSnapshot GetGroupSnapshot(FName GroupId, bool& bFound) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Groups")
    TArray<FTMOPGroupSnapshot> GetAllGroupSnapshots() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Groups")
    bool AreAllMembersWithinRadius(FName GroupId, FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool ValidateDefinitions(const TArray<FTMOPGroupDefinition>& Definitions,
        TArray<FString>& OutErrors) const;

private:
    struct FRuntimeGroup
    {
        FTMOPGroupDefinition Definition;
        TArray<TWeakObjectPtr<ATMOPHistoricalAgent>> Members;
        ETMOPGroupState State = ETMOPGroupState::Idle;
        FVector TargetLocation = FVector::ZeroVector;
        TArray<FVector> RouteLocations;
        int32 RouteLocationIndex = INDEX_NONE;
        float AcceptanceRadius = 100.0f;
        float RemainingConversationSeconds = 0.0f;
        bool bConversationHasNoAutomaticEnd = false;
        float SocialElapsedSeconds = 0.0f;
        float NextSpeakerChangeSeconds = 0.0f;
        int32 ActiveSpeakerIndex = INDEX_NONE;
        bool bWaitingCircleInitialized = false;
    };

    ATMOPHistoricalAgent* FindAgent(FName EntityId) const;
    ATMOPHistoricalAgent* FindLeader(const FRuntimeGroup& Group) const;
    FRuntimeGroup* FindGroup(FName GroupId);
    const FRuntimeGroup* FindGroup(FName GroupId) const;
    void SetState(FRuntimeGroup& Group, ETMOPGroupState NewState);
    void UpdateConversation(FRuntimeGroup& Group, float DeltaSeconds);
    void UpdateMovement(FRuntimeGroup& Group);
    void UpdateSocialPresentation(FRuntimeGroup& Group, float DeltaSeconds);
    void UpdateSocialSpeaker(FRuntimeGroup& Group);
    bool IsGroupCloseEnoughForSocialPresentation(const FRuntimeGroup& Group) const;
    bool IsGroupWaiting(const FRuntimeGroup& Group) const;
    FVector GetGroupCenter(const FRuntimeGroup& Group) const;
    void ArrangeWaitingCircle(FRuntimeGroup& Group, const FVector& Center);
    void RefreshMembers(FRuntimeGroup& Group);
    void RefreshCompanionLists(FRuntimeGroup& Group);
    FVector GetFormationOffset(const FRuntimeGroup& Group, int32 MemberIndex) const;
    FTMOPGroupSnapshot MakeSnapshot(const FRuntimeGroup& Group) const;

    TArray<FRuntimeGroup> RuntimeGroups;
};
