#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPGrandAuditoriumExitComponent.generated.h"

/** A walkable destination immediately outside one auditorium door. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPGrandAuditoriumExitComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPGrandAuditoriumExitComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Auditorium Exit")
    FName ExitId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Auditorium Exit")
    FName VenueId = TEXT("PLACE_GRAND");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Auditorium Exit")
    FName AuditoriumId = TEXT("GRAND_SALON_1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Auditorium Exit",
        meta=(ClampMin="10.0"))
    float AcceptanceRadius = 60.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Auditorium Exit")
    bool ValidateExit(TArray<FString>& OutErrors) const;
};
