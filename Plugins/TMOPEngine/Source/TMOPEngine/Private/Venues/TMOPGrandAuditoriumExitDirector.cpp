#include "Venues/TMOPGrandAuditoriumExitDirector.h"

#include "AIController.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "EngineUtils.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatExitComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"
#include "Venues/TMOPGrandAuditoriumExitComponent.h"
#include "Venues/TMOPGrandRowExitDirector.h"

ATMOPGrandAuditoriumExitDirector::ATMOPGrandAuditoriumExitDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPGrandAuditoriumExitDirector::BeginPlay()
{
    Super::BeginPlay();
    DiscoverExits();
}

void ATMOPGrandAuditoriumExitDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ScanAccumulator += DeltaSeconds;
    if (ScanAccumulator >= ScanIntervalSeconds)
    {
        ScanAccumulator = 0.0f;
        ImportReachedAisleAgents();
    }
    UpdateMovingAgents();
    StartQueuedAgents();
}

int32 ATMOPGrandAuditoriumExitDirector::DiscoverExits()
{
    Exits.Reset();
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    TSet<FName> ExitIds;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPGrandAuditoriumExitComponent*> Components;
        It->GetComponents<UTMOPGrandAuditoriumExitComponent>(Components);
        for (UTMOPGrandAuditoriumExitComponent* Exit : Components)
        {
            if (!IsValid(Exit) || Exit->ExitId.IsNone()) continue;
            if (ExitIds.Contains(Exit->ExitId))
            {
                UE_LOG(LogTemp, Error, TEXT("TMOP duplicate auditorium ExitId '%s'."), *Exit->ExitId.ToString());
                continue;
            }
            ExitIds.Add(Exit->ExitId);
            Exits.Add(Exit);
        }
    }
    return Exits.Num();
}

bool ATMOPGrandAuditoriumExitDirector::ValidateExits(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (Exits.IsEmpty()) OutErrors.Add(TEXT("No Grand auditorium exits were discovered."));
    TSet<FName> Ids;
    for (const UTMOPGrandAuditoriumExitComponent* Exit : Exits)
    {
        if (!IsValid(Exit)) { OutErrors.Add(TEXT("An auditorium exit is invalid.")); continue; }
        TArray<FString> Errors;
        Exit->ValidateExit(Errors);
        for (const FString& Error : Errors)
            OutErrors.Add(FString::Printf(TEXT("Exit '%s': %s"), *Exit->ExitId.ToString(), *Error));
        if (Ids.Contains(Exit->ExitId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate ExitId '%s'."), *Exit->ExitId.ToString()));
        Ids.Add(Exit->ExitId);
    }
    return OutErrors.IsEmpty();
}

void ATMOPGrandAuditoriumExitDirector::ImportReachedAisleAgents()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPGrandRowExitDirector> It(World); It; ++It)
    {
        for (const FTMOPGrandRowExitStatus& RowStatus : It->GetExitStatuses())
        {
            if (RowStatus.State != ETMOPGrandRowExitState::ReachedAisle || !IsValid(RowStatus.Agent)) continue;
            bool bKnown = false;
            for (const FTMOPGrandAuditoriumExitStatus& Existing : Statuses)
                bKnown |= Existing.Agent == RowStatus.Agent;
            if (bKnown) continue;
            UTMOPGrandAuditoriumExitComponent* Exit = ChooseExit(RowStatus.Agent);
            FTMOPGrandAuditoriumExitStatus NewStatus;
            NewStatus.Agent = RowStatus.Agent;
            NewStatus.ExitId = IsValid(Exit) ? Exit->ExitId : NAME_None;
            NewStatus.State = IsValid(Exit)
                ? ETMOPGrandAuditoriumExitState::Queued : ETMOPGrandAuditoriumExitState::Failed;
            Statuses.Add(NewStatus);
        }
    }
}

UTMOPGrandAuditoriumExitComponent* ATMOPGrandAuditoriumExitDirector::ChooseExit(ATMOPHistoricalAgent* Agent) const
{
    if (!IsValid(Agent)) return nullptr;
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPCinemaSeatSubsystem* Seats = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>() : nullptr;
    UTMOPCinemaSeatComponent* Seat = Seats != nullptr
        ? Seats->FindSeat(Agent->InitialSeatAssignment.SeatId) : nullptr;
    AActor* SeatOwner = IsValid(Seat) ? Seat->GetOwner() : nullptr;
    UTMOPCinemaSeatExitComponent* Metadata = SeatOwner != nullptr
        ? SeatOwner->FindComponentByClass<UTMOPCinemaSeatExitComponent>() : nullptr;
    if (Metadata != nullptr && !Metadata->AuditoriumExitRouteId.IsNone())
        if (UTMOPGrandAuditoriumExitComponent* Preferred = FindExit(Metadata->AuditoriumExitRouteId))
            return Preferred;

    UTMOPGrandAuditoriumExitComponent* Best = nullptr;
    float BestDistance = TNumericLimits<float>::Max();
    for (UTMOPGrandAuditoriumExitComponent* Exit : Exits)
    {
        if (!IsValid(Exit) || Exit->AuditoriumId != Agent->InitialSeatAssignment.AuditoriumId) continue;
        const float Distance = FVector::DistSquared2D(Agent->GetActorLocation(), Exit->GetComponentLocation());
        if (Distance < BestDistance) { BestDistance = Distance; Best = Exit; }
    }
    return Best;
}

UTMOPGrandAuditoriumExitComponent* ATMOPGrandAuditoriumExitDirector::FindExit(const FName ExitId) const
{
    for (UTMOPGrandAuditoriumExitComponent* Exit : Exits)
        if (IsValid(Exit) && Exit->ExitId == ExitId) return Exit;
    return nullptr;
}

void ATMOPGrandAuditoriumExitDirector::StartQueuedAgents()
{
    for (FTMOPGrandAuditoriumExitStatus& Status : Statuses)
    {
        if (Status.State != ETMOPGrandAuditoriumExitState::Queued ||
            GetMovingCountForExit(Status.ExitId) >= MaximumSimultaneousPerExit) continue;
        ATMOPHistoricalAgent* Agent = Status.Agent;
        UTMOPGrandAuditoriumExitComponent* Exit = FindExit(Status.ExitId);
        AAIController* Controller = IsValid(Agent) ? Cast<AAIController>(Agent->GetController()) : nullptr;
        if (!IsValid(Agent) || !IsValid(Exit) || Controller == nullptr)
        {
            Status.State = ETMOPGrandAuditoriumExitState::Failed;
            continue;
        }
        Agent->SetActivityState(ETMOPAgentActivityState::Walking);
        Controller->MoveToLocation(Exit->GetComponentLocation(), Exit->AcceptanceRadius,
            true, true, false, true);
        Status.State = ETMOPGrandAuditoriumExitState::Moving;
    }
}

void ATMOPGrandAuditoriumExitDirector::UpdateMovingAgents()
{
    for (FTMOPGrandAuditoriumExitStatus& Status : Statuses)
    {
        if (Status.State != ETMOPGrandAuditoriumExitState::Moving) continue;
        ATMOPHistoricalAgent* Agent = Status.Agent;
        UTMOPGrandAuditoriumExitComponent* Exit = FindExit(Status.ExitId);
        if (!IsValid(Agent) || !IsValid(Exit))
        {
            Status.State = ETMOPGrandAuditoriumExitState::Failed;
            continue;
        }
        if (FVector::DistSquared2D(Agent->GetActorLocation(), Exit->GetComponentLocation()) <=
            FMath::Square(Exit->AcceptanceRadius))
        {
            if (AAIController* Controller = Cast<AAIController>(Agent->GetController()))
                Controller->StopMovement();
            Agent->SetActivityState(ETMOPAgentActivityState::Standing);
            Status.State = ETMOPGrandAuditoriumExitState::LeftAuditorium;
        }
    }
}

int32 ATMOPGrandAuditoriumExitDirector::GetMovingCountForExit(const FName ExitId) const
{
    int32 Count = 0;
    for (const FTMOPGrandAuditoriumExitStatus& Status : Statuses)
        if (Status.ExitId == ExitId && Status.State == ETMOPGrandAuditoriumExitState::Moving) ++Count;
    return Count;
}

void ATMOPGrandAuditoriumExitDirector::ResetAuditoriumExits()
{
    for (FTMOPGrandAuditoriumExitStatus& Status : Statuses)
        if (IsValid(Status.Agent))
            if (AAIController* Controller = Cast<AAIController>(Status.Agent->GetController()))
                Controller->StopMovement();
    Statuses.Reset();
}

int32 ATMOPGrandAuditoriumExitDirector::GetQueuedCount() const
{
    int32 Count = 0;
    for (const FTMOPGrandAuditoriumExitStatus& Status : Statuses)
        if (Status.State == ETMOPGrandAuditoriumExitState::Queued) ++Count;
    return Count;
}
int32 ATMOPGrandAuditoriumExitDirector::GetLeftAuditoriumCount() const
{
    int32 Count = 0;
    for (const FTMOPGrandAuditoriumExitStatus& Status : Statuses)
        if (Status.State == ETMOPGrandAuditoriumExitState::LeftAuditorium) ++Count;
    return Count;
}
