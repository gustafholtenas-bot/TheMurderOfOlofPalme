#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPFindingActor.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPFindingActor : public AActor
{
    GENERATED_BODY()

public:
    ATMOPFindingActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Finding")
    TObjectPtr<UStaticMeshComponent> FindingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Finding")
    TObjectPtr<UTextRenderComponent> FindingLabel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding")
    FString EvidenceId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding")
    FString SourceTimeLabel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding|Geo")
    double SourceLatitude = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding|Geo")
    double SourceLongitude = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Finding|Geo")
    bool bLocationApproximate = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Finding")
    void ConfigureFinding(
        const FText& InDisplayName,
        const FString& InEvidenceId,
        const FString& InSourceReference,
        const FString& InSourceTimeLabel,
        double InLatitude,
        double InLongitude,
        bool bInLocationApproximate,
        UStaticMesh* InMesh,
        FVector InScale,
        FLinearColor InColor);
};

