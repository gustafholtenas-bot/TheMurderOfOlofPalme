#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "GameFramework/Actor.h"
#include "TMOPNavigationTestDirector.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPNavigationTestResultSignature,
    bool,
    bStartedSuccessfully);

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPNavigationTestDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPNavigationTestDirector();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    FName AgentId = TEXT("AGENT_TEST_001");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    FName StartAnchorId = TEXT("ANCHOR_TEST_START");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    FName EndAnchorId = TEXT("ANCHOR_TEST_END");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    bool bStartAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test",
        meta = (ClampMin = "0.0"))
    float StartDelaySeconds = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    bool bTeleportAgentToStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    float StartHeightOffset = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    ETMOPMovementPolicy MovementPolicy =
        ETMOPMovementPolicy::NormalPedestrian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Test")
    ETMOPAgentActivityState MovementActivity =
        ETMOPAgentActivityState::Walking;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Test")
    FTMOPNavigationTestResultSignature OnNavigationTestStarted;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Test")
    bool StartNavigationTest();

private:
    float RemainingDelaySeconds = 0.0f;
    bool bAutomaticStartPending = false;
};
