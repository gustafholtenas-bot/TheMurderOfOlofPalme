#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPGrandRowComponent.generated.h"

UENUM(BlueprintType)
enum class ETMOPGrandAisleSide : uint8
{
    Automatic,
    Left,
    Right
};

/** Defines one physical seat row and its two aisle connection points. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPGrandRowComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPGrandRowComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Identity")
    FName RowId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Identity")
    FName VenueId = TEXT("PLACE_GRAND");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Identity")
    FName AuditoriumId = TEXT("GRAND_SALON_1");

    /** Seat IDs ordered physically from the left aisle to the right aisle. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Seats")
    TArray<FName> OrderedSeatIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Aisles")
    FVector LeftAisleLocalOffset = FVector(0.0f, -300.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Aisles")
    FVector RightAisleLocalOffset = FVector(0.0f, 300.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Aisles")
    FRotator LeftAisleRotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row|Aisles")
    FRotator RightAisleRotationOffset = FRotator::ZeroRotator;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row")
    bool ContainsSeat(FName SeatId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row")
    int32 GetSeatIndex(FName SeatId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row")
    ETMOPGrandAisleSide ChooseNearestAisle(FName SeatId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row")
    FTransform GetAisleWorldTransform(ETMOPGrandAisleSide Side) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Row")
    bool ValidateRow(TArray<FString>& OutErrors) const;
};
