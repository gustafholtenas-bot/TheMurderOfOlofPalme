#include "Venues/TMOPGrandRowExitDirector.h"

#include "AIController.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Venues/TMOPCinemaSeatExitComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"
#include "Venues/TMOPGrandFilmBehaviorComponent.h"
#include "Venues/TMOPGrandRowSubsystem.h"

ATMOPGrandRowExitDirector::ATMOPGrandRowExitDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPGrandRowExitDirector::BeginPlay()
{
    Super::BeginPlay();
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPGrandRowSubsystem* Rows = GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>())
            Rows->DiscoverRowsInWorld();
    }
}

void ATMOPGrandRowExitDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ScanAccumulator += DeltaSeconds;
    if (bAutoProcessStandingAgents && ScanAccumulator >= ScanIntervalSeconds)
    {
        ScanAccumulator = 0.0f;
        ScanForStandingAgents();
    }
    UpdateMovingAgents();
    StartAvailableAgents();
}

void ATMOPGrandRowExitDirector::ScanForStandingAgents()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPHistoricalAgent> It(World); It; ++It)
    {
        ATMOPHistoricalAgent* Agent = *It;
        if (!IsValid(Agent) || Agent->ActivityState != ETMOPAgentActivityState::Standing) continue;
        UTMOPGrandFilmBehaviorComponent* FilmBehavior =
            Agent->FindComponentByClass<UTMOPGrandFilmBehaviorComponent>();
        if (FilmBehavior == nullptr || !FilmBehavior->bHasStoodThisLoop) continue;
        QueueAgent(Agent);
    }
}

bool ATMOPGrandRowExitDirector::QueueAgent(ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || Agent->InitialSeatAssignment.SeatId.IsNone()) return false;
    for (const FTMOPGrandRowExitStatus& Existing : ExitStatuses)
        if (Existing.Agent == Agent) return false;

    UGameInstance* GameInstance = GetGameInstance();
    UTMOPGrandRowSubsystem* Rows = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>() : nullptr;
    UTMOPGrandRowComponent* Row = Rows != nullptr
        ? Rows->FindRowForSeat(Agent->InitialSeatAssignment.SeatId) : nullptr;
    if (!IsValid(Row))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP cannot find a Grand row for seat '%s'."),
            *Agent->InitialSeatAssignment.SeatId.ToString());
        return false;
    }

    ETMOPGrandAisleSide Side = Row->ChooseNearestAisle(Agent->InitialSeatAssignment.SeatId);
    UTMOPCinemaSeatSubsystem* Seats = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>() : nullptr;
    UTMOPCinemaSeatComponent* Seat = Seats != nullptr
        ? Seats->FindSeat(Agent->InitialSeatAssignment.SeatId) : nullptr;
    AActor* SeatOwner = IsValid(Seat) ? Seat->GetOwner() : nullptr;
    if (UTMOPCinemaSeatExitComponent* Exit = SeatOwner != nullptr
        ? SeatOwner->FindComponentByClass<UTMOPCinemaSeatExitComponent>() : nullptr)
    {
        if (Exit->PreferredAisle != ETMOPGrandAisleSide::Automatic)
            Side = Exit->PreferredAisle;
    }

    FTMOPGrandRowExitStatus Status;
    Status.Agent = Agent;
    Status.SeatId = Agent->InitialSeatAssignment.SeatId;
    Status.RowId = Row->RowId;
    Status.AisleSide = Side;
    Status.State = ETMOPGrandRowExitState::Queued;
    ExitStatuses.Add(Status);
    return true;
}

void ATMOPGrandRowExitDirector::StartAvailableAgents()
{
    TArray<int32> Candidates;
    for (int32 Index = 0; Index < ExitStatuses.Num(); ++Index)
        if (ExitStatuses[Index].State == ETMOPGrandRowExitState::Queued) Candidates.Add(Index);
    Candidates.Sort([this](const int32 A, const int32 B)
    {
        return GetDistanceInSeats(ExitStatuses[A]) < GetDistanceInSeats(ExitStatuses[B]);
    });

    for (const int32 Index : Candidates)
    {
        FTMOPGrandRowExitStatus& Status = ExitStatuses[Index];
        if (IsLaneActive(Status.RowId, Status.AisleSide)) continue;
        if (!bAllowBothAislesInParallel)
        {
            const ETMOPGrandAisleSide Other = Status.AisleSide == ETMOPGrandAisleSide::Left
                ? ETMOPGrandAisleSide::Right : ETMOPGrandAisleSide::Left;
            if (IsLaneActive(Status.RowId, Other)) continue;
        }
        StartMoving(Status);
    }
}

bool ATMOPGrandRowExitDirector::StartMoving(FTMOPGrandRowExitStatus& Status)
{
    ATMOPHistoricalAgent* Agent = Status.Agent;
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPGrandRowSubsystem* Rows = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>() : nullptr;
    UTMOPGrandRowComponent* Row = Rows != nullptr ? Rows->FindRow(Status.RowId) : nullptr;
    AAIController* Controller = IsValid(Agent) ? Cast<AAIController>(Agent->GetController()) : nullptr;
    if (!IsValid(Agent) || !IsValid(Row) || Controller == nullptr)
    {
        Status.State = ETMOPGrandRowExitState::Failed;
        return false;
    }

    Agent->SetActivityState(ETMOPAgentActivityState::Walking);
    const FVector Target = Row->GetAisleWorldTransform(Status.AisleSide).GetLocation();
    Controller->MoveToLocation(Target, AisleAcceptanceRadius, true, true, false, true);
    Status.State = ETMOPGrandRowExitState::MovingToAisle;
    return true;
}

void ATMOPGrandRowExitDirector::UpdateMovingAgents()
{
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPGrandRowSubsystem* Rows = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>() : nullptr;
    for (FTMOPGrandRowExitStatus& Status : ExitStatuses)
    {
        if (Status.State != ETMOPGrandRowExitState::MovingToAisle) continue;
        ATMOPHistoricalAgent* Agent = Status.Agent;
        UTMOPGrandRowComponent* Row = Rows != nullptr ? Rows->FindRow(Status.RowId) : nullptr;
        if (!IsValid(Agent) || !IsValid(Row))
        {
            Status.State = ETMOPGrandRowExitState::Failed;
            continue;
        }
        const FVector Target = Row->GetAisleWorldTransform(Status.AisleSide).GetLocation();
        if (FVector::DistSquared2D(Agent->GetActorLocation(), Target) <= FMath::Square(AisleAcceptanceRadius))
        {
            if (AAIController* Controller = Cast<AAIController>(Agent->GetController()))
                Controller->StopMovement();
            Agent->SetActivityState(ETMOPAgentActivityState::Standing);
            Status.State = ETMOPGrandRowExitState::ReachedAisle;
        }
    }
}

bool ATMOPGrandRowExitDirector::IsLaneActive(const FName RowId, const ETMOPGrandAisleSide Side) const
{
    for (const FTMOPGrandRowExitStatus& Status : ExitStatuses)
        if (Status.RowId == RowId && Status.AisleSide == Side &&
            Status.State == ETMOPGrandRowExitState::MovingToAisle) return true;
    return false;
}

int32 ATMOPGrandRowExitDirector::GetDistanceInSeats(const FTMOPGrandRowExitStatus& Status) const
{
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPGrandRowSubsystem* Rows = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPGrandRowSubsystem>() : nullptr;
    UTMOPGrandRowComponent* Row = Rows != nullptr ? Rows->FindRow(Status.RowId) : nullptr;
    if (!IsValid(Row)) return MAX_int32;
    const int32 Index = Row->GetSeatIndex(Status.SeatId);
    return Status.AisleSide == ETMOPGrandAisleSide::Right
        ? Row->OrderedSeatIds.Num() - 1 - Index : Index;
}

void ATMOPGrandRowExitDirector::ResetRowExits()
{
    for (FTMOPGrandRowExitStatus& Status : ExitStatuses)
        if (IsValid(Status.Agent))
            if (AAIController* Controller = Cast<AAIController>(Status.Agent->GetController()))
                Controller->StopMovement();
    ExitStatuses.Reset();
}

int32 ATMOPGrandRowExitDirector::GetQueuedCount() const
{
    int32 Count = 0;
    for (const FTMOPGrandRowExitStatus& Status : ExitStatuses)
        if (Status.State == ETMOPGrandRowExitState::Queued) ++Count;
    return Count;
}
int32 ATMOPGrandRowExitDirector::GetMovingCount() const
{
    int32 Count = 0;
    for (const FTMOPGrandRowExitStatus& Status : ExitStatuses)
        if (Status.State == ETMOPGrandRowExitState::MovingToAisle) ++Count;
    return Count;
}
int32 ATMOPGrandRowExitDirector::GetReachedAisleCount() const
{
    int32 Count = 0;
    for (const FTMOPGrandRowExitStatus& Status : ExitStatuses)
        if (Status.State == ETMOPGrandRowExitState::ReachedAisle) ++Count;
    return Count;
}
