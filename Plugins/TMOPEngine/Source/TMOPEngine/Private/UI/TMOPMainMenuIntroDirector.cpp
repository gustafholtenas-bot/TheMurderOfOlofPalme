#include "UI/TMOPMainMenuIntroDirector.h"
#include "Time/TMOPSimulationDebugDirector.h"
#include "HAL/FileManager.h"
#include "People/TMOPPlayerAppearanceDirector.h"
#include "Misc/PackageName.h"
#include "Engine/Engine.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "UI/TMOPMainMenuWidget.h"
#include "UI/TMOPPauseMenuWidget.h"
#include "UI/TMOPSaveGameService.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Vehicles/TMOPVehicleModelData.h"

namespace
{
const FTMOPHistoricalVehicleRow* FindIntroVehicleRow(
    UDataTable* Table, const FName VehicleId)
{
    if (!IsValid(Table) || VehicleId.IsNone()) return nullptr;
    if (Table->GetRowStruct() != FTMOPHistoricalVehicleRow::StaticStruct())
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP intro: Intro Vehicle Table has the wrong row type."));
        return nullptr;
    }
    if (const FTMOPHistoricalVehicleRow* Direct =
        Table->FindRow<FTMOPHistoricalVehicleRow>(VehicleId, TEXT("TMOP intro"), false))
        return Direct;
    for (const TPair<FName, uint8*>& Pair : Table->GetRowMap())
    {
        const FTMOPHistoricalVehicleRow* Row =
            reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
        if (Row != nullptr && Row->VehicleId == VehicleId) return Row;
    }
    return nullptr;
}

bool IsDrivingEntry(const FTMOPHistoricalVehicleTimelineEntry& Entry)
{
    return Entry.Action == ETMOPHistoricalVehicleAction::BeginDriving ||
        Entry.Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
}

FName FindPlacementAnchorBefore(const FTMOPHistoricalVehicleRow& Row,
    const int32 DrivingIndex)
{
    for (int32 Index = DrivingIndex - 1; Index >= 0; --Index)
        if (!Row.Timeline[Index].PlacementAnchorId.IsNone())
            return Row.Timeline[Index].PlacementAnchorId;
    return NAME_None;
}

FName FindPlacementAnchorAfter(const FTMOPHistoricalVehicleRow& Row,
    const int32 DrivingIndex)
{
    for (int32 Index = DrivingIndex + 1; Index < Row.Timeline.Num(); ++Index)
        if (!Row.Timeline[Index].PlacementAnchorId.IsNone())
            return Row.Timeline[Index].PlacementAnchorId;
    return NAME_None;
}

float DistanceOnLane(UTMOPTrafficLaneComponent* Lane, const FVector& Location)
{
    if (!IsValid(Lane)) return 0.0f;
    const float Key = Lane->FindInputKeyClosestToWorldLocation(Location);
    return Lane->GetDistanceAlongSplineAtSplineInputKey(Key);
}
}

ATMOPMainMenuIntroDirector::ATMOPMainMenuIntroDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    IntroVehicleClass = ATMOPConfiguredVehicle::StaticClass();
    IntroDriverClass = ATMOPHistoricalAgent::StaticClass();
    MainMenuWidgetClass = UTMOPMainMenuWidget::StaticClass();
}

void ATMOPMainMenuIntroDirector::BeginPlay()
{
    Super::BeginPlay();
    // Offline bake authoring must not wait for the new-game/appearance flow.
    for (TActorIterator<ATMOPSimulationDebugDirector> It(GetWorld()); It; ++It)
        if (It->bBakeOnNextBeginPlay || IFileManager::Get().FileExists(*(It->GetResolvedBakePath() + TEXT(".request"))))
        {
            bInitialized = true;
            SetActorTickEnabled(false);
            return;
        }
    if (GEngine)
        TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ATMOPMainMenuIntroDirector::HandleAppearanceTravelFailure);
    if (GetGameInstance())
    {
        auto* Session = GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
        bArrivedFromAppearanceTravel = Session->ConsumeAppearanceTravel(GetWorld());
        if (bArrivedFromAppearanceTravel)
        {
            LocalPlayerCount = Session->GetSelectedPlayerCount();
            bKeyboardForPlayerOne = Session->UsesKeyboardForPlayerOne();
            bSharedKeyboardForPlayerTwo = Session->UsesSharedKeyboardForPlayerTwo();
            AppearanceLobby.Begin(LocalPlayerCount);
            for (int32 Slot=0; Slot<LocalPlayerCount; ++Slot)
                AppearanceLobby.Confirm(Slot, AppearanceLobby.GetRevision(Slot));
        }
    }
    if (!bEnableMainMenu && !bArrivedFromAppearanceTravel)
    {
        bInitialized = true;
        return;
    }
    if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
        Clock->PauseClock();
}

void ATMOPMainMenuIntroDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
    ReleaseAppearanceWorldPause();
    SetMenuInput(false);
    if (ATMOPPlayerCharacter* Player = GetPlayerCharacter())
    {
        Player->SetGameplayHUDHidden(TEXT("MainMenu"), false);
        Player->SetGameplayHUDHidden(TEXT("Cinematic"), false);
    }
    if (IsValid(MainMenuWidget)) MainMenuWidget->RemoveFromParent();
    if (IsValid(RuntimeFollowCamera)) RuntimeFollowCamera->Destroy();
    Super::EndPlay(EndPlayReason);
}

void ATMOPMainMenuIntroDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bInitialized) TryInitializeMenu();
    // World timers do not advance during a real pause. This actor's paused
    // tick drains the ready request after the Slate callback has returned.
    if (bAppearanceSetupActive && bStartQueued &&
        AppearanceLobby.GetGeneration() == QueuedAppearanceGeneration && AppearanceLobby.AllReady())
    {
        bStartQueued = false;
        StartNewGame();
    }
    if (bWaitingForSettingsClose)
    {
        if (ATMOPPlayerCharacter* Player = GetPlayerCharacter())
            if (!Player->bPauseMenuOpen)
            {
                bWaitingForSettingsClose = false;
                if (IsValid(MainMenuWidget)) MainMenuWidget->SetMenuMode(true);
                SetMenuInput(true);
            }
    }
    if (bIntroActive) UpdateIntro(DeltaSeconds);
}

ATMOPPlayerCharacter* ATMOPMainMenuIntroDirector::GetPlayerCharacter() const
{
    return Cast<ATMOPPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void ATMOPMainMenuIntroDirector::TryInitializeMenu()
{
    APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
    if (!IsValid(Controller) || !IsValid(GetPlayerCharacter())) return;
    if (GetGameInstance() && !bArrivedFromAppearanceTravel)
        GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>()->PrepareMainMenu();
    TSubclassOf<UTMOPMainMenuWidget> WidgetClass = MainMenuWidgetClass;
    if (!WidgetClass) WidgetClass = UTMOPMainMenuWidget::StaticClass();
    MainMenuWidget = CreateWidget<UTMOPMainMenuWidget>(Controller, WidgetClass);
    if (!IsValid(MainMenuWidget)) return;
    MainMenuWidget->InitializeMainMenu(this, GameLogo);
    MainMenuWidget->ConfigureIntroText(IntroTextSettings);
    MainMenuWidget->AddToViewport(1000);
    MainMenuWidget->SetMenuMode(true);
    GetPlayerCharacter()->SetGameplayHUDHidden(TEXT("MainMenu"), true);
    if (IsValid(MainMenuBackgroundCamera))
        Controller->SetViewTarget(MainMenuBackgroundCamera);
    SetMenuInput(true);
    bInitialized = true;
    if (bArrivedFromAppearanceTravel)
        GetWorldTimerManager().SetTimerForNextTick(this, &ATMOPMainMenuIntroDirector::StartNewGame);
}

void ATMOPMainMenuIntroDirector::SetMenuInput(const bool bMenuInput)
{
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance) return;
    APlayerController* PrimaryController = UGameplayStatics::GetPlayerController(this, 0);
    if (!IsValid(PrimaryController)) return;

    // Ignore-input uses a counter. Track each controller because players 2–4
    // may be created while player 1 already owns the menu input lock.
    if (bMenuInput)
    {
        for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
            if (APlayerController* Controller = LocalPlayer
                ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr)
            {
                if (!MenuInputControllers.Contains(Controller))
                {
                    Controller->SetIgnoreMoveInput(true);
                    Controller->SetIgnoreLookInput(true);
                    MenuInputControllers.Add(Controller);
                }
                Controller->bShowMouseCursor = Controller == PrimaryController;
                if (Controller != PrimaryController && IsValid(MainMenuWidget))
                {
                    FInputModeUIOnly Mode;
                    Mode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
                    Controller->SetInputMode(Mode);
                    MainMenuWidget->SetUserFocus(Controller);
                }
            }
    }
    else
    {
        for (const TWeakObjectPtr<APlayerController>& Entry : MenuInputControllers)
            if (APlayerController* Controller = Entry.Get())
            {
                Controller->SetIgnoreMoveInput(false);
                Controller->SetIgnoreLookInput(false);
                Controller->bShowMouseCursor = false;
                Controller->SetInputMode(FInputModeGameOnly());
            }
        MenuInputControllers.Reset();
    }
    bMenuInputApplied = bMenuInput;

    if (bMenuInput && IsValid(MainMenuWidget))
    {
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PrimaryController->SetInputMode(Mode);
    }
    else PrimaryController->SetInputMode(FInputModeGameOnly());
}

bool ATMOPMainMenuIntroDirector::PauseAppearanceWorld()
{
    if (UGameplayStatics::IsGamePaused(this)) return true;
    bOwnsAppearanceWorldPause = UGameplayStatics::SetGamePaused(this, true);
    if (!bOwnsAppearanceWorldPause)
        StartupStatus = FText::FromString(TEXT("Kunde inte pausa spelvärlden. Kontrollera att GameMode tillåter paus."));
    return bOwnsAppearanceWorldPause;
}

bool ATMOPMainMenuIntroDirector::ReleaseAppearanceWorldPause()
{
    if (!bOwnsAppearanceWorldPause) return true;
    if (!UGameplayStatics::SetGamePaused(this, false))
    {
        StartupStatus = FText::FromString(TEXT("Kunde inte återuppta spelvärlden."));
        return false;
    }
    bOwnsAppearanceWorldPause = false;
    return true;
}

void ATMOPMainMenuIntroDirector::HandleAppearanceTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Message)
{
    if (World != GetWorld() || !bAppearanceTravelPending) return;
    bAppearanceTravelPending = false;
    bNewGameRequested = false;
    bStartQueued = false;
    bAppearanceSetupActive = true;
    GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>()->QueueAppearanceTravel(NAME_None);
    AppearanceLobby.Begin(LocalPlayerCount);
    PauseAppearanceWorld();
    StartupStatus = FText::FromString(TEXT("Kunde inte ladda spelnivån: ") + Message);
    if (MainMenuWidget) MainMenuWidget->ShowAppearanceSetup(LocalPlayerCount);
    SetMenuInput(true);
}

bool ATMOPMainMenuIntroDirector::BeginAppearanceSetup(int32 Count)
{
    if (bNewGameRequested || !GetGameInstance() || Count < 1 || Count > 4) return false;
    auto* Session = GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
    LocalPlayerCount = Count;
    AppearanceLobby.Begin(Count);
    bStartQueued = false;
    Session->ConfigureSession(Count, bKeyboardForPlayerOne, bSharedKeyboardForPlayerTwo);
    if (!Session->EnsurePlayerCount(Count, StartupStatus)) return false;
    Session->SetSplitScreenEnabled(false);
    Session->RefreshAppearances();
    if (auto* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()) Clock->PauseClock();
    if (!PauseAppearanceWorld()) return false;
    for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
        Player->SetGameplayHUDHidden(TEXT("MainMenu"), true);
    bAppearanceSetupActive = true;
    StartupStatus = FText::FromString(TEXT("Välj utseende på varje spelares flik. Alla måste trycka Klar."));
    if (MainMenuWidget) MainMenuWidget->ShowAppearanceSetup(Count);
    SetMenuInput(true);
    return true;
}

void ATMOPMainMenuIntroDirector::CancelAppearanceSetup()
{
    if (bNewGameRequested) return;
    AppearanceLobby.Cancel();
    bAppearanceSetupActive = false;
    bStartQueued = false;
    SetMenuInput(false);
    ReleaseAppearanceWorldPause();
    if (MainMenuWidget) MainMenuWidget->HideAppearanceSetup();
    if (GetGameInstance())
    {
        auto* Session = GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
        Session->QueueAppearanceTravel(NAME_None);
        Session->PrepareMainMenu();
    }
    StartupStatus = FText::GetEmpty();
    if (MainMenuWidget) MainMenuWidget->ShowPlayerCountPage();
    SetMenuInput(true);
}

void ATMOPMainMenuIntroDirector::AppearanceEdited(int32 Slot)
{
    if (!bAppearanceSetupActive || bNewGameRequested) return;
    if (AppearanceLobby.Edit(Slot))
    {
        bStartQueued = false;
        StartupStatus = FText::FromString(TEXT("Ändrat utseende behöver bekräftas med Klar."));
    }
}

void ATMOPMainMenuIntroDirector::ConfirmPlayerAppearance(int32 Slot)
{
    if (!bAppearanceSetupActive || bNewGameRequested || !AppearanceLobby.Contains(Slot)) return;
    const unsigned Revision = AppearanceLobby.GetRevision(Slot);
    auto* Player = Cast<ATMOPPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, Slot));
    auto* Appearance = ATMOPPlayerAppearanceDirector::ForCharacter(Player);
    FTMOPPersonProfileRow Profile;
    FString Error;
    if (!Appearance || !Appearance->GetEditableProfile(Profile) ||
        !Appearance->PreviewProfile(Profile, Error) || !Appearance->SaveEditedProfile(Error))
    {
        AppearanceLobby.Edit(Slot);
        StartupStatus = FText::FromString(Error.IsEmpty() ? TEXT("Kunde inte validera eller spara spelarens utseende.") : Error);
        return;
    }
    if (!AppearanceLobby.Confirm(Slot, Revision)) return;
    StartupStatus = FText::FromString(TEXT("Klar. Väntar tills alla spelare har bekräftat."));
    if (AppearanceLobby.AllReady() && !bStartQueued)
    {
        bStartQueued = true;
        QueuedAppearanceGeneration = AppearanceLobby.GetGeneration();
    }
}

void ATMOPMainMenuIntroDirector::StartNewGame()
{
    if (bNewGameRequested || !GetGameInstance()) return;
    if ((bEnableMainMenu || bArrivedFromAppearanceTravel) &&
        (!AppearanceLobby.AllReady() || AppearanceLobby.GetCount() != LocalPlayerCount))
    {
        StartupStatus = FText::FromString(TEXT("Alla spelare måste välja utseende och trycka Klar."));
        if (!bAppearanceSetupActive && MainMenuWidget) MainMenuWidget->ShowPlayerCountPage();
        return;
    }
    bNewGameRequested = true;
    StartupStatus = FText::GetEmpty();
    ActiveIntroDestinationAnchorId = IntroDestinationAnchorId;
    UTMOPLocalMultiplayerSubsystem* LocalSession =
        GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
    LocalSession->ConfigureSession(
        LocalPlayerCount, bKeyboardForPlayerOne, bSharedKeyboardForPlayerTwo);
    // Create every selected local player before hiding the menu. Previously
    // this happened only when the optional driving intro ended, so a stalled
    // route looked like a frozen menu and the world contained only player 1.
    if (!LocalSession->EnsurePlayerCount(LocalPlayerCount, StartupStatus))
    {
        bNewGameRequested = false;
        UE_LOG(LogTemp, Error, TEXT("TMOP main menu could not create %d local players: %s"),
            LocalPlayerCount, *StartupStatus.ToString());
        return;
    }
    // Revalidate all selected players immediately before intro/travel, not just
    // the last player's UI button. Never permit a missing/disconnected pawn.
    if (bArrivedFromAppearanceTravel) LocalSession->RefreshAppearances();
    for (int32 Slot=0; Slot<LocalPlayerCount; ++Slot)
    {
        auto* Player = Cast<ATMOPPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, Slot));
        auto* Appearance = ATMOPPlayerAppearanceDirector::ForCharacter(Player);
        FTMOPPersonProfileRow Profile;
        FString Error;
        if (!Appearance || !Appearance->GetEditableProfile(Profile) || !Appearance->PreviewProfile(Profile, Error))
        {
            bNewGameRequested = false;
            bStartQueued = false;
            AppearanceLobby.Edit(Slot);
            StartupStatus = FText::FromString(Error.IsEmpty() ? TEXT("En spelares utseende kunde inte valideras.") : Error);
            if (MainMenuWidget) { bAppearanceSetupActive = true; MainMenuWidget->ShowAppearanceSetup(LocalPlayerCount); }
            PauseAppearanceWorld();
            SetMenuInput(true);
            return;
        }
    }
    if (!bArrivedFromAppearanceTravel && !GameplayLevel.IsNull() &&
        UGameplayStatics::GetCurrentLevelName(this, true) != FPackageName::GetShortName(GameplayLevel.ToSoftObjectPath().GetLongPackageName()))
    {
        const FString Package = GameplayLevel.ToSoftObjectPath().GetLongPackageName();
        if (!FPackageName::DoesPackageExist(Package))
        {
            bNewGameRequested = false;
            bStartQueued = false;
            StartupStatus = FText::FromString(TEXT("Spelnivån saknas. Kontrollera Gameplay Level och att kartan är paketerad."));
            return;
        }
        LocalSession->QueueAppearanceTravel(FName(*Package));
        if (!ReleaseAppearanceWorldPause()) { bNewGameRequested = false; return; }
        bAppearanceTravelPending = true;
        // This is the only new map-load path, strictly behind the ready barrier.
        if (MainMenuWidget) MainMenuWidget->HideAppearanceSetup();
        UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameplayLevel);
        return;
    }
    if (!ReleaseAppearanceWorldPause()) { bNewGameRequested = false; return; }
    bAppearanceSetupActive = false;
    bStartQueued = false;
    if (IsValid(MainMenuWidget))
    {
        MainMenuWidget->HideAppearanceSetup();
        MainMenuWidget->SetMenuMode(false);
        MainMenuWidget->SetIntroControlsVisible(bEnableIntro);
    }
    // Keep UI input active while the intro is running so the bottom-right
    // SKIP button can be clicked. FinishIntro restores normal game input.
    SetMenuInput(bEnableIntro);
    if (ATMOPPlayerCharacter* Player = GetPlayerCharacter())
    {
        Player->SetGameplayHUDHidden(TEXT("Cinematic"), bEnableIntro);
        Player->SetGameplayHUDHidden(TEXT("MainMenu"), false);
    }
    if (!bEnableIntro || !SpawnAndStartIntro()) FinishIntro();
}

bool ATMOPMainMenuIntroDirector::SpawnAndStartIntro()
{
    ActiveIntroDestinationAnchorId = IntroDestinationAnchorId;
    ATMOPPlayerCharacter* Player = GetPlayerCharacter();
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;

    const FTMOPHistoricalVehicleRow* SelectedRow = nullptr;
    const FTMOPHistoricalVehicleTimelineEntry* SelectedRouteEntry = nullptr;
    int32 SelectedRouteIndex = INDEX_NONE;
    if (IntroRouteSource == ETMOPIntroRouteSource::VehicleEditorTimeline)
    {
        UDataTable* VehicleTable = IntroVehicleTable;
        if (!IsValid(VehicleTable))
            for (TActorIterator<ATMOPHistoricalVehicleDirector> It(GetWorld()); It; ++It)
                if (IsValid(It->HistoricalVehicleTable))
                { VehicleTable = It->HistoricalVehicleTable; break; }
        SelectedRow = FindIntroVehicleRow(VehicleTable, IntroRouteVehicleId);
        if (SelectedRow != nullptr)
            for (int32 Index = 0; Index < SelectedRow->Timeline.Num(); ++Index)
            {
                const FTMOPHistoricalVehicleTimelineEntry& Candidate =
                    SelectedRow->Timeline[Index];
                if (IsDrivingEntry(Candidate) &&
                    (IntroRouteEntryId.IsNone() || Candidate.EntryId == IntroRouteEntryId))
                {
                    SelectedRouteEntry = &Candidate;
                    SelectedRouteIndex = Index;
                    break;
                }
            }
        if ((SelectedRow == nullptr || SelectedRouteEntry == nullptr) &&
            !bFallbackToAutomaticIntroRoute)
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP intro: vehicle '%s' or driving entry '%s' was not found."),
                *IntroRouteVehicleId.ToString(), *IntroRouteEntryId.ToString());
            return false;
        }
        if (SelectedRouteEntry == nullptr)
            UE_LOG(LogTemp, Warning, TEXT("TMOP intro: timeline route was not found; using automatic anchors."));
    }

    FName ResolvedStartAnchorId = IntroStartAnchorId;
    FName ResolvedDestinationAnchorId = IntroDestinationAnchorId;
    if (SelectedRow != nullptr && SelectedRouteEntry != nullptr)
    {
        ResolvedStartAnchorId = !SelectedRouteEntry->RouteStartAnchorId.IsNone()
            ? SelectedRouteEntry->RouteStartAnchorId
            : FindPlacementAnchorBefore(*SelectedRow, SelectedRouteIndex);
        ResolvedDestinationAnchorId = !SelectedRouteEntry->RouteDestinationAnchorId.IsNone()
            ? SelectedRouteEntry->RouteDestinationAnchorId
            : FindPlacementAnchorAfter(*SelectedRow, SelectedRouteIndex);
        if (ResolvedStartAnchorId.IsNone()) ResolvedStartAnchorId = IntroStartAnchorId;
        if (ResolvedDestinationAnchorId.IsNone())
            ResolvedDestinationAnchorId = IntroDestinationAnchorId;
    }
    ATMOPHistoricalAnchor* StartAnchor = Anchors != nullptr
        ? Anchors->FindAnchor(ResolvedStartAnchorId) : nullptr;
    ATMOPHistoricalAnchor* DestinationAnchor = Anchors != nullptr
        ? Anchors->FindAnchor(ResolvedDestinationAnchorId) : nullptr;
    if (!IsValid(Player) || !IsValid(StartAnchor) || !IsValid(DestinationAnchor) ||
        !IsValid(Network) || !IntroVehicleClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP intro: player, vehicle class, start or destination anchor is missing."));
        return false;
    }

    IntroVehicle = GetWorld()->SpawnActorDeferred<ATMOPConfiguredVehicle>(
        IntroVehicleClass, StartAnchor->GetActorTransform(), this, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if (!IsValid(IntroVehicle)) return false;
    IntroVehicle->VehicleId = TEXT("INTRO_JAN_NILSSON_LIMO");
    IntroVehicle->DisplayName = NSLOCTEXT("TMOP", "IntroLimoName", "Jan Nilssons limousine");
    IntroVehicle->VehicleModel = bInheritIntroVehicleAppearance && SelectedRow != nullptr &&
        IsValid(SelectedRow->ModelData) ? SelectedRow->ModelData : IntroVehicleModel;
    if (bInheritIntroVehicleAppearance && SelectedRow != nullptr)
    {
        IntroVehicle->bOverrideBodyColor = SelectedRow->bOverrideBodyColor;
        IntroVehicle->BodyColor = SelectedRow->BodyColor;
        IntroVehicle->RoofAccessory = SelectedRow->RoofAccessory;
    }
    IntroVehicle->bShowNameLabel = false;
    UGameplayStatics::FinishSpawningActor(IntroVehicle, StartAnchor->GetActorTransform());
    IntroVehicle->ApplyConfiguration();

    UTMOPTrafficVehicleMovementComponent* Movement = IntroVehicle->TrafficMovement;
    FName StartLane, DestinationLane;
    float StartDistance = 0.0f, DestinationDistance = 0.0f;
    TArray<FName> Route;
    bool bRouteResolved = false;
    if (IsValid(Movement) && SelectedRouteEntry != nullptr &&
        !SelectedRouteEntry->OrderedLaneIds.IsEmpty())
    {
        Route = SelectedRouteEntry->OrderedLaneIds;
        StartLane = !SelectedRouteEntry->RouteStartLaneId.IsNone()
            ? SelectedRouteEntry->RouteStartLaneId : Route[0];
        DestinationLane = !SelectedRouteEntry->RouteDestinationLaneId.IsNone()
            ? SelectedRouteEntry->RouteDestinationLaneId : Route.Last();
        UTMOPTrafficLaneComponent* StartLaneComponent = Network->FindLane(StartLane);
        UTMOPTrafficLaneComponent* DestinationLaneComponent = Network->FindLane(DestinationLane);
        bRouteResolved = IsValid(StartLaneComponent) && IsValid(DestinationLaneComponent);
        if (bRouteResolved)
        {
            if (Route[0] != StartLane) Route.Insert(StartLane, 0);
            if (Route.Last() != DestinationLane) Route.Add(DestinationLane);
            StartDistance = DistanceOnLane(StartLaneComponent, StartAnchor->GetActorLocation());
            DestinationDistance = DistanceOnLane(DestinationLaneComponent,
                DestinationAnchor->GetActorLocation());
        }
    }
    if (!bRouteResolved && (SelectedRouteEntry == nullptr || bFallbackToAutomaticIntroRoute))
        bRouteResolved = IsValid(Movement) &&
            Network->FindNearestLane(StartAnchor->GetActorLocation(), StartLane, StartDistance) &&
            Network->FindNearestReachableLane(DestinationAnchor->GetActorLocation(),
                StartLane, DestinationLane, DestinationDistance, Route);
    if (!bRouteResolved)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP intro: no reachable traffic route from '%s' to '%s'."),
            *ResolvedStartAnchorId.ToString(), *ResolvedDestinationAnchorId.ToString());
        IntroVehicle->Destroy(); IntroVehicle = nullptr;
        return false;
    }
    Movement->PlannedLaneIds = Route;
    Movement->SpeedLimitMultiplier = IntroVehicleSpeedMultiplier;
    if (SelectedRouteEntry != nullptr)
    {
        Movement->DesiredCruiseSpeedKmh = SelectedRouteEntry->CruiseSpeedOverrideKmh;
        Movement->bIgnoreOneWayRestrictions = SelectedRouteEntry->bIgnoreOneWayRestrictions;
        Movement->bRunRedLights = SelectedRouteEntry->bRunRedLights;
    }
    Movement->bDespawnAtRouteEnd = false;
    if (!Movement->InitializeOnLane(StartLane, StartDistance)) return false;
    Movement->ConfigureFinalApproach(DestinationLane, DestinationDistance,
        DestinationAnchor->GetActorTransform());
    ActiveIntroDestinationAnchorId = ResolvedDestinationAnchorId;

    if (IntroDriverClass)
    {
        IntroDriver = GetWorld()->SpawnActor<ATMOPHistoricalAgent>(IntroDriverClass,
            StartAnchor->GetActorTransform());
        if (IsValid(IntroDriver))
        {
            IntroDriver->DisplayName = IntroDriverDisplayName;
            IntroDriver->bShowNameLabel = false;
            if (IsValid(IntroDriver->EntityIdentity))
                IntroDriver->EntityIdentity->SetEntityIdentity(IntroDriverEntityId, TEXT("Person"));
            IntroDriver->RefreshNameLabel();
            IntroVehicle->EnterDriverSeat(IntroDriver);
        }
    }

    if (!IsValid(Player->VehicleSession) ||
        Player->VehicleSession->EnterVehicle(IntroVehicle, false) ==
            ETMOPVehicleTakeoverResult::FailedInternal)
        UE_LOG(LogTemp, Warning, TEXT("TMOP intro: player could not enter the passenger seat."));

    Movement->StartDriving();
    RuntimeFollowCamera = GetWorld()->SpawnActor<ACameraActor>();
    IntroElapsedSeconds = 0.0f;
    ActiveCameraShotIndex = INDEX_NONE;
    ActiveCardId = NAME_None;
    bIntroActive = true;
    if (!CameraShots.IsEmpty()) ApplyCameraShot(0);
    UpdateIntroCard();
    return true;
}

void ATMOPMainMenuIntroDirector::UpdateIntro(const float DeltaSeconds)
{
    IntroElapsedSeconds += DeltaSeconds;
    if (IntroElapsedSeconds >= FMath::Max(10.0f, IntroTimeoutSeconds))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP intro exceeded %.1f seconds; completing start flow."),
            IntroTimeoutSeconds);
        FinishIntro();
        return;
    }
    for (int32 Index = CameraShots.Num() - 1; Index >= 0; --Index)
        if (IntroElapsedSeconds >= CameraShots[Index].StartSeconds)
        { if (Index != ActiveCameraShotIndex) ApplyCameraShot(Index); break; }
    UpdateFollowCamera();
    UpdateIntroCard();
    if (!IsValid(IntroVehicle)) { FinishIntro(); return; }
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    ATMOPHistoricalAnchor* Destination = Anchors != nullptr
        ? Anchors->FindAnchor(ActiveIntroDestinationAnchorId) : nullptr;
    const bool bArrived = IsValid(Destination) &&
        FVector::DistSquared2D(IntroVehicle->GetActorLocation(),
            Destination->GetActorLocation()) <= FMath::Square(DestinationAcceptanceRadiusCm);
    const bool bRouteComplete = IsValid(IntroVehicle->TrafficMovement) &&
        IntroVehicle->TrafficMovement->TrafficState == ETMOPTrafficVehicleState::RouteComplete;
    if (bArrived || bRouteComplete) FinishIntro();
}

void ATMOPMainMenuIntroDirector::ApplyCameraShot(const int32 Index)
{
    if (!CameraShots.IsValidIndex(Index)) return;
    ActiveCameraShotIndex = Index;
    const FTMOPIntroCameraShot& Shot = CameraShots[Index];
    UpdateFollowCamera();
    ACameraActor* Target = Shot.bFollowIntroVehicle || Shot.bEnableHandheldMotion
        ? RuntimeFollowCamera.Get() : Shot.PlacedCamera.Get();
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
        if (IsValid(Target)) Controller->SetViewTargetWithBlend(Target, Shot.BlendSeconds);
}

void ATMOPMainMenuIntroDirector::UpdateFollowCamera()
{
    if (!CameraShots.IsValidIndex(ActiveCameraShotIndex) ||
        !IsValid(RuntimeFollowCamera)) return;
    const FTMOPIntroCameraShot& Shot = CameraShots[ActiveCameraShotIndex];
    if (!Shot.bFollowIntroVehicle && !Shot.bEnableHandheldMotion) return;
    if (Shot.bFollowIntroVehicle && !IsValid(IntroVehicle)) return;
    if (!Shot.bFollowIntroVehicle && !IsValid(Shot.PlacedCamera)) return;

    FTransform CameraTransform = Shot.bFollowIntroVehicle
        ? Shot.VehicleRelativeTransform * IntroVehicle->GetActorTransform()
        : Shot.PlacedCamera->GetActorTransform();
    if (Shot.bFollowIntroVehicle && Shot.bLookAtVehicle)
        CameraTransform.SetRotation((IntroVehicle->GetActorLocation() -
            CameraTransform.GetLocation()).Rotation().Quaternion());

    if (Shot.bEnableHandheldMotion)
    {
        const float LocalSeconds = FMath::Max(0.0f,
            IntroElapsedSeconds - Shot.StartSeconds);
        const float T = LocalSeconds * FMath::Max(0.01f, Shot.HandheldFrequencyHz) +
            static_cast<float>(Shot.HandheldSeed) * 0.137f;
        const FVector LocalOffset(
            FMath::PerlinNoise1D(T) * Shot.HandheldLocationAmplitude.X,
            FMath::PerlinNoise1D(T + 11.7f) * Shot.HandheldLocationAmplitude.Y,
            FMath::PerlinNoise1D(T + 23.4f) * Shot.HandheldLocationAmplitude.Z);
        CameraTransform.AddToTranslation(
            CameraTransform.GetRotation().RotateVector(LocalOffset));
        FRotator Rotation = CameraTransform.Rotator();
        Rotation.Pitch += FMath::PerlinNoise1D(T + 37.1f) *
            Shot.HandheldRotationAmplitude.Pitch;
        Rotation.Yaw += FMath::PerlinNoise1D(T + 49.8f) *
            Shot.HandheldRotationAmplitude.Yaw;
        Rotation.Roll += FMath::PerlinNoise1D(T + 61.5f) *
            Shot.HandheldRotationAmplitude.Roll;
        CameraTransform.SetRotation(Rotation.Quaternion());
    }
    RuntimeFollowCamera->SetActorTransform(CameraTransform);
    if (!Shot.bFollowIntroVehicle && IsValid(Shot.PlacedCamera->GetCameraComponent()) &&
        IsValid(RuntimeFollowCamera->GetCameraComponent()))
    {
        UCameraComponent* SourceCamera = Shot.PlacedCamera->GetCameraComponent();
        UCameraComponent* RuntimeCamera = RuntimeFollowCamera->GetCameraComponent();
        RuntimeCamera->SetFieldOfView(SourceCamera->FieldOfView);
        RuntimeCamera->SetAspectRatio(SourceCamera->AspectRatio);
        RuntimeCamera->SetConstraintAspectRatio(SourceCamera->bConstrainAspectRatio);
        RuntimeCamera->PostProcessSettings = SourceCamera->PostProcessSettings;
        RuntimeCamera->PostProcessBlendWeight = SourceCamera->PostProcessBlendWeight;
    }
}

void ATMOPMainMenuIntroDirector::UpdateIntroCard()
{
    if (!IsValid(MainMenuWidget) || !IsValid(IntroCardsTable)) return;
    if (IntroCardsTable->GetRowStruct() != FTMOPIntroPresentationCard::StaticStruct())
        return;
    const FTMOPIntroPresentationCard* Active = nullptr;
    FName NewId = NAME_None;
    float LatestStartSeconds = -1.0f;
    for (const TPair<FName, uint8*>& Pair : IntroCardsTable->GetRowMap())
    {
        const FTMOPIntroPresentationCard* Card =
            reinterpret_cast<const FTMOPIntroPresentationCard*>(Pair.Value);
        if (Card != nullptr && IntroElapsedSeconds >= Card->StartSeconds &&
            IntroElapsedSeconds < Card->EndSeconds &&
            Card->StartSeconds >= LatestStartSeconds)
        {
            Active = Card;
            LatestStartSeconds = Card->StartSeconds;
            NewId = Card->CardId.IsNone() ? Pair.Key : Card->CardId;
        }
    }
    if (NewId == ActiveCardId) return;
    ActiveCardId = NewId;
    MainMenuWidget->SetIntroCard(Active != nullptr ? Active->Heading : FText(),
        Active != nullptr ? Active->Body : FText(),
        Active != nullptr ? Active->Image.LoadSynchronous() : nullptr,
        Active != nullptr);
}

void ATMOPMainMenuIntroDirector::FinishIntro()
{
    if ((bEnableMainMenu || bArrivedFromAppearanceTravel) && !AppearanceLobby.AllReady()) return;
    bIntroActive = false;
    if (IsValid(MainMenuWidget)) MainMenuWidget->SetIntroControlsVisible(false);
    ATMOPPlayerCharacter* Player = GetPlayerCharacter();
    if (IsValid(Player) && IsValid(Player->VehicleSession) &&
        Player->VehicleSession->IsInVehicle()) Player->VehicleSession->ExitVehicle();
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    ATMOPHistoricalAnchor* Destination = Anchors != nullptr
        ? Anchors->FindAnchor(ActiveIntroDestinationAnchorId) : nullptr;
    if (IsValid(Player) && IsValid(Destination))
        Player->SetActorTransform(Destination->GetActorTransform(), false, nullptr,
            ETeleportType::TeleportPhysics);
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
        if (IsValid(Player)) Controller->SetViewTargetWithBlend(Player, 0.5f);
    if (IsValid(IntroDriver)) IntroDriver->Destroy();
    if (IsValid(IntroVehicle)) IntroVehicle->Destroy();
    if (IsValid(RuntimeFollowCamera)) RuntimeFollowCamera->Destroy();
    IntroDriver = nullptr; IntroVehicle = nullptr; RuntimeFollowCamera = nullptr;
    auto* LocalSession = GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
    const FName PartyAnchor = PlayerStartAnchorId.IsNone() ? ActiveIntroDestinationAnchorId : PlayerStartAnchorId;
    if (!LocalSession->StartParty(PartyAnchor,
        Player ? Player->GetActorTransform() : GetActorTransform(), StartupStatus))
    {
        bNewGameRequested = false;
        if (IsValid(MainMenuWidget)) MainMenuWidget->SetMenuMode(true);
        if (Player)
        {
            Player->SetGameplayHUDHidden(TEXT("Cinematic"), false);
            Player->SetGameplayHUDHidden(TEXT("MainMenu"), true);
        }
        SetMenuInput(true);
        return;
    }
    if (IsValid(MainMenuWidget)) MainMenuWidget->RemoveFromParent();
    SetMenuInput(false);
    for (ATMOPPlayerCharacter* PartyPlayer : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
    {
        PartyPlayer->SetGameplayHUDHidden(TEXT("MainMenu"), false);
        PartyPlayer->SetGameplayHUDHidden(TEXT("Cinematic"), false);
        PartyPlayer->ApplyLocalInputMode();
    }
    if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
    { Clock->SetCurrentTime(GameStartTime); Clock->StartClock(); }
}

void ATMOPMainMenuIntroDirector::SkipIntro()
{ if (bIntroActive) FinishIntro(); }

void ATMOPMainMenuIntroDirector::LoadGame()
{
    if (IsValid(MainMenuWidget)) MainMenuWidget->SetLoadMenuMode(true);
}

bool ATMOPMainMenuIntroDirector::LoadGameSlot(const FString& SlotName)
{
    ATMOPPlayerCharacter* Player = GetPlayerCharacter();
    FText Status;
    if (!IsValid(Player) || !FTMOPSaveGameService::LoadPlayer(
        GetWorld(), Player, SlotName, Status))
    {
        if (IsValid(MainMenuWidget)) MainMenuWidget->SetLoadStatus(Status);
        UE_LOG(LogTemp, Error, TEXT("TMOP main menu load failed: %s"), *Status.ToString());
        return false;
    }
    if (IsValid(MainMenuWidget)) MainMenuWidget->RemoveFromParent();
    SetMenuInput(false);
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
        Controller->SetViewTarget(Player);
    Player->SetGameplayHUDHidden(TEXT("MainMenu"), false);
    Player->SetGameplayHUDHidden(TEXT("Cinematic"), false);
    for (ATMOPPlayerCharacter* Member : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
    {
        Member->ApplyLocalInputMode();
        Member->RefreshLoopEndMenu();
    }
    if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
        Clock->StartClock();
    return true;
}

void ATMOPMainMenuIntroDirector::CloseLoadGameMenu()
{
    if (IsValid(MainMenuWidget)) MainMenuWidget->SetLoadMenuMode(false);
}

void ATMOPMainMenuIntroDirector::OpenSettings()
{
    if (ATMOPPlayerCharacter* Player = GetPlayerCharacter())
    {
        if (IsValid(MainMenuWidget)) MainMenuWidget->SetMenuMode(false);
        Player->SetPauseMenuOpen(true);
        if (IsValid(Player->PauseMenuWidget))
            Player->PauseMenuWidget->OpenSettingsPage();
        bWaitingForSettingsClose = true;
    }
}

void ATMOPMainMenuIntroDirector::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this,
        UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, false);
}
