#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Transit/TMOPMetroScheduleTypes.h"
#include "TMOPMetroEntranceBoardComponent.generated.h"

class UDataTable;
class UTMOPMetroBoardWidget;
class UWidgetComponent;

/** Runtime-created arrival display for Hötorget and Rådmansgatan entrances. */
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPMetroEntranceBoardComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPMetroEntranceBoardComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FName StationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    TObjectPtr<UDataTable> ScheduleTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FVector BoardOffset = FVector(0.0f, 0.0f, 285.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    int32 UpcomingRows = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro",
        meta=(ClampMin="100.0", Units="cm"))
    float MaximumVisibleDistanceCm = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    bool bShowSouthboundDisclaimer = true;

    void ConfigureForStation(FName NewStationId, const FText& NewStationName);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<UWidgetComponent>> BoardComponents;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTMOPMetroBoardWidget>> BoardWidgets;

    TArray<FTMOPMetroArrivalRow> Arrivals;
    int32 LastDisplayedSecond = INDEX_NONE;
    void LoadArrivals();
    void AddBuiltInArrivals();
    void EnsurePlayerBoards();
    void RefreshBoard(int32 CurrentSecond);
};
