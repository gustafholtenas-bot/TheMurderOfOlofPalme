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

/** First-person newspaper reader with an editor-adjustable floating paper. */
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

    /** Optional legacy mode. Disabled by default so the paper floats in front of the player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Player Mesh")
    bool bUseExistingPlayerMesh = false;

    /**
     * Authoritative modern layout. When enabled, stale Blueprint values for
     * hand sockets and reading arms cannot pull the paper back into one hand.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Floating")
    bool bForceFloatingWaistLayout = true;

    /** Centres meshes whose imported pivot is at an edge instead of the page centre. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Floating")
    bool bCenterMeshBoundsOnFloatingTransform = true;

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

    /** Optional first-person arms. Keep disabled for a freely positioned newspaper. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Floating")
    bool bShowReadingArms = false;

    /** Optional legacy arms mesh, used only when Show Reading Arms is enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D",
        meta=(EditCondition="!bUseExistingPlayerMesh && bShowReadingArms", EditConditionHides))
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Floating",
        meta=(DisplayName="Floating Arms Transform", EditCondition="bShowReadingArms",
            EditConditionHides, MakeEditWidget="true"))
    FTransform ArmsRelativeTransform = FTransform(FRotator::ZeroRotator,
        FVector(8.0f, 0.0f, -18.0f));

    /** Position, rotation/tilt and scale relative to the dedicated reading camera. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Newspaper|3D|Floating",
        meta=(DisplayName="Floating Newspaper Transform", MakeEditWidget="true"))
    FTransform NewspaperRelativeTransform = FTransform(FRotator(-10.0f, 90.0f, 0.0f),
        FVector(65.0f, 0.0f, -45.0f), FVector(1.0f));

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
    void ApplyNewspaperTransform(UStaticMesh* Mesh);

    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> ReadingArms;
    UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> ReadingNewspaper;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> ReadingCamera;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> PreviousCamera;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> HiddenPlayerMesh;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> FrontPageMID;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> LeftPageMID;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> RightPageMID;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> EndPageMID;
    UPROPERTY(Transient) TObjectPtr<UTMOPNewspaperItemDefinition> ActiveNewspaper;
    FTransform CurrentNewspaperTransform;
    bool bUsingExistingPlayerMesh = false;
    bool bShowingFoldedMesh = false;
    bool bPreviousPlayerMeshOwnerNoSee = false;
    bool bPlayerMeshVisibilityOverridden = false;
};
