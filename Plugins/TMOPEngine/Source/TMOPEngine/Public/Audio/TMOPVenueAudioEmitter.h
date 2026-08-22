#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Audio/TMOPAudioTypes.h"
#include "TMOPVenueAudioEmitter.generated.h"

class UAudioComponent;
class UDataTable;
class USceneComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPVenueAudioEmitter : public AActor
{
    GENERATED_BODY()

public:
    ATMOPVenueAudioEmitter();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Venue") FName VenueId = NAME_None;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Audio|Venue") TObjectPtr<USceneComponent> AudioOrigin;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Venue") TObjectPtr<UDataTable> VenueAudioTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Venue") bool bSnapToPlacementAnchorOnBeginPlay = true;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Audio|Venue") void RefreshFromVenueTable();

private:
    void EvaluateSlots(int32 CurrentSecond);
    void StopAllSlots();
    FTMOPVenueAudioRow VenueProfile;
    TMap<FName, TWeakObjectPtr<UAudioComponent>> ActiveSlots;
    int32 LastEvaluatedSecond = INDEX_NONE;
};
