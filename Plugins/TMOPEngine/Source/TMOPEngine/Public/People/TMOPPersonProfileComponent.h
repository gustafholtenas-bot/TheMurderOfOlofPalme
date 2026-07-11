#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPersonProfileComponent.generated.h"

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPersonProfileComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPersonProfileComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Optional override. Empty uses the owner's EntityIdentity.EntityId. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person")
    FName EntityIdOverride = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Person")
    FName ResolvedEntityId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Person")
    FTMOPPersonProfileRow Profile;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Person")
    bool bHasLoadedProfile = false;

    UFUNCTION(BlueprintCallable, Category="TMOP|Person")
    bool LoadProfile();

    UFUNCTION(BlueprintPure, Category="TMOP|Person")
    FText GetFullName() const { return Profile.FullName; }

    UFUNCTION(BlueprintPure, Category="TMOP|Person")
    float GetHeightCentimeters() const { return Profile.HeightCentimeters; }

private:
    FName ResolveEntityId() const;
};
