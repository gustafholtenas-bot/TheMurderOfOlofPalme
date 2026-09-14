#include "Transit/TMOPMetroEntranceBoardComponent.h"

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

UTMOPMetroEntranceBoardComponent::UTMOPMetroEntranceBoardComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UTMOPMetroEntranceBoardComponent::ConfigureForStation(
    const FName NewStationId, const FText& NewStationName)
{
    StationId = NewStationId;
    StationName = NewStationName;
}

void UTMOPMetroEntranceBoardComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->GetRootComponent() || StationId.IsNone()) return;

    LoadArrivals();
    EnsurePlayerBoards();
    RefreshBoard(-1);
}

void UTMOPMetroEntranceBoardComponent::EnsurePlayerBoards()
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
    for (int32 Index = 0; Index < LocalPlayers.Num(); ++Index)
    {
        ULocalPlayer* LocalPlayer = LocalPlayers[Index];
        if (!IsValid(LocalPlayer)) continue;
        UWidgetComponent* BoardComponent = NewObject<UWidgetComponent>(
            Owner, NAME_None, RF_Transient);
        if (!IsValid(BoardComponent)) continue;
        Owner->AddInstanceComponent(BoardComponent);
        BoardComponent->SetupAttachment(Owner->GetRootComponent());
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

void UTMOPMetroEntranceBoardComponent::LoadArrivals()
{
    if (!IsValid(ScheduleTable.Get()))
    {
        static const TCHAR* Candidates[] = {
            TEXT("/Game/TMOP/Data/DT_TMOP_MetroArrivals_Northbound.DT_TMOP_MetroArrivals_Northbound"),
            TEXT("/Game/TMOP/DataTables/09_13/DT_TMOP_MetroArrivals_Northbound.DT_TMOP_MetroArrivals_Northbound"),
            TEXT("/Game/DataTables/09_13/DT_TMOP_MetroArrivals_Northbound.DT_TMOP_MetroArrivals_Northbound")};
        for (const TCHAR* Path : Candidates)
            if (UDataTable* Loaded = LoadObject<UDataTable>(nullptr, Path))
            {
                ScheduleTable = Loaded;
                break;
            }
    }

    Arrivals.Reset();
    if (IsValid(ScheduleTable.Get()) &&
        ScheduleTable->GetRowStruct() == FTMOPMetroArrivalRow::StaticStruct())
    {
        TArray<FTMOPMetroArrivalRow*> Rows;
        ScheduleTable->GetAllRows(TEXT("Metro entrance board"), Rows);
        for (const FTMOPMetroArrivalRow* Row : Rows)
            if (Row && Row->StationId == StationId &&
                Row->Direction == TEXT("Northbound"))
                Arrivals.Add(*Row);
    }
    if (Arrivals.IsEmpty()) AddBuiltInArrivals();
    Arrivals.Sort([](const FTMOPMetroArrivalRow& A,
        const FTMOPMetroArrivalRow& B)
    {
        return A.ArrivalTime.ToSecondsFromMidnight() <
            B.ArrivalTime.ToSecondsFromMidnight();
    });
}

void UTMOPMetroEntranceBoardComponent::AddBuiltInArrivals()
{
    struct FArrival { int32 Line; int32 Minute; int32 Second; const TCHAR* Destination; };
    static const FArrival Hotorget[] = {
        {17, 4, 0, TEXT("Vällingby")}, {19, 9, 0, TEXT("Alvik")},
        {18, 14, 0, TEXT("Hässelby strand")}, {17, 24, 0, TEXT("Vällingby")},
        {19, 29, 0, TEXT("Alvik")}, {18, 34, 0, TEXT("Hässelby strand")},
        {17, 44, 0, TEXT("Vällingby")}};
    static const FArrival Radmansgatan[] = {
        {17, 5, 40, TEXT("Vällingby")}, {19, 10, 40, TEXT("Alvik")},
        {18, 15, 40, TEXT("Hässelby strand")}, {17, 25, 40, TEXT("Vällingby")},
        {19, 30, 40, TEXT("Alvik")}, {18, 35, 40, TEXT("Hässelby strand")},
        {17, 45, 40, TEXT("Vällingby")}};
    const FArrival* Source = StationId == TEXT("HOTORGET")
        ? Hotorget : Radmansgatan;
    const int32 Count = 7;
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FTMOPMetroArrivalRow Row;
        Row.StationId = StationId;
        Row.StationName = StationName;
        Row.Direction = TEXT("Northbound");
        Row.Line = Source[Index].Line;
        Row.Destination = FText::FromString(Source[Index].Destination);
        Row.ArrivalTime = FTMOPTime(23, Source[Index].Minute, Source[Index].Second);
        Row.bWithinSimulationWindow =
            Row.ArrivalTime.ToSecondsFromMidnight() <= FTMOPTime(23, 45, 0).ToSecondsFromMidnight();
        Row.SourceNote = TEXT("Körplan norrut, måndag–fredag; mellantid beräknad från stationskörningen TCE–Hötorget–Rådmansgatan.");
        Arrivals.Add(Row);
    }
}

void UTMOPMetroEntranceBoardComponent::RefreshBoard(const int32 CurrentSecond)
{
    const int32 Now = CurrentSecond >= 0 ? CurrentSecond : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    TArray<FString> Lines;
    for (const FTMOPMetroArrivalRow& Row : Arrivals)
    {
        if (!Row.bWithinSimulationWindow) continue;
        const int32 Arrival = Row.ArrivalTime.ToSecondsFromMidnight();
        const int32 Remaining = Arrival - Now;
        if (Remaining < -15) continue;
        FString Countdown;
        if (Remaining <= 15) Countdown = TEXT("NU");
        else if (Remaining < 60) Countdown = FString::Printf(TEXT("%d sek"), Remaining);
        else Countdown = FString::Printf(TEXT("%d min"), FMath::CeilToInt(Remaining / 60.0f));
        Lines.Add(FString::Printf(TEXT("%d   %-18s %s"), Row.Line,
            *Row.Destination.ToString(), *Countdown));
        if (Lines.Num() >= FMath::Clamp(UpcomingRows, 1, 5)) break;
    }
    if (Lines.IsEmpty()) Lines.Add(TEXT("Inga fler norrgående tåg i spelperioden"));

    const FText Header = FText::FromString(FString::Printf(TEXT("%s  •  NORRUT"),
        *StationName.ToString()));
    const FText Disclaimer = bShowSouthboundDisclaimer
        ? NSLOCTEXT("TMOP", "MetroSouthboundMissing",
            "Södergående tidtabell saknas och visas därför inte.")
        : FText::GetEmpty();
    const FText ArrivalText = FText::FromString(FString::Join(Lines, TEXT("\n")));
    for (UTMOPMetroBoardWidget* Widget : BoardWidgets)
        if (IsValid(Widget)) Widget->SetBoard(Header, ArrivalText, Disclaimer);
}

void UTMOPMetroEntranceBoardComponent::TickComponent(const float DeltaTime,
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
                GetOwner()->GetActorLocation() + BoardOffset) <=
            FMath::Square(FMath::Max(MaximumVisibleDistanceCm, 100.0f));
        if (IsValid(Board)) Board->SetVisibility(
            IsValid(Player) && Player->IsGameplayHUDVisible() && bNear, true);
    }
    const UTMOPClockSubsystem* Clock = GameInstance
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!Clock) return;
    const int32 CurrentSecond = FMath::FloorToInt(Clock->GetCurrentTimeSecondsExact());
    if (CurrentSecond == LastDisplayedSecond) return;
    LastDisplayedSecond = CurrentSecond;
    RefreshBoard(CurrentSecond);
}

void UTMOPMetroEntranceBoardComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    for (UWidgetComponent* Board : BoardComponents)
        if (IsValid(Board)) Board->DestroyComponent();
    BoardComponents.Reset();
    BoardWidgets.Reset();
    Super::EndPlay(EndPlayReason);
}
