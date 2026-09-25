#include "Transit/TMOPBusArrivalBoardComponent.h"
#include "Localization/TMOPLocalization.h"

#include "Components/WidgetComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Time/TMOPClockSubsystem.h"
#include "UI/TMOPMetroBoardWidget.h"
#include "EngineUtils.h"
#include "Transit/TMOPBusScheduleDirector.h"
#include "Transit/TMOPBusRouteData.h"
#include "Transit/TMOPBusServiceComponent.h"
#include "Transit/TMOPBusStopSubsystem.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleBase.h"

UTMOPBusArrivalBoardComponent::UTMOPBusArrivalBoardComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UTMOPBusArrivalBoardComponent::ConfigureForStation(
    const FName NewStationId, const FText& NewStationName)
{
    StationId = NewStationId;
    StationName = NewStationName;
}

void UTMOPBusArrivalBoardComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->GetRootComponent() || StationId.IsNone()) return;

    EnsurePlayerBoards();
    RefreshBoard(-1);
}

void UTMOPBusArrivalBoardComponent::EnsurePlayerBoards()
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
    if (!IsValid(Owner) || !Owner->GetRootComponent() || !GameInstance) return;
    const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
    bool bMustRebuild = BoardComponents.Num() != LocalPlayers.Num();
    if (!bMustRebuild)
        for (int32 Index = 0; Index < BoardComponents.Num(); ++Index)
            if (!IsValid(BoardComponents[Index]) ||
                BoardComponents[Index]->GetOwnerPlayer() != LocalPlayers[Index])
            {
                bMustRebuild = true;
                break;
            }
    if (!bMustRebuild) return;

    for (UWidgetComponent* Existing : BoardComponents)
        if (IsValid(Existing)) Existing->DestroyComponent();
    BoardComponents.Reset();
    BoardWidgets.Reset();
    LastDisplayedSecond = INDEX_NONE;
    for (int32 Index = 0; Index < LocalPlayers.Num(); ++Index)
    {
        ULocalPlayer* LocalPlayer = LocalPlayers[Index];
        if (!IsValid(LocalPlayer)) continue;
        UWidgetComponent* BoardComponent = NewObject<UWidgetComponent>(
            Owner, NAME_None, RF_Transient);
        if (!IsValid(BoardComponent)) continue;
        Owner->AddInstanceComponent(BoardComponent);
        BoardComponent->SetupAttachment(IsValid(Stop) ? Stop.Get() : Owner->GetRootComponent());
        BoardComponent->SetRelativeLocation(BoardOffset);
        BoardComponent->SetWidgetSpace(EWidgetSpace::Screen);
        BoardComponent->SetOwnerPlayer(LocalPlayer);
        BoardComponent->SetDrawAtDesiredSize(true);
        BoardComponent->SetPivot(FVector2D(0.5f, 1.0f));
        BoardComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BoardComponent->SetGenerateOverlapEvents(false);
        BoardComponent->SetWidgetClass(UTMOPMetroBoardWidget::StaticClass());
        BoardComponent->RegisterComponent();
        BoardComponent->InitWidget();
        BoardComponents.Add(BoardComponent);
        BoardWidgets.Add(Cast<UTMOPMetroBoardWidget>(
            BoardComponent->GetUserWidgetObject()));
    }
}

void UTMOPBusArrivalBoardComponent::RefreshBoard(const int32 CurrentSecond)
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    auto* Network = GI ? GI->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    auto* Stops = GI ? GI->GetSubsystem<UTMOPBusStopSubsystem>() : nullptr;
    struct FArrival { float Seconds; FString Line; FString Destination; bool Here; };
    TArray<FArrival> Arrivals;
    const int32 Now = CurrentSecond >= 0 ? CurrentSecond : 23 * 3600;
    bool bIncomplete = !Network || !Stops || !IsValid(Stop);
    if (Network && Stops && IsValid(Stop))
    for (TActorIterator<ATMOPBusScheduleDirector> It(GetWorld()); It; ++It)
    {
        for (const FTMOPBusRunRuntime& Runtime : It->GetRuntimeRuns())
        {
            if (Runtime.State == ETMOPBusRunState::Completed ||
                Runtime.State == ETMOPBusRunState::Failed ||
                !It->ScheduledRuns.IsValidIndex(Runtime.SourceIndex)) continue;
            const FTMOPBusScheduledRun& Run = It->ScheduledRuns[Runtime.SourceIndex];
            const UTMOPBusRouteData* Route = Run.RouteData;
            if (!IsValid(Route) || !Route->OrderedStopIds.Contains(Stop->StopId)) continue;
            const auto* Bus = Runtime.SpawnedBus.Get();
            const auto* Service = IsValid(Bus) ? Bus->FindComponentByClass<UTMOPBusServiceComponent>() : nullptr;
            const auto* Movement = IsValid(Bus) ? Bus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>() : nullptr;
            const bool bActive = Runtime.State == ETMOPBusRunState::Active;
            if (bActive && (!Service || !Movement)) { bIncomplete = true; continue; }
            if (Service && (Service->ServiceState == ETMOPBusServiceState::RouteComplete ||
                Service->ServiceState == ETMOPBusServiceState::InvalidRoute)) continue;
            const int32 FirstStop = bActive ? Service->CurrentStopIndex : 0;
            if (FirstStop < 0) { bIncomplete = true; continue; }
            int32 TargetStop = INDEX_NONE;
            for (int32 I = FirstStop; I < Route->OrderedStopIds.Num(); ++I)
                if (Route->OrderedStopIds[I] == Stop->StopId) { TargetStop = I; break; }
            if (TargetStop == INDEX_NONE) continue; // Already departed from this stop.
            const bool bHere = bActive && FirstStop == TargetStop &&
                Service->ServiceState == ETMOPBusServiceState::Dwelling;
            float Seconds = bActive ? 0.0f : FMath::Max(0, Runtime.ResolvedStartTime.ToSecondsFromMidnight() - Now);
            const FName StartLane = bActive ? Movement->CurrentLaneId :
                (!Run.InitialLaneId.IsNone() ? Run.InitialLaneId :
                 (Route->OrderedLaneIds.IsEmpty() ? NAME_None : Route->OrderedLaneIds[0]));
            int32 LaneIndex = Route->OrderedLaneIds.IndexOfByKey(StartLane);
            float Distance = bActive ? Movement->DistanceAlongLane : Run.InitialDistanceAlongLane;
            bool bValid = LaneIndex != INDEX_NONE;
            // Integrate the authored directed lane path, including intermediate stops.
            for (int32 S = FirstStop; bValid && !bHere && S <= TargetStop; ++S)
            {
                const UTMOPBusStopComponent* DestinationStop = Stops->FindStop(Route->OrderedStopIds[S]);
                if (!DestinationStop) { bValid = false; break; }
                bool bReached = false;
                for (; LaneIndex < Route->OrderedLaneIds.Num(); ++LaneIndex)
                {
                    const auto* Lane = Network->FindLane(Route->OrderedLaneIds[LaneIndex]);
                    if (!Lane) { bValid = false; break; }
                    const bool bDestinationLane = Lane->LaneId == DestinationStop->LaneId;
                    const float End = bDestinationLane ? DestinationStop->DistanceAlongLane : Lane->GetSplineLength();
                    if (bDestinationLane && End + DestinationStop->StopBufferCm < Distance)
                    { bValid = false; break; }
                    float Speed = Lane->GetSpeedLimitCentimetersPerSecond() *
                        (bActive ? Movement->SpeedLimitMultiplier : Run.SpeedLimitMultiplier);
                    if (bActive && Movement->DesiredCruiseSpeedKmh > 0)
                        Speed = FMath::Min(Speed, Movement->DesiredCruiseSpeedKmh / 0.036f);
                    if (Speed <= 0) { bValid = false; break; }
                    Seconds += FMath::Max(0.0f, End - Distance) / Speed;
                    Distance = bDestinationLane ? End : 0.0f;
                    if (bDestinationLane) { bReached = true; break; }
                }
                if (!bReached) bValid = false;
                if (bValid && S < TargetStop)
                    Seconds += 0.5f * (DestinationStop->MinimumDwellSeconds + DestinationStop->MaximumDwellSeconds);
            }
            if (!bValid && !bHere) { bIncomplete = true; continue; }
            if (Run.bUseForcedDespawnTime && Now + Seconds > Run.ForcedDespawnTime.ToSecondsFromMidnight()) continue;
            if (Now + Seconds > 23 * 3600 + 45 * 60) continue;
            Arrivals.Add({Seconds, Route->PublicLineNumber.IsEmpty() ? Route->RouteId.ToString() : Route->PublicLineNumber.ToString(),
                Route->DestinationDisplay.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.32e677136ad1666a", "Destination saknas").ToString() : Route->DestinationDisplay.ToString(), bHere});
        }
    }
    Arrivals.Sort([](const FArrival& A, const FArrival& B) { return A.Seconds < B.Seconds; });
    TArray<FString> Lines;
    for (const FArrival& A : Arrivals)
    {
        const FString Countdown = A.Here ? NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.fde432f9941c92f8", "NU").ToString() :
            (A.Seconds < 60 ? TEXT("<1 min") : FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.259b807bdcb5c581", "ca {0} min"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), FMath::CeilToInt(A.Seconds / 60.0f)))).ToString());
        Lines.Add(FString::Printf(TEXT("%s   %-18s %s"), *A.Line, *A.Destination, *Countdown));
        if (Lines.Num() >= FMath::Clamp(UpcomingRows, 1, 5)) break;
    }
    if (Lines.IsEmpty()) Lines.Add(bIncomplete ? NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.7b01292798c94ecf", "Ankomsttid saknas").ToString() : NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.55f32b0ee14c45ce", "Inga fler bussar i spelperioden").ToString());
    const FText Header = FText::FromString((StationName.IsEmpty() ? StationId.ToString() : StationName.ToString()) + NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.6d08886363d330ad", "  •  BUSS").ToString());
    const FText Note = bShowEstimateNote ? FText::FromString(NSLOCTEXT("TMOP", "TMOPBusArrivalBoardComponent.0c700eb820604064", "Beräknade tider – trafiken kan påverka ankomsten.").ToString()) : FText::GetEmpty();
    for (UTMOPMetroBoardWidget* Widget : BoardWidgets)
        if (IsValid(Widget)) Widget->SetBoard(Header, FText::FromString(FString::Join(Lines, TEXT("\n"))), Note);
}


void UTMOPBusArrivalBoardComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
    EnsurePlayerBoards();
    const TArray<ULocalPlayer*>* LocalPlayers = GameInstance
        ? &GameInstance->GetLocalPlayers() : nullptr;
    for (int32 Index = 0; Index < BoardComponents.Num(); ++Index)
    {
        UWidgetComponent* Board = BoardComponents[Index];
        ULocalPlayer* Local = LocalPlayers && LocalPlayers->IsValidIndex(Index)
            ? (*LocalPlayers)[Index] : nullptr;
        APlayerController* Controller = IsValid(Local)
            ? Local->GetPlayerController(World) : nullptr;
        const ATMOPPlayerCharacter* Player = IsValid(Controller)
            ? Cast<ATMOPPlayerCharacter>(Controller->GetPawn()) : nullptr;
        const bool bNear = IsValid(Controller) && IsValid(Controller->PlayerCameraManager) &&
            FVector::DistSquared(Controller->PlayerCameraManager->GetCameraLocation(),
                (IsValid(Stop) ? Stop->GetComponentTransform().TransformPosition(BoardOffset) : GetOwner()->GetActorLocation() + BoardOffset)) <=
            FMath::Square(FMath::Max(MaximumVisibleDistanceCm, 100.0f));
        if (IsValid(Board)) Board->SetVisibility(
            IsValid(Player) && Player->IsGameplayHUDVisible() && bNear, true);
    }
    const UTMOPClockSubsystem* Clock = GameInstance
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!Clock) return;
    const int32 CurrentSecond = FMath::FloorToInt(Clock->GetCurrentTimeSecondsExact());
    // Refresh also after local-player boards are rebuilt.
    if (CurrentSecond == LastDisplayedSecond) return;
    LastDisplayedSecond = CurrentSecond;
    RefreshBoard(CurrentSecond);
}

void UTMOPBusArrivalBoardComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    for (UWidgetComponent* Board : BoardComponents)
        if (IsValid(Board)) Board->DestroyComponent();
    BoardComponents.Reset();
    BoardWidgets.Reset();
    Super::EndPlay(EndPlayReason);
}

