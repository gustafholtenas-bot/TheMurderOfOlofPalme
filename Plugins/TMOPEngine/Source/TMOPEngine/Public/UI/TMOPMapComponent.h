#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPMapComponent.generated.h"

class UTexture2D;
class ATMOPHistoricalAgent;

UENUM(BlueprintType)
enum class ETMOPMapMarkerCategory : uint8
{
    Important,
    Metro,
    Cinema,
    Restaurant,
    Club,
    Pub,
    Evidence,
    Custom,
    Church,
    ATM,
    Hotel,
    BusStop
};

USTRUCT(BlueprintType)
struct FTMOPMapMarker
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    FName MarkerId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    ETMOPMapMarkerCategory Category = ETMOPMapMarkerCategory::Custom;

    /** Optional per-place override. Empty uses the category icon. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    FLinearColor Color = FLinearColor(0.95f, 0.68f, 0.12f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map")
    bool bDiscovered = true;
};

/** Shared map projection and marker data for the full map and minimap. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPMapComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPMapComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** North-up, top-down Stockholm image covering WorldMinimum to WorldMaximum. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    TObjectPtr<UTexture2D> MapTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    FVector2D WorldMinimum = FVector2D(-20000.0f, -20000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    FVector2D WorldMaximum = FVector2D(20000.0f, 20000.0f);

    /** SceneCapture2D at Pitch -90/Yaw 0 puts world Y across the image and world X vertically. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    bool bSwapWorldAxes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    bool bInvertImageX = false;

    /** Enable for ordinary images where pixel Y grows downwards. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    bool bInvertImageY = true;

    /** Optional additional rotation when the source image itself was rotated after capture. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    float MapNorthYawDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Minimap",
        meta=(ClampMin="1.0", ClampMax="12.0"))
    float MinimapZoom = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Minimap")
    bool bShowMinimap = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues")
    bool bAutoDiscoverVenueMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> RestaurantIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> CinemaIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> MetroIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> ClubIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> PubIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> ChurchIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> ATMIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> HotelIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Venues|Icons")
    TObjectPtr<UTexture2D> BusStopIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    bool bTrackOlofPalme = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    FName OlofPalmeEntityId = TEXT("OLOF_PALME");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    FText OlofPalmeMapLabel = NSLOCTEXT("TMOP", "OlofPalmeLiveMapLabel", "Olof Palme – live");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    TObjectPtr<UTexture2D> OlofPalmeIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    FLinearColor OlofPalmeMarkerColor = FLinearColor(1.0f, 0.12f, 0.08f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    bool bTrackObservedPeople = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    TObjectPtr<UTexture2D> ObservedPersonIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    FLinearColor ObservedPersonMarkerColor = FLinearColor(0.15f, 1.0f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    bool bTrackPolice = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    TObjectPtr<UTexture2D> PoliceTrackingIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking")
    FLinearColor PoliceMarkerColor = FLinearColor(0.15f, 0.45f, 1.0f, 1.0f);

    /** Agent classification is refreshed at this interval, not every frame. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Live Tracking",
        meta=(ClampMin="1.0", ClampMax="30.0", Units="s"))
    float LiveTrackingRefreshSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Markers")
    TArray<FTMOPMapMarker> Markers;

    UFUNCTION(BlueprintPure, Category="TMOP|Map")
    FVector2D WorldToMapUV(FVector WorldLocation) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Map")
    FVector MapUVToWorld(FVector2D MapUV, float WorldZ = 0.0f) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Map|Markers")
    void AddOrUpdateMarker(const FTMOPMapMarker& Marker);

    UFUNCTION(BlueprintCallable, Category="TMOP|Map|Markers")
    bool SetMarkerDiscovered(FName MarkerId, bool bDiscovered);

    /** Finds venue entrance anchors and creates one labelled marker per public place. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Map|Venues")
    int32 DiscoverVenueMarkers();

    UFUNCTION(BlueprintPure, Category="TMOP|Map|Venues")
    UTexture2D* GetCategoryIcon(ETMOPMapMarkerCategory Category) const;

    /** Returns false while Olof has not spawned or after he has despawned. */
    UFUNCTION(BlueprintPure, Category="TMOP|Map|Live Tracking")
    bool GetOlofPalmeMapLocation(FVector& OutWorldLocation) const;

    void GetObservedPersonMapLocations(TArray<FVector>& OutLocations) const;
    void GetPoliceMapLocations(TArray<FVector>& OutLocations) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Map")
    FVector GetTrackedWorldLocation() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Map")
    FVector2D GetTrackedMapDirection() const;

private:
    void RefreshLiveTrackingCache();
    float LiveTrackingRefreshRemaining = 0.0f;
    bool bCachedOlofPalmeLocationValid = false;
    FVector CachedOlofPalmeLocation = FVector::ZeroVector;
    TArray<FVector> CachedObservedPeopleLocations;
    TArray<FVector> CachedPoliceLocations;
};
