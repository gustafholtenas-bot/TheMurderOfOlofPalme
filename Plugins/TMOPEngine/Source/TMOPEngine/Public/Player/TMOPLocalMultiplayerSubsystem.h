#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Time/TMOPTime.h"
#include "TMOPLocalMultiplayerSubsystem.generated.h"

class ATMOPPlayerCharacter;
class APlayerController;
class APlayerCameraManager;

/** One shared local session, not one simulation per viewport. Network play is not enabled. */
UCLASS()
class TMOPENGINE_API UTMOPLocalMultiplayerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="TMOP|Local Multiplayer")
    void ConfigureSession(int32 PlayerCount, bool bKeyboardForPlayerOne);
    UFUNCTION(BlueprintPure, Category="TMOP|Local Multiplayer")
    int32 GetSelectedPlayerCount() const { return SelectedPlayerCount; }
    UFUNCTION(BlueprintPure, Category="TMOP|Local Multiplayer")
    bool UsesKeyboardForPlayerOne() const { return bUseKeyboardForPlayerOne; }

    bool StartParty(FName AnchorId, const FTransform& Fallback, FText& OutError);
    bool EnsurePlayerCount(int32 Count, FText& OutError);
    void PrepareMainMenu();
    void RestartFromBeginning();
    void ReturnToMainMenu();
    void RefreshAppearances();
    void AdoptLoadedSession();
    void CloseAllPlayerMenus();
    void SetSplitScreenEnabled(bool bEnabled);

    static TArray<ATMOPPlayerCharacter*> GetPlayers(const UObject* Context);
    static bool IsMultiplayer(const UObject* Context);
    static int32 GetPlayerSlot(const ATMOPPlayerCharacter* Player);
    static APlayerCameraManager* FindNearestCamera(const UObject* Context, const FVector& Location);
    static void GetPlayerViewRect(APlayerController* PC, FVector2D& Origin, FVector2D& Size);

private:
    int32 SelectedPlayerCount = 1;
    bool bUseKeyboardForPlayerOne = true;
    FName StartAnchorId;
    FTransform StartFallback = FTransform::Identity;
    TWeakObjectPtr<UWorld> SessionWorld;
    bool bSessionStarted = false;
    bool bChangingSession = false;
    bool bRestartPartyWithClock = false;
    bool bSavedSplitScreenSetting = true;
    bool bSavedGamepadOffset = false;
    bool PlaceParty(FName AnchorId, const FTransform& Fallback, FText& OutError);
    UFUNCTION()
    void HandleLoopRestarted(int32 LoopNumber, FTMOPTime StartTime);
};
