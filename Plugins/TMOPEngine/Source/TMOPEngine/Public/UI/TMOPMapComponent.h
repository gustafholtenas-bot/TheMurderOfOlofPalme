#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPMapComponent.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class ETMOPMapMarkerCategory : uint8
{
    Important,
    Metro,
    Cinema,
    Restaurant,
    Evidence,
    Custom
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

    /** North-up, top-down Stockholm image covering WorldMinimum to WorldMaximum. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    TObjectPtr<UTexture2D> MapTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    FVector2D WorldMinimum = FVector2D(-20000.0f, -20000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    FVector2D WorldMaximum = FVector2D(20000.0f, 20000.0f);

    /** Enable for ordinary images where pixel Y grows downwards. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    bool bInvertImageY = true;

    /** Converts Unreal world yaw to the orientation of the supplied map image. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Projection")
    float MapNorthYawDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Minimap",
        meta=(ClampMin="1.0", ClampMax="12.0"))
    float MinimapZoom = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Map|Minimap")
    bool bShowMinimap = true;

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

    UFUNCTION(BlueprintPure, Category="TMOP|Map")
    FVector GetTrackedWorldLocation() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Map")
    float GetTrackedMapYawDegrees() const;
};
