#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPNewspaperReadingComponent.generated.h"

class UAnimSequence;
class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USkeletalMesh;
class USkeletalMeshComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTMOPNewspaperItemDefinition;

/** Camera-attached 3D arms and newspaper used by the first-person reader. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPNewspaperReadingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPNewspaperReadingComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<USkeletalMesh> FirstPersonArmsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<UAnimSequence> HoldNewspaperAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<UAnimSequence> TurnPageAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<UStaticMesh> NewspaperMesh;

    /** Material needs texture parameters named LeftPage and RightPage by default. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<UMaterialInterface> NewspaperMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FName LeftPageTextureParameter = TEXT("LeftPage");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FName RightPageTextureParameter = TEXT("RightPage");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FTransform ArmsRelativeTransform = FTransform(FRotator::ZeroRotator,
        FVector(8.0f, 0.0f, -18.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FTransform NewspaperRelativeTransform = FTransform(FRotator(0.0f, 90.0f, 0.0f),
        FVector(65.0f, 0.0f, -18.0f), FVector(1.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Camera")
    FVector FirstPersonCameraOffset = FVector(0.0f, 0.0f, 64.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Camera",
        meta=(ClampMin="30.0", ClampMax="120.0"))
    float FirstPersonFieldOfView = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D",
        meta=(ClampMin="1.0"))
    float PanStepCm = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D",
        meta=(ClampMin="0.1"))
    float ZoomStepCm = 5.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Newspaper|3D")
    bool BeginReading(UTMOPNewspaperItemDefinition* Newspaper, int32 PageIndex = 0);

    UFUNCTION(BlueprintCallable, Category="TMOP|Newspaper|3D")
    void EndReading();

    UFUNCTION(BlueprintCallable, Category="TMOP|Newspaper|3D")
    bool ShowPage(int32 PageIndex, bool bPlayTurnAnimation = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Newspaper|3D")
    void Pan(float HorizontalDirection, float VerticalDirection);

    UFUNCTION(BlueprintCallable, Category="TMOP|Newspaper|3D")
    void Zoom(float Direction);

private:
    void CreateReadingComponents();
    UCameraComponent* FindActiveCamera() const;

    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> ReadingArms;
    UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> ReadingNewspaper;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> ReadingCamera;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> PreviousCamera;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> NewspaperMID;
    UPROPERTY(Transient) TObjectPtr<UTMOPNewspaperItemDefinition> ActiveNewspaper;
    FTransform CurrentNewspaperTransform;
    float PageTurnSecondsRemaining = 0.0f;
};
