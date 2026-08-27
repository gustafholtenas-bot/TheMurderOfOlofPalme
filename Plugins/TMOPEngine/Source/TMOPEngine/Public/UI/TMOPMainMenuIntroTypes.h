#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TMOPMainMenuIntroTypes.generated.h"

class ACameraActor;
class UTexture2D;

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPIntroPresentationCard : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CardId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float StartSeconds = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float EndSeconds = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Heading;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true)) FText Body;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Image;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPIntroCameraShot
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float StartSeconds = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float BlendSeconds = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<ACameraActor> PlacedCamera = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFollowIntroVehicle = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bFollowIntroVehicle")) FTransform VehicleRelativeTransform = FTransform(FRotator(-12.0f, 180.0f, 0.0f), FVector(-650.0f, 0.0f, 260.0f));
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bFollowIntroVehicle")) bool bLookAtVehicle = true;
};
