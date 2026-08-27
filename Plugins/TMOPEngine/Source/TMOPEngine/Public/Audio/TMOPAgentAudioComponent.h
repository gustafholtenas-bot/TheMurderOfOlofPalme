#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPAgentAudioComponent.generated.h"

UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPAgentAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPAgentAudioComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps") bool bEnableDistanceBasedFootstepFallback = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps") FName DefaultWalkFootstepId = TEXT("FOOTSTEP_ASPHALT_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps") FName DefaultRunFootstepId = TEXT("FOOTSTEP_ASPHALT_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName AsphaltWalkFootstepId = TEXT("FOOTSTEP_ASPHALT_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName AsphaltRunFootstepId = TEXT("FOOTSTEP_ASPHALT_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName CarpetWalkFootstepId = TEXT("FOOTSTEP_CARPET_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName CarpetRunFootstepId = TEXT("FOOTSTEP_CARPET_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName GrassWalkFootstepId = TEXT("FOOTSTEP_GRASS_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName GrassRunFootstepId = TEXT("FOOTSTEP_GRASS_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName SnowWalkFootstepId = TEXT("FOOTSTEP_SNOW_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName SnowRunFootstepId = TEXT("FOOTSTEP_SNOW_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName StairsWalkFootstepId = TEXT("FOOTSTEP_STAIRS_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName StairsRunFootstepId = TEXT("FOOTSTEP_STAIRS_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName StoneTilesWalkFootstepId = TEXT("FOOTSTEP_STONE_TILES_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName StoneTilesRunFootstepId = TEXT("FOOTSTEP_STONE_TILES_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName WoodWalkFootstepId = TEXT("FOOTSTEP_WOOD_WALK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps|Surface IDs") FName WoodRunFootstepId = TEXT("FOOTSTEP_WOOD_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps", meta=(ClampMin="40.0", Units="cm")) float WalkStepDistanceCm = 145.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps", meta=(ClampMin="40.0", Units="cm")) float RunStepDistanceCm = 110.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps", meta=(ClampMin="1.0", Units="cm/s")) float MinimumMovingSpeedCmPerSecond = 35.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Footsteps", meta=(ClampMin="100.0", Units="cm")) float MaximumAudibleDistanceCm = 500.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Footsteps")
    void NotifyFootstep(FName ExplicitAudioId = NAME_None);

private:
    FName ResolveSurfaceFootstepId(bool bRunning) const;
    FVector PreviousLocation = FVector::ZeroVector;
    float TravelSinceFootstepCm = 0.0f;
};
