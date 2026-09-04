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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu|Save")
    FString ManualSaveSlotPrefix = TEXT("TMOP_Manual_");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Main Menu|Save",
        meta=(ClampMin="1", ClampMax="99"))
    int32 ManualSaveSlotCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") TSubclassOf<ATMOPConfiguredVehicle> IntroVehicleClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") TObjectPtr<UTMOPVehicleModelData> IntroVehicleModel;
    /** Automatic keeps the old anchor-to-anchor setup. Vehicle Editor Timeline
     * uses one Begin Driving/Enter Traffic Route row from the vehicle table. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle|Route")
    ETMOPIntroRouteSource IntroRouteSource = ETMOPIntroRouteSource::AutomaticFromAnchors;
    /** Optional override. Empty searches the Historical Vehicle Director in the level. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle|Route",
        meta=(EditCondition="IntroRouteSource==ETMOPIntroRouteSource::VehicleEditorTimeline"))
    TObjectPtr<UDataTable> IntroVehicleTable;
    /** Vehicle Id (or DataTable row name) created in TMOP Vehicle Editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle|Route",
        meta=(EditCondition="IntroRouteSource==ETMOPIntroRouteSource::VehicleEditorTimeline"))
    FName IntroRouteVehicleId = TEXT("VEHICLE_INTRO_JAN_NILSSON_LIMO");
    /** Empty selects the first driving row. Otherwise this must match Entry Id. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle|Route",
        meta=(EditCondition="IntroRouteSource==ETMOPIntroRouteSource::VehicleEditorTimeline"))
    FName IntroRouteEntryId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle|Route",
        meta=(EditCondition="IntroRouteSource==ETMOPIntroRouteSource::VehicleEditorTimeline"))
    bool bFallbackToAutomaticIntroRoute = true;
    /** Uses the model, body colour and roof accessory from the selected vehicle row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle|Route",
        meta=(EditCondition="IntroRouteSource==ETMOPIntroRouteSource::VehicleEditorTimeline"))
    bool bInheritIntroVehicleAppearance = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") FName IntroStartAnchorId = TEXT("EnterKungsgatanE_Car");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") FName IntroDestinationAnchorId = TEXT("Grand_entrance");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") float DestinationAcceptanceRadiusCm = 350.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Vehicle") float IntroVehicleSpeedMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Driver") TSubclassOf<ATMOPHistoricalAgent> IntroDriverClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Driver") FName IntroDriverEntityId = TEXT("INTRO_JAN_NILSSON");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Driver") FText IntroDriverDisplayName = NSLOCTEXT("TMOP", "IntroJanNilsson", "Jan Nilsson");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Camera") TArray<FTMOPIntroCameraShot> CameraShots;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Presentation") TObjectPtr<UDataTable> IntroCardsTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Presentation")
    FTMOPIntroTextPresentationSettings IntroTextSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Intro|Finish") FTMOPTime GameStartTime = FTMOPTime(23, 0, 0);

    UFUNCTION(BlueprintCallable, Category="TMOP|Main Menu") void StartNewGame();
    UFUNCTION(BlueprintCallable, Category="TMOP|Main Menu") void LoadGame();
    bool LoadGameSlot(const FString& SlotName);
    void CloseLoadGameMenu();
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
    FName ActiveIntroDestinationAnchorId = NAME_None;
};
