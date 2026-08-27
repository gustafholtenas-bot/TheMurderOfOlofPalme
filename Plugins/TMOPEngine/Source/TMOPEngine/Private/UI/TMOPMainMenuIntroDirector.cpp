#include "UI/TMOPMainMenuIntroDirector.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Camera/CameraActor.h"
#include "Engine/DataTable.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "UI/TMOPMainMenuWidget.h"
#include "UI/TMOPPauseMenuWidget.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleModelData.h"

ATMOPMainMenuIntroDirector::ATMOPMainMenuIntroDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    IntroVehicleClass = ATMOPConfiguredVehicle::StaticClass();
    IntroDriverClass = ATMOPHistoricalAgent::StaticClass();
    MainMenuWidgetClass = UTMOPMainMenuWidget::StaticClass();
}

void ATMOPMainMenuIntroDirector::BeginPlay()
{
    Super::BeginPlay();
    if (!bEnableMainMenu)
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
    if (IsValid(MainMenuWidget)) MainMenuWidget->RemoveFromParent();
    if (IsValid(RuntimeFollowCamera)) RuntimeFollowCamera->Destroy();
    Super::EndPlay(EndPlayReason);
}

void ATMOPMainMenuIntroDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bInitialized) TryInitializeMenu();
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
    TSubclassOf<UTMOPMainMenuWidget> WidgetClass = MainMenuWidgetClass;
    if (!WidgetClass) WidgetClass = UTMOPMainMenuWidget::StaticClass();
    MainMenuWidget = CreateWidget<UTMOPMainMenuWidget>(Controller, WidgetClass);
    if (!IsValid(MainMenuWidget)) return;
    MainMenuWidget->InitializeMainMenu(this, GameLogo);
    MainMenuWidget->AddToViewport(1000);
    MainMenuWidget->SetMenuMode(true);
    if (IsValid(MainMenuBackgroundCamera))
        Controller->SetViewTarget(MainMenuBackgroundCamera);
    SetMenuInput(true);
    bInitialized = true;
}

void ATMOPMainMenuIntroDirector::SetMenuInput(const bool bMenuInput)
{
    APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
    if (!IsValid(Controller)) return;
    Controller->bShowMouseCursor = bMenuInput;
    Controller->SetIgnoreMoveInput(bMenuInput);
    Controller->SetIgnoreLookInput(bMenuInput);
    if (bMenuInput && IsValid(MainMenuWidget))
    {
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Controller->SetInputMode(Mode);
    }
    else Controller->SetInputMode(FInputModeGameOnly());
}

void ATMOPMainMenuIntroDirector::StartNewGame()
{
    if (IsValid(MainMenuWidget)) MainMenuWidget->SetMenuMode(false);
    SetMenuInput(false);
    if (!bEnableIntro || !SpawnAndStartIntro()) FinishIntro();
}

bool ATMOPMainMenuIntroDirector::SpawnAndStartIntro()
{
    ATMOPPlayerCharacter* Player = GetPlayerCharacter();
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    ATMOPHistoricalAnchor* StartAnchor = Anchors != nullptr
        ? Anchors->FindAnchor(IntroStartAnchorId) : nullptr;
    ATMOPHistoricalAnchor* DestinationAnchor = Anchors != nullptr
        ? Anchors->FindAnchor(IntroDestinationAnchorId) : nullptr;
    if (!IsValid(Player) || !IsValid(StartAnchor) || !IsValid(DestinationAnchor) ||
        !IntroVehicleClass)
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
    IntroVehicle->VehicleModel = IntroVehicleModel;
    IntroVehicle->bShowNameLabel = false;
    UGameplayStatics::FinishSpawningActor(IntroVehicle, StartAnchor->GetActorTransform());
    IntroVehicle->ApplyConfiguration();

    UTMOPTrafficVehicleMovementComponent* Movement = IntroVehicle->TrafficMovement;
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    FName StartLane, DestinationLane;
    float StartDistance = 0.0f, DestinationDistance = 0.0f;
    TArray<FName> Route;
    if (!IsValid(Movement) || !IsValid(Network) ||
        !Network->FindNearestLane(StartAnchor->GetActorLocation(), StartLane, StartDistance) ||
        !Network->FindNearestReachableLane(DestinationAnchor->GetActorLocation(),
            StartLane, DestinationLane, DestinationDistance, Route))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP intro: no reachable traffic route from '%s' to '%s'."),
            *IntroStartAnchorId.ToString(), *IntroDestinationAnchorId.ToString());
        IntroVehicle->Destroy(); IntroVehicle = nullptr;
        return false;
    }
    Movement->PlannedLaneIds = Route;
    Movement->SpeedLimitMultiplier = IntroVehicleSpeedMultiplier;
    Movement->bDespawnAtRouteEnd = false;
    if (!Movement->InitializeOnLane(StartLane, StartDistance)) return false;
    Movement->ConfigureFinalApproach(DestinationLane, DestinationDistance,
        DestinationAnchor->GetActorTransform());

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
    for (int32 Index = CameraShots.Num() - 1; Index >= 0; --Index)
        if (IntroElapsedSeconds >= CameraShots[Index].StartSeconds)
        { if (Index != ActiveCameraShotIndex) ApplyCameraShot(Index); break; }
    UpdateFollowCamera();
    UpdateIntroCard();
    if (!IsValid(IntroVehicle)) { FinishIntro(); return; }
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    ATMOPHistoricalAnchor* Destination = Anchors != nullptr
        ? Anchors->FindAnchor(IntroDestinationAnchorId) : nullptr;
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
    ACameraActor* Target = Shot.bFollowIntroVehicle
        ? RuntimeFollowCamera.Get() : Shot.PlacedCamera.Get();
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
        if (IsValid(Target)) Controller->SetViewTargetWithBlend(Target, Shot.BlendSeconds);
    UpdateFollowCamera();
}

void ATMOPMainMenuIntroDirector::UpdateFollowCamera()
{
    if (!CameraShots.IsValidIndex(ActiveCameraShotIndex) ||
        !IsValid(RuntimeFollowCamera) || !IsValid(IntroVehicle)) return;
    const FTMOPIntroCameraShot& Shot = CameraShots[ActiveCameraShotIndex];
    if (!Shot.bFollowIntroVehicle) return;
    const FTransform WorldTransform = Shot.VehicleRelativeTransform * IntroVehicle->GetActorTransform();
    RuntimeFollowCamera->SetActorTransform(WorldTransform);
    if (Shot.bLookAtVehicle)
        RuntimeFollowCamera->SetActorRotation((IntroVehicle->GetActorLocation() -
            RuntimeFollowCamera->GetActorLocation()).Rotation());
}

void ATMOPMainMenuIntroDirector::UpdateIntroCard()
{
    if (!IsValid(MainMenuWidget) || !IsValid(IntroCardsTable)) return;
    const FTMOPIntroPresentationCard* Active = nullptr;
    for (const TPair<FName, uint8*>& Pair : IntroCardsTable->GetRowMap())
    {
        const FTMOPIntroPresentationCard* Card =
            reinterpret_cast<const FTMOPIntroPresentationCard*>(Pair.Value);
        if (Card != nullptr && IntroElapsedSeconds >= Card->StartSeconds &&
            IntroElapsedSeconds < Card->EndSeconds) { Active = Card; break; }
    }
    const FName NewId = Active != nullptr ? Active->CardId : NAME_None;
    if (NewId == ActiveCardId) return;
    ActiveCardId = NewId;
    MainMenuWidget->SetIntroCard(Active != nullptr ? Active->Heading : FText(),
        Active != nullptr ? Active->Body : FText(),
        Active != nullptr ? Active->Image.LoadSynchronous() : nullptr,
        Active != nullptr);
}

void ATMOPMainMenuIntroDirector::FinishIntro()
{
    bIntroActive = false;
    ATMOPPlayerCharacter* Player = GetPlayerCharacter();
    if (IsValid(Player) && IsValid(Player->VehicleSession) &&
        Player->VehicleSession->IsInVehicle()) Player->VehicleSession->ExitVehicle();
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    ATMOPHistoricalAnchor* Destination = Anchors != nullptr
        ? Anchors->FindAnchor(IntroDestinationAnchorId) : nullptr;
    if (IsValid(Player) && IsValid(Destination))
        Player->SetActorTransform(Destination->GetActorTransform(), false, nullptr,
            ETeleportType::TeleportPhysics);
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
        if (IsValid(Player)) Controller->SetViewTargetWithBlend(Player, 0.5f);
    if (IsValid(IntroDriver)) IntroDriver->Destroy();
    if (IsValid(IntroVehicle)) IntroVehicle->Destroy();
    if (IsValid(RuntimeFollowCamera)) RuntimeFollowCamera->Destroy();
    IntroDriver = nullptr; IntroVehicle = nullptr; RuntimeFollowCamera = nullptr;
    if (IsValid(MainMenuWidget)) MainMenuWidget->RemoveFromParent();
    SetMenuInput(false);
    if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
    { Clock->SetCurrentTime(GameStartTime); Clock->StartClock(); }
}

void ATMOPMainMenuIntroDirector::SkipIntro()
{ if (bIntroActive) FinishIntro(); }

void ATMOPMainMenuIntroDirector::LoadGame()
{
    ATMOPPlayerCharacter* Player = GetPlayerCharacter();
    if (!IsValid(Player) || !IsValid(Player->PauseMenuWidget)) return;
    if (Player->PauseMenuWidget->LoadQuickSave(SaveSlotName))
    {
        if (IsValid(MainMenuWidget)) MainMenuWidget->RemoveFromParent();
        SetMenuInput(false);
        if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
            Controller->SetViewTarget(Player);
        if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
            Clock->StartClock();
    }
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
