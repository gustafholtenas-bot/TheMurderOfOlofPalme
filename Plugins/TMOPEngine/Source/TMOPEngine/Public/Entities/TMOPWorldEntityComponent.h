#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPWorldEntityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPWorldEntityRegistrationSignature,
    FName,
    EntityId,
    bool,
    bSuccessful);

UCLASS(
    ClassGroup = (TMOP),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPWorldEntityComponent final : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPWorldEntityComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TMOP|Entity")
    FName EntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TMOP|Entity")
    FName EntityType = TEXT("Entity");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Entity")
    bool bAutoRegister = true;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Entity")
    FTMOPWorldEntityRegistrationSignature OnRegistrationChanged;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Entity")
    bool RegisterEntity();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Entity")
    bool UnregisterEntity();

    UFUNCTION(BlueprintPure, Category = "TMOP|Entity")
    bool IsEntityRegistered() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Entity")
    FName GetEntityId() const { return EntityId; }

    UFUNCTION(BlueprintPure, Category = "TMOP|Entity")
    FName GetEntityType() const { return EntityType; }

    UFUNCTION(BlueprintCallable, Category = "TMOP|Entity")
    bool SetEntityIdentity(FName NewEntityId, FName NewEntityType);

private:
    bool bRegistered = false;
};
