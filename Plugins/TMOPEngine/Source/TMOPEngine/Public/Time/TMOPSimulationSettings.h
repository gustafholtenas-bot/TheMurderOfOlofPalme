#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Time/TMOPTime.h"
#include "TMOPSimulationSettings.generated.h"

/**
 * Project-wide scenario clock defaults.
 *
 * Edit these under Project Settings > Plugins > TMOP Simulation.
 * Level debug tools may temporarily override the current time at runtime.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="TMOP Simulation"))
class TMOPENGINE_API UTMOPSimulationSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
    virtual FName GetSectionName() const override { return TEXT("TMOP Simulation"); }

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Scenario Clock")
    FTMOPTime ScenarioStartTime = FTMOPTime(23, 0, 0);

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Scenario Clock")
    FTMOPTime ScenarioEndTime = FTMOPTime(23, 45, 0);

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Scenario Clock",
        meta=(ClampMin="0.0", ClampMax="100.0"))
    float DefaultTimeScale = 1.0f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Debug")
    bool bEnableDebugTimeKeys = true;

    /** Five seconds keeps a 45 minute bake reasonably small even with many people. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Person Bake",
        meta=(ClampMin="1", ClampMax="60"))
    int32 PersonBakeIntervalSeconds = 5;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Person Bake")
    FString DefaultPersonBakeName = TEXT("TMOP_People_2300_2345");
};
