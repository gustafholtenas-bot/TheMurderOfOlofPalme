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

    UPROPERTY(BlueprintAssignable, Category="TMOP|Groups|Events")
    FTMOPGroupChangedSignature OnGroupStateChanged;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool CreateGroup(const FTMOPGroupDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool DissolveGroup(FName GroupId);

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

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool MoveGroupToLocation(FName GroupId, FVector TargetLocation,
        float AcceptanceRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category="TMOP|Groups")
    bool StopGroup(FName GroupId);

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
        float AcceptanceRadius = 100.0f;
        float RemainingConversationSeconds = 0.0f;
        bool bConversationHasNoAutomaticEnd = false;
    };

    ATMOPHistoricalAgent* FindAgent(FName EntityId) const;
    ATMOPHistoricalAgent* FindLeader(const FRuntimeGroup& Group) const;
    FRuntimeGroup* FindGroup(FName GroupId);
    const FRuntimeGroup* FindGroup(FName GroupId) const;
    void SetState(FRuntimeGroup& Group, ETMOPGroupState NewState);
    void UpdateConversation(FRuntimeGroup& Group, float DeltaSeconds);
    void UpdateMovement(FRuntimeGroup& Group);
    FVector GetFormationOffset(const FRuntimeGroup& Group, int32 MemberIndex) const;
    FTMOPGroupSnapshot MakeSnapshot(const FRuntimeGroup& Group) const;

    TArray<FRuntimeGroup> RuntimeGroups;
};
