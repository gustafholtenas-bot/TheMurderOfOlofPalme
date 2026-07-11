#pragma once

#include "CoreMinimal.h"
#include "TMOPAnimationTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPAnimPosture : uint8
{
    Standing,
    Sitting,
    SittingInCar,
    Squatting,
    Grounded
};

UENUM(BlueprintType)
enum class ETMOPAnimLocomotionStyle : uint8
{
    Normal,
    Fast,
    Drunk,
    MildRun,
    FastRun,
    HurtLeg
};

UENUM(BlueprintType)
enum class ETMOPAnimOverlay : uint8
{
    None,
    Nervous,
    Smoking,
    Dancing,
    Talking,
    PhoneOrRadio,
    LookAround
};

UENUM(BlueprintType)
enum class ETMOPAnimReaction : uint8
{
    None,
    SurprisedOrScared,
    HitButStanding,
    FallingFromHit,
    GettingUp,
    Kick,
    Punch,
    SwingWeapon,
    AimGun,
    ShootGun
};

UENUM(BlueprintType)
enum class ETMOPAnimWeaponPose : uint8
{
    None,
    SwordLikeRightHand,
    GunRightHand
};
