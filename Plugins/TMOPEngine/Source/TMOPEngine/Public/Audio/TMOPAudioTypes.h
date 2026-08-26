#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "TMOPAudioTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPAudioCategory : uint8
{
    BackgroundMusic,
    Footstep,
    Weapon,
    Vehicle,
    Emergency,
    Environment,
    Venue,
    Film,
    Church,
    UI
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPSoundLibraryRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AudioId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETMOPAudioCategory Category = ETMOPAudioCategory::Environment;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundBase> Sound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSoftObjectPtr<USoundBase>> Variants;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSpatial = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLoop = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float Volume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.01")) float PitchMin = 0.96f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.01")) float PitchMax = 1.04f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="cm")) float InnerRadiusCm = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0", Units="cm")) float FalloffDistanceCm = 5000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ConcurrencyGroup = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPScheduledAudioRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AudioId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="86399")) int32 StartSecond = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="-1", ClampMax="86399")) int32 EndSecond = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBackground2D = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AnchorId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float VolumeMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float FadeInSeconds = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float FadeOutSeconds = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPVenueAudioSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SlotId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AudioId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="86399")) int32 StartSecond = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="-1", ClampMax="86399")) int32 EndSecond = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLoop = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float VolumeMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float FadeInSeconds = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float FadeOutSeconds = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPVenueAudioRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName VenueId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName PlacementAnchorId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FTMOPVenueAudioSlot> AudioSlots;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="100.0", Units="cm")) float DefaultInnerRadiusCm = 400.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="100.0", Units="cm")) float DefaultFalloffDistanceCm = 2500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPVehicleAudioProfileRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ProfileId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName EngineIdleAudioId = TEXT("VEHICLE_ENGINE_IDLE");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName EngineDrivingAudioId = TEXT("VEHICLE_ENGINE_DRIVING");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName NormalStartAudioId = TEXT("VEHICLE_START_NORMAL");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName HardStartAudioId = TEXT("VEHICLE_START_HARD");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SkidAudioId = TEXT("VEHICLE_SKID");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName HornAudioId = TEXT("VEHICLE_HORN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName DoorOpenAudioId = TEXT("VEHICLE_DOOR_OPEN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName DoorCloseAudioId = TEXT("VEHICLE_DOOR_CLOSE");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TireBurstAudioId = TEXT("VEHICLE_TIRE_BURST");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LowSpeedCollisionAudioId = TEXT("VEHICLE_COLLISION_LOW");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName HighSpeedCollisionAudioId = TEXT("VEHICLE_COLLISION_HIGH");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SirenAudioId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="100.0")) float ReferenceTopSpeedCmPerSecond = 1800.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
};
