#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Transit/TMOPBusStopComponent.h"
#include "TMOPBusArrivalBoardComponent.generated.h"

class UDataTable;
class UTMOPMetroBoardWidget;
class UWidgetComponent;

/** Arrival estimates for a registered bus stop, using the live bus schedule. */
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPBusArrivalBoardComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPBusArrivalBoardComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board")
    FName StationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board")
    TObjectPtr<UTMOPBusStopComponent> Stop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board")
    FVector BoardOffset = FVector(0.0f, 0.0f, 285.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board")
    int32 UpcomingRows = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board",
        meta=(ClampMin="100.0", Units="cm"))
    float MaximumVisibleDistanceCm = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Board")
    bool bShowEstimateNote = true;

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


    int32 LastDisplayedSecond = INDEX_NONE;

    void EnsurePlayerBoards();
    void RefreshBoard(int32 CurrentSecond);
};

