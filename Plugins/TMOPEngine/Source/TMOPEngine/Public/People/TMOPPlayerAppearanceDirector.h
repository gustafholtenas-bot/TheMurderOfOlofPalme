#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPlayerAppearanceDirector.generated.h"

class ACharacter;
class UDataTable;
class USkeletalMesh;
class USkeletalMeshComponent;

/**
 * Level actor that gives the playable character an evidence/catalog driven
 * modular appearance using the same resolver and DT_TMOP_AppearanceAssets as
 * historical NPCs.
 */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API ATMOPPlayerAppearanceDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPPlayerAppearanceDirector();
    virtual void BeginPlay() override;

    /** Optional placed character. Empty resolves GetPlayerCharacter(PlayerIndex). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Target")
    TObjectPtr<ACharacter> TargetCharacterOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Target",
        meta=(ClampMin="0"))
    int32 PlayerIndex = 0;

    /** Optional full-body component. Empty uses ACharacter::GetMesh(). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Target")
    TObjectPtr<USkeletalMeshComponent> BodyMeshOverride;

    /** When enabled, appearance is copied from one DT_TMOP_People row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Profile")
    bool bUsePersonProfileRow = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Profile",
        meta=(EditCondition="bUsePersonProfileRow"))
    FDataTableRowHandle PlayerProfileRow;

    /** Used when no person row is selected. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Profile",
        meta=(EditCondition="!bUsePersonProfileRow"))
    ETMOPPersonGender PlayerGender = ETMOPPersonGender::Male;

    /** Expand this to choose Body, Hair, Outerwear, Headwear and all other
     *  catalog IDs exactly as they are chosen for an NPC. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Profile",
        meta=(EditCondition="!bUsePersonProfileRow"))
    FTMOPAppearanceProfile InlineAppearanceProfile;

    /** Optional catalog. Empty uses the registry's DT_TMOP_AppearanceAssets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Catalog")
    TObjectPtr<UDataTable> AppearanceAssetTableOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Body")
    bool bAutomaticallySelectMannyOrQuinnByGender = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Body",
        meta=(EditCondition="bAutomaticallySelectMannyOrQuinnByGender"))
    TSoftObjectPtr<USkeletalMesh> MaleBaseBodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Body",
        meta=(EditCondition="bAutomaticallySelectMannyOrQuinnByGender"))
    TSoftObjectPtr<USkeletalMesh> FemaleBaseBodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Startup")
    bool bApplyOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Startup",
        meta=(ClampMin="0.0", Units="s", EditCondition="bApplyOnBeginPlay"))
    float InitialApplyDelaySeconds = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Startup",
        meta=(ClampMin="0", EditCondition="bApplyOnBeginPlay"))
    int32 MaximumStartupRetries = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Appearance|Startup",
        meta=(ClampMin="0.05", Units="s", EditCondition="bApplyOnBeginPlay"))
    float StartupRetryIntervalSeconds = 0.25f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Appearance")
    FTMOPResolvedAppearance ResolvedAppearance;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Appearance")
    bool bHasAppliedAppearance = false;

    /** Applies/reapplies the current director settings to the player. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Player Appearance")
    bool ApplyPlayerAppearance();

    /** Hides the modular pieces created/managed by this director. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Player Appearance")
    void ClearPlayerAppearance();

    /** Useful after changing the possessed pawn or changing clothes at runtime. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Player Appearance")
    bool RefreshPlayerAppearance();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Player Appearance")
    void ValidatePlayerAppearance();

private:
    ACharacter* ResolveTargetCharacter() const;
    USkeletalMeshComponent* ResolveBodyMesh(ACharacter* Character) const;
    UDataTable* ResolveAssetCatalog() const;
    bool BuildPlayerProfile(FTMOPPersonProfileRow& OutProfile) const;
    USkeletalMeshComponent* EnsurePartComponent(ACharacter* Character,
        USkeletalMeshComponent* Body, FName ComponentName);
    bool ApplyResolvedPart(ACharacter* Character, USkeletalMeshComponent* Body,
        FName ComponentName, const FTMOPResolvedAppearancePart& Part);
    bool ApplyResolvedBody(USkeletalMeshComponent* Body,
        const FTMOPPersonProfileRow& Profile);
    void ApplyBodyRegionMask(USkeletalMeshComponent* Body);
    void TryStartupApply();

    UPROPERTY(Transient)
    TArray<TObjectPtr<USkeletalMeshComponent>> ManagedPartComponents;

    FTimerHandle StartupRetryTimer;
    int32 RemainingStartupRetries = 0;
};
