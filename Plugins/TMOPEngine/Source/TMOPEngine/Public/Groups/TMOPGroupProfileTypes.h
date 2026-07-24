#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "TMOPGroupProfileTypes.generated.h"

/**
 * Editable, source-backed definition of one scenario group.
 *
 * DT_TMOP_Groups is authoritative when assigned to the person registry
 * director. Person rows only need to keep SocialGroupId for display and
 * backwards compatibility; leader, membership and formation come from here.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGroupProfileRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    FName GroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    TArray<FName> MemberEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    FName LeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    ETMOPGroupFormation Formation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group",
        meta=(ClampMin="30.0"))
    float FormationSpacingCm = 110.0f;

    /** Followers ignore their own movement entries and follow the leader. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    bool bUseLeaderTimeline = true;

    /** Create this group after the scenario's people have spawned. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group")
    bool bCreateAtScenarioStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group|Source")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Reconstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Group|Source")
    FString Notes;
};
