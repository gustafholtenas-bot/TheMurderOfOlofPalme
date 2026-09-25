#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "TMOPMapWidget.generated.h"

class ATMOPPlayerCharacter;
class UTMOPMapComponent;

/** Native map view. Full-map mode supports zoom/pan; minimap mode follows the player. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeMap(UTMOPMapComponent* InMapComponent,
        ATMOPPlayerCharacter* InPlayerCharacter, bool bInMinimap);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Map")
    void SetMapVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Map")
    void ResetViewToPlayer();

    UFUNCTION(BlueprintPure, Category="TMOP|UI|Map")
    UTMOPMapComponent* GetMapComponent() const { return MapComponent; }

    void SetEmbeddedInMenu(bool bEmbedded) { bEmbeddedInMenu = bEmbedded; }

    bool IsMinimap() const { return bMinimap; }
    float GetZoom() const;
    FVector2D GetViewCenterUV() const;
    void ChangeZoom(float WheelDelta);
    void PanByPixels(FVector2D PixelDelta, FVector2D ViewSize);
    void RequestClose();

    bool ShouldShowPlaces() const { return bShowPlaces; }
    bool ShouldShowObservations() const { return bShowObservations; }
    bool ShouldShowWitnesses() const { return bShowWitnesses; }
    bool InspectMapPerson(FName EntityId);
    bool ShouldShowPolice() const { return bShowPolice; }
    void ToggleMapFilter(int32 FilterIndex);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UTMOPMapComponent> MapComponent;

    UPROPERTY(Transient)
    TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;

    bool bMinimap = false;
    bool bEmbeddedInMenu = false;
    bool bShowPlaces = true;
    bool bShowObservations = true;
    bool bShowPolice = true;
    bool bShowWitnesses = true;
    float FullMapZoom = 2.0f;
    FVector2D FullMapCenterUV = FVector2D(0.5f, 0.5f);
    FSlateBrush MapBrush;
};
