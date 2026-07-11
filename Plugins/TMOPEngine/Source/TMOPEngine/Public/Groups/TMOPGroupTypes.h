#pragma once

#include "CoreMinimal.h"
#include "TMOPGroupTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPGroupState : uint8
{
    Idle,
    WaitingForMembers,
    Conversing,
    Moving,
    Arrived,
    Dissolved
};

UENUM(BlueprintType)
enum class ETMOPGroupFormation : uint8
{
    FollowLeader,
    SideBySide,
    CompactCluster
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGroupDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups")
    FName GroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups")
    TArray<FName> MemberEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups")
    FName LeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups")
    ETMOPGroupFormation Formation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Groups",
        meta=(ClampMin="30.0"))
    float FormationSpacing = 110.0f;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGroupSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    FName GroupId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    TArray<FName> MemberEntityIds;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    FName LeaderEntityId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    ETMOPGroupState State = ETMOPGroupState::Idle;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    ETMOPGroupFormation Formation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    float RemainingConversationSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Groups")
    bool bConversationHasNoAutomaticEnd = false;
};
