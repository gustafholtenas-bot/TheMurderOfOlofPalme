#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPLocalSessionPolicy.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/UnrealTemplate.h"
#include "People/TMOPPlayerAppearanceDirector.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Player/TMOPControlSettingsSubsystem.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "UI/TMOPMainMenuIntroDirector.h"
#include "Misc/PackageName.h"

void UTMOPLocalMultiplayerSubsystem::QueueAppearanceTravel(FName LevelPackage)
{
    PendingAppearanceLevel = LevelPackage;
    if (!LevelPackage.IsNone())
        AppearanceMainMenuLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
}

bool UTMOPLocalMultiplayerSubsystem::ConsumeAppearanceTravel(UWorld* World)
{
    if (!World || PendingAppearanceLevel.IsNone() ||
        UGameplayStatics::GetCurrentLevelName(World, true) != FPackageName::GetShortName(PendingAppearanceLevel.ToString())) return false;
    PendingAppearanceLevel = NAME_None;
    return true;
}

void UTMOPLocalMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Collection.InitializeDependency<UTMOPClockSubsystem>();
    GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->OnLoopRestarted.AddUniqueDynamic(
        this, &UTMOPLocalMultiplayerSubsystem::HandleLoopRestarted);
    bSavedSplitScreenSetting = GetDefault<UGameMapsSettings>()->bUseSplitscreen;
    bSavedGamepadOffset = GetDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds;
}

void UTMOPLocalMultiplayerSubsystem::Deinitialize()
{
    if (UTMOPClockSubsystem* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
        Clock->OnLoopRestarted.RemoveDynamic(this, &UTMOPLocalMultiplayerSubsystem::HandleLoopRestarted);
    GetMutableDefault<UGameMapsSettings>()->bUseSplitscreen = bSavedSplitScreenSetting;
    GetMutableDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds = bSavedGamepadOffset;
    Super::Deinitialize();
}

TArray<ATMOPPlayerCharacter*> UTMOPLocalMultiplayerSubsystem::GetPlayers(const UObject* Context)
{
    TArray<ATMOPPlayerCharacter*> Result;
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
    if (!GI) return Result;
    for (ULocalPlayer* Local : GI->GetLocalPlayers())
        if (APlayerController* PC = Local ? Local->GetPlayerController(World) : nullptr)
            if (ATMOPPlayerCharacter* Player = Cast<ATMOPPlayerCharacter>(PC->GetPawn()))
                Result.Add(Player);
    return Result;
}

bool UTMOPLocalMultiplayerSubsystem::IsMultiplayer(const UObject* Context)
{
    const UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
    return GI && GI->GetNumLocalPlayers() > 1;
}

int32 UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(const ATMOPPlayerCharacter* Player)
{
    const APlayerController* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
    const UGameInstance* GI = Player ? Player->GetGameInstance() : nullptr;
    return PC && GI ? GI->GetLocalPlayers().IndexOfByKey(PC->GetLocalPlayer()) : INDEX_NONE;
}

APlayerCameraManager* UTMOPLocalMultiplayerSubsystem::FindNearestCamera(
    const UObject* Context, const FVector& Location)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    if (!World) return nullptr;
    APlayerCameraManager* Best = nullptr;
    double Distance = TNumericLimits<double>::Max();
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !PC->IsLocalController() || !IsValid(PC->PlayerCameraManager)) continue;
        const double Candidate = FVector::DistSquared(PC->PlayerCameraManager->GetCameraLocation(), Location);
        if (Candidate < Distance) { Best = PC->PlayerCameraManager; Distance = Candidate; }
    }
    return Best;
}

void UTMOPLocalMultiplayerSubsystem::GetPlayerViewRect(
    APlayerController* PC, FVector2D& Origin, FVector2D& Size)
{
    Origin = FVector2D::ZeroVector;
    int32 Width = 0, Height = 0;
    if (PC) PC->GetViewportSize(Width, Height);
    Size = FVector2D(Width, Height);
    if (ULocalPlayer* Local = PC ? PC->GetLocalPlayer() : nullptr)
    {
        Origin = Size * Local->Origin;
        Size *= Local->Size;
    }
}

void UTMOPLocalMultiplayerSubsystem::ConfigureSession(int32 PlayerCount,
    bool bKeyboardForPlayerOne, bool bSharedKeyboardForPlayerTwo)
{
    SelectedPlayerCount = FMath::Clamp(PlayerCount, 1, 4);
    bUseKeyboardForPlayerOne = bKeyboardForPlayerOne;
    bUseSharedKeyboardForPlayerTwo = bUseKeyboardForPlayerOne &&
        bSharedKeyboardForPlayerTwo && SelectedPlayerCount == 2;
    // Session-only: do not save over the user's project settings.
    GetMutableDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds =
        bUseKeyboardForPlayerOne && SelectedPlayerCount > 1;
    if (UTMOPControlSettingsSubsystem* Controls =
        GetGameInstance()->GetSubsystem<UTMOPControlSettingsSubsystem>())
        Controls->ConfigureForSession(SelectedPlayerCount, bUseKeyboardForPlayerOne,
            bUseSharedKeyboardForPlayerTwo);
}

void UTMOPLocalMultiplayerSubsystem::SetSplitScreenEnabled(bool bEnabled)
{
    GetMutableDefault<UGameMapsSettings>()->bUseSplitscreen = true;
    if (UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient())
    {
        Viewport->SetForceDisableSplitscreen(!bEnabled);
        Viewport->LayoutPlayers();
    }
}

void UTMOPLocalMultiplayerSubsystem::UpdateControlLayoutFromProfiles()
{
    if (auto* Controls = GetGameInstance()->GetSubsystem<UTMOPControlSettingsSubsystem>())
    {
        bUseKeyboardForPlayerOne = Controls->GetProfile(0).Device == ETMOPControlDevice::KeyboardMouse;
        bUseSharedKeyboardForPlayerTwo = SelectedPlayerCount == 2 &&
            Controls->GetProfile(1).Device == ETMOPControlDevice::KeyboardOnly;
        GetMutableDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds =
            bUseKeyboardForPlayerOne && SelectedPlayerCount > 1;
    }
}

bool UTMOPLocalMultiplayerSubsystem::EnsurePlayerCount(int32 Count, FText& OutError)
{
    UWorld* World = GetWorld();
    UGameInstance* GI = GetGameInstance();
    if (!World || World->GetNetMode() != NM_Standalone || !TMOPLocalSessionPolicy::IsValidPlayerCount(Count))
    {
        OutError = NSLOCTEXT("TMOP", "LocalOnly", "Starta som Standalone med 1–4 lokala spelare, inte som nätverksklient.");
        return false;
    }
    const TArray<ATMOPPlayerCharacter*> Before = GetPlayers(this);
    if (Before.IsEmpty() || GetPlayerSlot(Before[0]) != 0)
    {
        OutError = NSLOCTEXT("TMOP", "LocalMissingPlayer", "Spelare 1 måste vara BP_TMOPPlayerCharacter innan spelet startas.");
        return false;
    }
    const int32 OriginalCount = GI->GetNumLocalPlayers();
    while (GI->GetNumLocalPlayers() < Count)
    {
        APlayerController* PC = UGameplayStatics::CreatePlayer(World, -1, true);
        if (!PC) break;
        // Honour the actual configured BP player, including input and component defaults.
        if (!PC->GetPawn() || PC->GetPawn()->GetClass() != Before[0]->GetClass())
        {
            ATMOPPlayerCharacter* Pawn = World->SpawnActorDeferred<ATMOPPlayerCharacter>(
                Before[0]->GetClass(), Before[0]->GetActorTransform(), nullptr, nullptr,
                ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
            if (!Pawn) break;
            Pawn->AutoPossessPlayer = EAutoReceiveInput::Disabled;
            Pawn->FinishSpawning(Before[0]->GetActorTransform());
            APawn* UnusedPawn = PC->GetPawn();
            PC->Possess(Pawn);
            if (UnusedPawn) UnusedPawn->Destroy();
        }
    }
    if (GetPlayers(this).Num() < Count)
    {
        while (GI->GetNumLocalPlayers() > OriginalCount)
            UGameplayStatics::RemovePlayer(GI->GetLocalPlayers().Last()->GetPlayerController(World), true);
        OutError = NSLOCTEXT("TMOP", "LocalSpawnFailed", "Kunde inte skapa alla spelare. Kontrollera GameMode och BP_TMOPPlayerCharacter.");
        return false;
    }
    while (GI->GetNumLocalPlayers() > Count)
        UGameplayStatics::RemovePlayer(GI->GetLocalPlayers().Last()->GetPlayerController(World), true);
    for (ATMOPPlayerCharacter* Player : GetPlayers(this))
        if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        {
            PC->PrimaryActorTick.bTickEvenWhenPaused = true;
        }
    return true;
}

bool UTMOPLocalMultiplayerSubsystem::PlaceParty(
    FName AnchorId, const FTransform& Fallback, FText& OutError)
{
    FTransform Centre = Fallback;
    if (!AnchorId.IsNone())
    {
        auto* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Anchor = Anchors ? Anchors->FindAnchor(AnchorId) : nullptr;
        if (!Anchor)
        {
            // An optional/moved anchor must not make multiplayer collapse back
            // to one player. The party leader's known transform is safe enough
            // for the normal grounded placement search below.
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP local multiplayer: start anchor '%s' is missing; using fallback transform."),
                *AnchorId.ToString());
        }
        else Centre = Anchor->GetActorTransform();
    }
    const TArray<ATMOPPlayerCharacter*> Players = GetPlayers(this);
    TArray<FVector> Positions;
    TArray<float> Radii;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TMOPPartySpawn), false);
    for (ATMOPPlayerCharacter* Player : Players) Query.AddIgnoredActor(Player);
    // Resolve every location before moving anyone. Never stack players or spawn below ground.
    for (ATMOPPlayerCharacter* Player : Players)
    {
        const UCapsuleComponent* Capsule = Player->GetCapsuleComponent();
        const float Radius = Capsule->GetScaledCapsuleRadius();
        const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        bool bFound = false;
        for (int32 Attempt = 0; Attempt < 65; ++Attempt)
        {
            const auto Candidate = TMOPLocalSessionPolicy::CandidateOffset(Attempt);
            const FVector Offset(Candidate.X, Candidate.Y, 0);
            FVector Point = Centre.GetLocation() + Centre.GetRotation().RotateVector(Offset);
            FHitResult Hit;
            if (!GetWorld()->LineTraceSingleByChannel(Hit, Point + FVector(0,0,1000),
                Point - FVector(0,0,2000), ECC_Visibility, Query) || Hit.ImpactNormal.Z < 0.7f) continue;
            Point = Hit.ImpactPoint + FVector(0,0,HalfHeight + 3.0f);
            bool bOverlapsParty = false;
            for (int32 Index = 0; Index < Positions.Num(); ++Index)
                bOverlapsParty |= FVector::DistSquared2D(Point, Positions[Index]) <
                    FMath::Square(Radius + Radii[Index] + 30.0f);
            if (bOverlapsParty || GetWorld()->OverlapBlockingTestByProfile(Point, FQuat::Identity,
                Capsule->GetCollisionProfileName(), FCollisionShape::MakeCapsule(Radius, HalfHeight), Query)) continue;
            Positions.Add(Point); Radii.Add(Radius); bFound = true; break;
        }
        if (!bFound)
        {
            OutError = NSLOCTEXT("TMOP", "PartyNoFloor", "Det saknas tillräckligt med fria, marknära startplatser vid ankaret (sökradie 6 m).");
            return false;
        }
    }
    for (int32 Index = 0; Index < Players.Num(); ++Index)
    {
        ATMOPPlayerCharacter* Player = Players[Index];
        if (IsValid(Player->VehicleSession)) Player->VehicleSession->ExitVehicle();
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorLocationAndRotation(Positions[Index], Centre.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
        if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        {
            PC->SetControlRotation(Centre.Rotator());
            PC->SetViewTarget(Player);
        }
    }
    return true;
}

bool UTMOPLocalMultiplayerSubsystem::StartParty(FName AnchorId, const FTransform& Fallback, FText& OutError)
{
    if (bChangingSession) return false;
    TGuardValue<bool> Guard(bChangingSession, true);
    if (!EnsurePlayerCount(SelectedPlayerCount, OutError))
    {
        SetSplitScreenEnabled(false);
        return false;
    }
    if (!PlaceParty(AnchorId, Fallback, OutError))
    {
        // Keep the selected local players alive so a placement/configuration
        // error never silently turns a 2–4 player selection into one player.
        SetSplitScreenEnabled(false);
        return false;
    }
    StartAnchorId = AnchorId; StartFallback = Fallback; SessionWorld = GetWorld(); bSessionStarted = true;
    SetSplitScreenEnabled(true);
    RefreshAppearances();
    return true;
}

void UTMOPLocalMultiplayerSubsystem::RefreshAppearances()
{
    TArray<ATMOPPlayerAppearanceDirector*> Directors;
    ATMOPPlayerAppearanceDirector* Master = nullptr;
    for (TActorIterator<ATMOPPlayerAppearanceDirector> It(GetWorld()); It; ++It)
    {
        Directors.Add(*It);
        if (It->PlayerIndex == 0 && !It->Tags.Contains(TEXT("TMOP_RuntimePlayerAppearance"))) Master = *It;
    }
    for (ATMOPPlayerCharacter* Player : GetPlayers(this))
    {
        const int32 Slot = GetPlayerSlot(Player);
        ATMOPPlayerAppearanceDirector* Chosen = nullptr;
        for (ATMOPPlayerAppearanceDirector* Director : Directors)
            if (Director->PlayerIndex == Slot && !Director->Tags.Contains(TEXT("TMOP_RuntimePlayerAppearance")))
            { Chosen = Director; break; }
        if (!Chosen)
            for (ATMOPPlayerAppearanceDirector* Director : Directors)
                if (Director->PlayerIndex == Slot) { Chosen = Director; break; }
        if (!Chosen && Master)
        {
            Chosen = GetWorld()->SpawnActorDeferred<ATMOPPlayerAppearanceDirector>(
                ATMOPPlayerAppearanceDirector::StaticClass(), FTransform::Identity);
            if (Chosen)
            {
                Chosen->bApplyOnBeginPlay = false;
                Chosen->Tags.Add(TEXT("TMOP_RuntimePlayerAppearance"));
                Chosen->ConfigureForLocalPlayer(Master, Player, Slot);
                Chosen->FinishSpawning(FTransform::Identity);
                Directors.Add(Chosen);
            }
        }
        if (!Chosen)
        {
            UE_LOG(LogTemp, Warning, TEXT("No PlayerAppearanceDirector for local player %d."), Slot + 1);
            continue;
        }
        if (Master && (Chosen == Master || Chosen->Tags.Contains(TEXT("TMOP_RuntimePlayerAppearance"))))
            Chosen->ConfigureForLocalPlayer(Master, Player, Slot);
        else Chosen->ConfigureForLocalPlayer(Chosen, Player, Slot);
        Chosen->RefreshPlayerAppearance();
    }
}

void UTMOPLocalMultiplayerSubsystem::CloseAllPlayerMenus()
{
    for (ATMOPPlayerCharacter* Player : GetPlayers(this)) Player->CloseSessionMenus();
}

void UTMOPLocalMultiplayerSubsystem::HandleLoopRestarted(int32 LoopNumber, FTMOPTime StartTime)
{
    if (!bRestartPartyWithClock || !bSessionStarted || SessionWorld.Get() != GetWorld()) return;
    CloseAllPlayerMenus();
    FText Error;
    if (!PlaceParty(StartAnchorId, StartFallback, Error))
    {
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->RequestPause(this, TEXT("SpawnFailed"));
        UE_LOG(LogTemp, Error, TEXT("TMOP party restart: %s"), *Error.ToString());
    }
    else GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->ReleasePause(this, TEXT("SpawnFailed"));
    RefreshAppearances();
}

void UTMOPLocalMultiplayerSubsystem::RestartFromBeginning()
{
    if (bChangingSession) return;
    TGuardValue<bool> Guard(bChangingSession, true);
    CloseAllPlayerMenus();
    // Vehicles may be destroyed by any OnLoopRestarted listener; detach first.
    for (ATMOPPlayerCharacter* Player : GetPlayers(this))
        if (Player->VehicleSession) Player->VehicleSession->ExitVehicle();
    TGuardValue<bool> RestartGuard(bRestartPartyWithClock, true);
    if (UTMOPClockSubsystem* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        Clock->RestartLoop();
        Clock->StartClock();
    }
}

void UTMOPLocalMultiplayerSubsystem::AdoptLoadedSession()
{
    SessionWorld = GetWorld(); bSessionStarted = true;
    StartAnchorId = NAME_None;
    const auto Players = GetPlayers(this);
    if (!Players.IsEmpty()) StartFallback = Players[0]->GetActorTransform();
    for (TActorIterator<ATMOPMainMenuIntroDirector> It(GetWorld()); It; ++It)
    {
        StartAnchorId = It->PlayerStartAnchorId.IsNone() ? It->IntroDestinationAnchorId : It->PlayerStartAnchorId;
        break;
    }
    SetSplitScreenEnabled(true);
    RefreshAppearances();
    for (ATMOPPlayerCharacter* Player : Players) Player->RefreshLoopEndMenu();
}

void UTMOPLocalMultiplayerSubsystem::PrepareMainMenu()
{
    bSessionStarted = false; SessionWorld.Reset();
    CloseAllPlayerMenus();
    FText Ignored;
    EnsurePlayerCount(1, Ignored);
    GetMutableDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds = false;
    SetSplitScreenEnabled(false);
    if (UTMOPClockSubsystem* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        Clock->PauseClock();
        Clock->ReleaseAllPauses(this);
        Clock->SetCurrentTime(Clock->GetLoopStartTime());
    }
}

void UTMOPLocalMultiplayerSubsystem::ReturnToMainMenu()
{
    if (bChangingSession || !GetWorld()) return;
    bChangingSession = true;
    const FName Level = AppearanceMainMenuLevel.IsNone()
        ? FName(*UGameplayStatics::GetCurrentLevelName(this, true)) : AppearanceMainMenuLevel;
    PrepareMainMenu();
    UGameplayStatics::OpenLevel(this, Level);
    bChangingSession = false;
}
