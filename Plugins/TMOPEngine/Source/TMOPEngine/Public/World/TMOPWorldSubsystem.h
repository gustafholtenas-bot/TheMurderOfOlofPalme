#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Time/TMOPTime.h"
#include "World/TMOPWorldTypes.h"
#include "TMOPWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTMOPWorldObjectChangedSignature,
    FName,
    ObjectId,
    FName,
    ObjectType,
    UObject*,
    Object);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTMOPWorldStateChangedSignature,
    FName,
    StateKey,
    FTMOPWorldStateValue,
    OldValue,
    FTMOPWorldStateValue,
    NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPWorldResetSignature,
    int32,
    LoopNumber);

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPWorldSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|World")
    FTMOPWorldObjectChangedSignature OnWorldObjectRegistered;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|World")
    FTMOPWorldObjectChangedSignature OnWorldObjectUnregistered;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|World")
    FTMOPWorldStateChangedSignature OnWorldStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|World")
    FTMOPWorldResetSignature OnRuntimeWorldReset;

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|Registry")
    bool RegisterWorldObject(FName ObjectId, FName ObjectType, UObject* Object);

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|Registry")
    bool UnregisterWorldObject(FName ObjectId, UObject* ExpectedObject = nullptr);

    UFUNCTION(BlueprintPure, Category = "TMOP|World|Registry")
    UObject* FindWorldObject(FName ObjectId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|World|Registry")
    bool IsWorldObjectRegistered(FName ObjectId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|World|Registry")
    TArray<FTMOPWorldObjectInfo> GetWorldObjectsByType(FName ObjectType) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|World|Registry")
    TArray<FName> GetRegisteredObjectIds() const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|State")
    bool SetWorldState(FName StateKey, FTMOPWorldStateValue NewValue);

    UFUNCTION(BlueprintPure, Category = "TMOP|World|State")
    bool TryGetWorldState(FName StateKey, FTMOPWorldStateValue& OutValue) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|World|State")
    bool HasWorldState(FName StateKey) const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|State")
    bool RemoveWorldState(FName StateKey);

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|State")
    void SetBaselineWorldState(FName StateKey, FTMOPWorldStateValue Value);

    UFUNCTION(BlueprintPure, Category = "TMOP|World|State")
    bool TryGetBaselineWorldState(FName StateKey, FTMOPWorldStateValue& OutValue) const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|World")
    void ResetRuntimeWorldState(int32 LoopNumber = 1);

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|Debug")
    FString BuildDebugReport() const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|World|Registry")
    int32 RemoveInvalidWorldObjects();

private:
    struct FRegisteredObject
    {
        FName ObjectType = NAME_None;
        TWeakObjectPtr<UObject> Object;
    };

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    TMap<FName, FRegisteredObject> RegisteredObjects;

    UPROPERTY(Transient)
    TMap<FName, FTMOPWorldStateValue> RuntimeWorldState;

    UPROPERTY(Transient)
    TMap<FName, FTMOPWorldStateValue> BaselineWorldState;
};
