#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "UI/TMOPMainMenuIntroTypes.h"
#include "TMOPMainMenuIntroDirector.generated.h"

class ACameraActor;
class ATMOPConfiguredVehicle;
class ATMOPHistoricalAgent;
class ATMOPPlayerCharacter;
class UDataTable;
class UTMOPMainMenuWidget;
class UTMOPVehicleModelData;
class UTexture2D;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPMainMenuIntroDirector : public AActor
{
    GENERATED_BODY()
public:
    ATMOPMainMenuIntroDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu") bool bEnableMainMenu = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu") bool bEnableIntro = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu") TObjectPtr<ACameraActor> MainMenuBackgroundCamera;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu") TObjectPtr<UTexture2D> GameLogo;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu") TSubclassOf<UTMOPMainMenuWidget> MainMenuWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu") FString SaveSlotName = TEXT("TMOP_QuickSave");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") TSubclassOf<ATMOPConfiguredVehicle> IntroVehicleClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") TObjectPtr<UTMOPVehicleModelData> IntroVehicleModel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") FName IntroStartAnchorId = TEXT("EnterKungsgatanE_Car");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") FName IntroDestinationAnchorId = TEXT("Grand_entrance");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") float DestinationAcceptanceRadiusCm = 350.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") float IntroVehicleSpeedMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Driver") TSubclassOf<ATMOPHistoricalAgent> IntroDriverClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Driver") FName IntroDriverEntityId = TEXT("INTRO_JAN_NILSSON");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Driver") FText IntroDriverDisplayName = NSLOCTEXT("TMOP", "IntroJanNilsson", "Jan Nilsson");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Camera") TArray<FTMOPIntroCameraShot> CameraShots;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Presentation") TObjectPtr<UDataTable> IntroCardsTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Finish") FTMOPTime GameStartTime = FTMOPTime(23, 0, 0);

    UFUNCTION(BlueprintCallable, Category="TMOP|Main Menu") void StartNewGame();
    UFUNCTION(BlueprintCallable, Category="TMOP|Main Menu") void LoadGame();
    UFUNCTION(BlueprintCallable, Category="TMOP|Main Menu") void OpenSettings();
    UFUNCTION(BlueprintCallable, Category="TMOP|Main Menu") void QuitGame();
    UFUNCTION(BlueprintCallable, Category="TMOP|Intro") void SkipIntro();

private:
    void TryInitializeMenu();
    bool SpawnAndStartIntro();
    void UpdateIntro(float DeltaSeconds);
    void ApplyCameraShot(int32 Index);
    void UpdateFollowCamera();
    void UpdateIntroCard();
    void FinishIntro();
    void SetMenuInput(bool bMenuInput);
    ATMOPPlayerCharacter* GetPlayerCharacter() const;

    UPROPERTY(Transient) TObjectPtr<UTMOPMainMenuWidget> MainMenuWidget;
    UPROPERTY(Transient) TObjectPtr<ATMOPConfiguredVehicle> IntroVehicle;
    UPROPERTY(Transient) TObjectPtr<ATMOPHistoricalAgent> IntroDriver;
    UPROPERTY(Transient) TObjectPtr<ACameraActor> RuntimeFollowCamera;
    float IntroElapsedSeconds = 0.0f;
    int32 ActiveCameraShotIndex = INDEX_NONE;
    FName ActiveCardId = NAME_None;
    bool bInitialized = false;
    bool bIntroActive = false;
    bool bWaitingForSettingsClose = false;
};
