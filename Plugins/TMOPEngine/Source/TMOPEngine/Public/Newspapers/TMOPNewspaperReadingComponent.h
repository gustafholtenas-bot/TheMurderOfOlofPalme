#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPNewspaperReadingComponent.generated.h"

class UCameraComponent;
class UAnimMontage;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USkeletalMesh;
class USkeletalMeshComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTMOPNewspaperItemDefinition;

/** First-person newspaper reader which can reuse the player's existing body mesh. */
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

    /** Reuses the character mesh and attaches the paper to its hand socket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh")
    bool bUseExistingPlayerMesh = true;

    /** The paper has one parent socket; the other hand is aligned by the reading pose/IK. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh")
    FName NewspaperHandSocket = TEXT("hand_rSocket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh")
    FTransform NewspaperHandRelativeTransform = FTransform::Identity;

    /** Full-body pose played on the player's existing mesh while the newspaper is open. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh|Animation")
    TObjectPtr<UAnimMontage> NewspaperReadingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh|Animation",
        meta=(ClampMin="0.01"))
    float NewspaperReadingMontagePlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh|Animation",
        meta=(ClampMin="0.0"))
    float NewspaperReadingMontageBlendOutTime = 0.2f;

    /** Optional legacy fallback, used only when Use Existing Player Mesh is disabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D",
        meta=(EditCondition="!bUseExistingPlayerMesh", EditConditionHides))
    TObjectPtr<USkeletalMesh> FirstPersonArmsMesh;

    /** Open mesh with slots: front, pageleft, pageright, endpage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D",
        meta=(DisplayName="Open Newspaper Mesh"))
    TObjectPtr<UStaticMesh> NewspaperMesh;

    /** Folded mesh with slots: front and endpage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<UStaticMesh> FoldedNewspaperMesh;

    /** Base material for all four mesh slots. It needs a texture parameter named PageTexture. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    TObjectPtr<UMaterialInterface> NewspaperMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Materials")
    FName PageTextureParameter = TEXT("PageTexture");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Materials")
    FName FrontMaterialSlot = TEXT("front");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Materials")
    FName LeftPageMaterialSlot = TEXT("pageleft");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Materials")
    FName RightPageMaterialSlot = TEXT("pageright");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Materials")
    FName EndPageMaterialSlot = TEXT("endpage");

    /** Extra rotation used when the folded newspaper displays its back page. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FRotator FoldedBackRotationOffset = FRotator(0.0f, 180.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FTransform ArmsRelativeTransform = FTransform(FRotator::ZeroRotator,
        FVector(8.0f, 0.0f, -18.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D")
    FTransform NewspaperRelativeTransform = FTransform(FRotator(0.0f, 90.0f, 0.0f),
        FVector(65.0f, 0.0f, -18.0f), FVector(1.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Camera")
    FVector FirstPersonCameraOffset = FVector(0.0f, 0.0f, 64.0f);

    /** Looks down from eye height towards the newspaper held around waist height. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Camera")
    FRotator FirstPersonCameraRotation = FRotator(-35.0f, 0.0f, 0.0f);

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
    UMaterialInstanceDynamic* CreatePageMaterial(FName SlotName, int32 FallbackIndex);
    UCameraComponent* FindActiveCamera() const;

    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> ReadingArms;
    UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> ReadingNewspaper;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> ReadingCamera;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> PreviousCamera;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> FrontPageMID;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> LeftPageMID;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> RightPageMID;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> EndPageMID;
    UPROPERTY(Transient) TObjectPtr<UTMOPNewspaperItemDefinition> ActiveNewspaper;
    FTransform CurrentNewspaperTransform;
    bool bUsingExistingPlayerMesh = false;
    bool bShowingFoldedMesh = false;
};
