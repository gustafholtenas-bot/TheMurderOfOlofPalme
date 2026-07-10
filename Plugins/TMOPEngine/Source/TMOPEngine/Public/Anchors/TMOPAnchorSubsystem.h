#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPAnchorSubsystem.generated.h"

class ATMOPHistoricalAnchor;
class ATMOPHistoricalPlace;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPAnchorRegistryChangedSignature,
    FName,
    ObjectId,
    UObject*,
    Object);

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPAnchorSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Anchors")
    FTMOPAnchorRegistryChangedSignature OnAnchorRegistered;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Anchors")
    FTMOPAnchorRegistryChangedSignature OnPlaceRegistered;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Anchors")
    bool RegisterAnchor(ATMOPHistoricalAnchor* Anchor);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Anchors")
    bool RegisterPlace(ATMOPHistoricalPlace* Place);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Anchors")
    bool UnregisterAnchor(FName AnchorId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Anchors")
    bool UnregisterPlace(FName PlaceId);

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchors")
    ATMOPHistoricalAnchor* FindAnchor(FName AnchorId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchors")
    ATMOPHistoricalPlace* FindPlace(FName PlaceId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchors")
    TArray<ATMOPHistoricalAnchor*> GetAnchorsForPlace(FName PlaceId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchors")
    TArray<ATMOPHistoricalAnchor*> GetAnchorsByCategory(
        ETMOPAnchorCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Anchors")
    int32 DiscoverAnchorsInWorld();

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchors")
    int32 GetAnchorCount() const { return Anchors.Num(); }

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchors")
    int32 GetPlaceCount() const { return Places.Num(); }

private:
    TMap<FName, TWeakObjectPtr<ATMOPHistoricalAnchor>> Anchors;
    TMap<FName, TWeakObjectPtr<ATMOPHistoricalPlace>> Places;
};
