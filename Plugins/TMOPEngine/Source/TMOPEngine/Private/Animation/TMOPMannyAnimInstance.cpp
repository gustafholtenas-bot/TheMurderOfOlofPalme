#include "Animation/TMOPMannyAnimInstance.h"

#include "Agents/TMOPHistoricalAgent.h"

void UTMOPMannyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    HistoricalAgent =
        Cast<ATMOPHistoricalAgent>(TryGetPawnOwner());
}

void UTMOPMannyAnimInstance::NativeUpdateAnimation(
    const float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!IsValid(HistoricalAgent))
    {
        HistoricalAgent =
            Cast<ATMOPHistoricalAgent>(TryGetPawnOwner());
    }

    if (!IsValid(HistoricalAgent))
    {
        Speed = 0.0f;
        bIsMoving = false;
        bIsSeated = false;
        bIsStandingStill = true;
        ActivityState = ETMOPAgentActivityState::Idle;
        return;
    }

    Speed = HistoricalAgent->GetVelocity().Size2D();
    ActivityState = HistoricalAgent->ActivityState;
    bIsMoving = Speed > 3.0f;
    bIsSeated =
        ActivityState == ETMOPAgentActivityState::Seated;
    bIsStandingStill =
        !bIsMoving && !bIsSeated;
}
