#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPGrandAuditoriumExitDirector.generated.h"

class ATMOPHistoricalAgent;
class UTMOPGrandAuditoriumExitComponent;

UENUM(BlueprintType)
enum class ETMOPGrandAuditoriumExitState : uint8
{
    Queued,
    Moving,
    LeftAuditorium,
    Failed
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandAuditoriumExitStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Auditorium Exit")
    TObjectPtr<ATMOPHistoricalAgent> Agent;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Auditorium Exit")
    FName ExitId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Auditorium Exit")
    ETMOPGrandAuditoriumExitState State = ETMOPGrandAuditoriumExitState::Queued;
};

/** Moves agents from row aisles through the auditorium doors. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandAuditoriumExitDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandAuditoriumExitDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Auditorium Exit",
        meta=(ClampMin="0.05"))
    float ScanIntervalSeconds = 0.20f;

    /** One moving agent per door prevents a pile-up in narrow doorways. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Auditorium Exit",
        meta=(ClampMin="1", ClampMax="4"))
    int32 MaximumSimultaneousPerExit = 1;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Auditorium Exit")
    int32 DiscoverExits();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Auditorium Exit")
    bool ValidateExits(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Auditorium Exit")
    void ResetAuditoriumExits();

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Auditorium Exit")
    int32 GetQueuedCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Auditorium Exit")
    int32 GetLeftAuditoriumCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Auditorium Exit")
    TArray<FTMOPGrandAuditoriumExitStatus> GetExitStatuses() const { return Statuses; }

private:
    void ImportReachedAisleAgents();
    void StartQueuedAgents();
    void UpdateMovingAgents();
    UTMOPGrandAuditoriumExitComponent* ChooseExit(ATMOPHistoricalAgent* Agent) const;
    UTMOPGrandAuditoriumExitComponent* FindExit(FName ExitId) const;
    int32 GetMovingCountForExit(FName ExitId) const;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTMOPGrandAuditoriumExitComponent>> Exits;

    UPROPERTY(Transient)
    TArray<FTMOPGrandAuditoriumExitStatus> Statuses;

    float ScanAccumulator = 0.0f;
};
