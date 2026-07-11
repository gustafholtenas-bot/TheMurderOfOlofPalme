#include "Venues/TMOPGrandFoyerDirector.h"

#include "AIController.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Venues/TMOPGrandAuditoriumExitDirector.h"
#include "Venues/TMOPGrandFoyerPointComponent.h"

ATMOPGrandFoyerDirector::ATMOPGrandFoyerDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPGrandFoyerDirector::BeginPlay()
{
    Super::BeginPlay();
    DiscoverFoyerPoints();
}

void ATMOPGrandFoyerDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ScanAccumulator += DeltaSeconds;
    if (ScanAccumulator >= ScanIntervalSeconds)
    {
        ScanAccumulator = 0.0f;
        ImportAgentsFromAuditorium();
    }
    UpdateStatuses(DeltaSeconds);
    StartExitQueues();
}

int32 ATMOPGrandFoyerDirector::DiscoverFoyerPoints()
{
    Points.Reset();
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    TSet<FName> Ids;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPGrandFoyerPointComponent*> Components;
        It->GetComponents<UTMOPGrandFoyerPointComponent>(Components);
        for (UTMOPGrandFoyerPointComponent* Point : Components)
        {
            if (!IsValid(Point) || Point->PointId.IsNone()) continue;
            if (Ids.Contains(Point->PointId))
            {
                UE_LOG(LogTemp, Error, TEXT("TMOP duplicate Grand foyer PointId '%s'."), *Point->PointId.ToString());
                continue;
            }
            Ids.Add(Point->PointId);
            Points.Add(Point);
        }
    }
    return Points.Num();
}

bool ATMOPGrandFoyerDirector::ValidateFoyer(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    int32 ExitCount = 0;
    for (const UTMOPGrandFoyerPointComponent* Point : Points)
        if (IsValid(Point) && Point->PointType == ETMOPGrandFoyerPointType::BuildingExit) ++ExitCount;
    if (ExitCount == 0) OutErrors.Add(TEXT("No BuildingExit foyer point exists."));
    if (!DefaultBuildingExitId.IsNone())
    {
        const UTMOPGrandFoyerPointComponent* DefaultExit = FindPoint(DefaultBuildingExitId);
        if (!IsValid(DefaultExit) || DefaultExit->PointType != ETMOPGrandFoyerPointType::BuildingExit)
            OutErrors.Add(TEXT("DefaultBuildingExitId is missing or is not a BuildingExit."));
    }
    TSet<FName> RuleEntities;
    for (const FTMOPGrandFoyerRule& Rule : PersonRules)
    {
        if (Rule.EntityId.IsNone()) OutErrors.Add(TEXT("A foyer rule has no EntityId."));
        else if (RuleEntities.Contains(Rule.EntityId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate foyer rule for '%s'."), *Rule.EntityId.ToString()));
        RuleEntities.Add(Rule.EntityId);
        if (Rule.bWaitInFoyer)
        {
            const UTMOPGrandFoyerPointComponent* Wait = FindPoint(Rule.WaitingPointId);
            if (!IsValid(Wait) || Wait->PointType != ETMOPGrandFoyerPointType::WaitingPoint)
                OutErrors.Add(FString::Printf(TEXT("Invalid waiting point for '%s'."), *Rule.EntityId.ToString()));
        }
    }
    return OutErrors.IsEmpty();
}

void ATMOPGrandFoyerDirector::ImportAgentsFromAuditorium()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPGrandAuditoriumExitDirector> It(World); It; ++It)
    {
        for (const FTMOPGrandAuditoriumExitStatus& ExitStatus : It->GetExitStatuses())
        {
            if (ExitStatus.State != ETMOPGrandAuditoriumExitState::LeftAuditorium ||
                !IsValid(ExitStatus.Agent)) continue;
            bool bKnown = false;
            for (const FTMOPGrandFoyerStatus& Existing : Statuses) bKnown |= Existing.Agent == ExitStatus.Agent;
            if (bKnown) continue;

            ATMOPHistoricalAgent* Agent = ExitStatus.Agent;
            const FName EntityId = Agent->EntityIdentity != nullptr ? Agent->EntityIdentity->EntityId : NAME_None;
            const FTMOPGrandFoyerRule* Rule = FindRule(EntityId);
            FTMOPGrandFoyerStatus Status;
            Status.Agent = Agent;
            Status.BuildingExitId = Rule != nullptr ? Rule->BuildingExitId : DefaultBuildingExitId;
            if (Rule != nullptr && Rule->bWaitInFoyer)
            {
                Status.WaitingPointId = Rule->WaitingPointId;
                Status.RemainingWaitSeconds = FMath::Max(0.0f, Rule->WaitDurationSeconds);
                UTMOPGrandFoyerPointComponent* WaitPoint = FindPoint(Status.WaitingPointId);
                Status.State = MoveAgentToPoint(Agent, WaitPoint)
                    ? ETMOPGrandFoyerState::MovingToWaitingPoint : ETMOPGrandFoyerState::Failed;
            }
            else Status.State = ETMOPGrandFoyerState::QueuedForBuildingExit;
            Statuses.Add(Status);
        }
    }
}

void ATMOPGrandFoyerDirector::UpdateStatuses(const float DeltaSeconds)
{
    for (FTMOPGrandFoyerStatus& Status : Statuses)
    {
        ATMOPHistoricalAgent* Agent = Status.Agent;
        if (!IsValid(Agent)) { Status.State = ETMOPGrandFoyerState::Failed; continue; }
        if (Status.State == ETMOPGrandFoyerState::MovingToWaitingPoint)
        {
            UTMOPGrandFoyerPointComponent* Point = FindPoint(Status.WaitingPointId);
            if (!IsValid(Point)) { Status.State = ETMOPGrandFoyerState::Failed; continue; }
            if (FVector::DistSquared2D(Agent->GetActorLocation(), Point->GetComponentLocation()) <=
                FMath::Square(Point->AcceptanceRadius))
            {
                if (AAIController* Controller = Cast<AAIController>(Agent->GetController())) Controller->StopMovement();
                Agent->SetActivityState(ETMOPAgentActivityState::Interacting);
                Status.State = ETMOPGrandFoyerState::Waiting;
            }
        }
        else if (Status.State == ETMOPGrandFoyerState::Waiting)
        {
            Status.RemainingWaitSeconds -= DeltaSeconds;
            if (Status.RemainingWaitSeconds <= 0.0f)
            {
                Agent->SetActivityState(ETMOPAgentActivityState::Standing);
                Status.State = ETMOPGrandFoyerState::QueuedForBuildingExit;
            }
        }
        else if (Status.State == ETMOPGrandFoyerState::MovingToBuildingExit)
        {
            UTMOPGrandFoyerPointComponent* Exit = FindPoint(Status.BuildingExitId);
            if (!IsValid(Exit)) { Status.State = ETMOPGrandFoyerState::Failed; continue; }
            if (FVector::DistSquared2D(Agent->GetActorLocation(), Exit->GetComponentLocation()) <=
                FMath::Square(Exit->AcceptanceRadius))
            {
                if (AAIController* Controller = Cast<AAIController>(Agent->GetController())) Controller->StopMovement();
                Agent->SetActivityState(ETMOPAgentActivityState::Standing);
                Status.State = ETMOPGrandFoyerState::LeftGrand;
            }
        }
    }
}

void ATMOPGrandFoyerDirector::StartExitQueues()
{
    for (FTMOPGrandFoyerStatus& Status : Statuses)
    {
        if (Status.State != ETMOPGrandFoyerState::QueuedForBuildingExit) continue;
        UTMOPGrandFoyerPointComponent* Exit = ChooseBuildingExit(Status.Agent, Status.BuildingExitId);
        if (!IsValid(Exit)) { Status.State = ETMOPGrandFoyerState::Failed; continue; }
        Status.BuildingExitId = Exit->PointId;
        if (MovingToExitCount(Status.BuildingExitId) >= MaximumSimultaneousPerBuildingExit) continue;
        Status.State = MoveAgentToPoint(Status.Agent, Exit)
            ? ETMOPGrandFoyerState::MovingToBuildingExit : ETMOPGrandFoyerState::Failed;
    }
}

const FTMOPGrandFoyerRule* ATMOPGrandFoyerDirector::FindRule(const FName EntityId) const
{
    for (const FTMOPGrandFoyerRule& Rule : PersonRules)
        if (!EntityId.IsNone() && Rule.EntityId == EntityId) return &Rule;
    return nullptr;
}

UTMOPGrandFoyerPointComponent* ATMOPGrandFoyerDirector::FindPoint(const FName PointId) const
{
    for (UTMOPGrandFoyerPointComponent* Point : Points)
        if (IsValid(Point) && Point->PointId == PointId) return Point;
    return nullptr;
}

UTMOPGrandFoyerPointComponent* ATMOPGrandFoyerDirector::ChooseBuildingExit(
    const ATMOPHistoricalAgent* Agent, const FName PreferredId) const
{
    UTMOPGrandFoyerPointComponent* Preferred = FindPoint(PreferredId);
    if (IsValid(Preferred) && Preferred->PointType == ETMOPGrandFoyerPointType::BuildingExit) return Preferred;
    UTMOPGrandFoyerPointComponent* Best = nullptr;
    float BestDistance = TNumericLimits<float>::Max();
    for (UTMOPGrandFoyerPointComponent* Point : Points)
    {
        if (!IsValid(Point) || Point->PointType != ETMOPGrandFoyerPointType::BuildingExit) continue;
        const float Distance = IsValid(Agent)
            ? FVector::DistSquared2D(Agent->GetActorLocation(), Point->GetComponentLocation()) : 0.0f;
        if (Distance < BestDistance) { BestDistance = Distance; Best = Point; }
    }
    return Best;
}

bool ATMOPGrandFoyerDirector::MoveAgentToPoint(ATMOPHistoricalAgent* Agent,
    UTMOPGrandFoyerPointComponent* Point)
{
    AAIController* Controller = IsValid(Agent) ? Cast<AAIController>(Agent->GetController()) : nullptr;
    if (!IsValid(Point) || Controller == nullptr) return false;
    Agent->SetActivityState(ETMOPAgentActivityState::Walking);
    Controller->MoveToLocation(Point->GetComponentLocation(), Point->AcceptanceRadius,
        true, true, false, true);
    return true;
}

int32 ATMOPGrandFoyerDirector::MovingToExitCount(const FName ExitId) const
{
    int32 Count = 0;
    for (const FTMOPGrandFoyerStatus& Status : Statuses)
        if (Status.BuildingExitId == ExitId && Status.State == ETMOPGrandFoyerState::MovingToBuildingExit) ++Count;
    return Count;
}

void ATMOPGrandFoyerDirector::ResetFoyer()
{
    for (FTMOPGrandFoyerStatus& Status : Statuses)
        if (IsValid(Status.Agent))
            if (AAIController* Controller = Cast<AAIController>(Status.Agent->GetController())) Controller->StopMovement();
    Statuses.Reset();
}

int32 ATMOPGrandFoyerDirector::GetLeftGrandCount() const
{
    int32 Count = 0;
    for (const FTMOPGrandFoyerStatus& Status : Statuses)
        if (Status.State == ETMOPGrandFoyerState::LeftGrand) ++Count;
    return Count;
}
