#include "Venues/TMOPGrandAudienceTestSpawner.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

ATMOPGrandAudienceTestSpawner::ATMOPGrandAudienceTestSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPGrandAudienceTestSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (bSpawnAutomatically) SpawnTestAudience();
}

ATMOPHistoricalAgent* ATMOPGrandAudienceTestSpawner::SpawnAudienceMember(
    const FName EntityId, const FText& Name, const FName SeatId,
    TSubclassOf<ATMOPHistoricalAgent> ClassOverride, const FString& Source)
{
    if (GetWorld() == nullptr || SeatId.IsNone()) return nullptr;
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPCinemaSeatSubsystem* Seats = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>() : nullptr;
    UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(SeatId) : nullptr;
    if (!IsValid(Seat) || Seat->IsOccupied())
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP audience skipped unavailable seat '%s'."), *SeatId.ToString());
        return nullptr;
    }

    TSubclassOf<ATMOPHistoricalAgent> SpawnClass = ClassOverride != nullptr ? ClassOverride : AgentClass;
    if (SpawnClass == nullptr) return nullptr;

    ATMOPHistoricalAgent* Agent = GetWorld()->SpawnActorDeferred<ATMOPHistoricalAgent>(
        SpawnClass, Seat->GetApproachWorldTransform(), this, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!IsValid(Agent)) return nullptr;

    Agent->DisplayName = Name;
    Agent->SourceReference = Source;
    Agent->InitialSeatAssignment.VenueId = VenueId;
    Agent->InitialSeatAssignment.AuditoriumId = AuditoriumId;
    Agent->InitialSeatAssignment.SeatId = SeatId;
    Agent->InitialSeatAssignment.RowNumber = Seat->RowNumber;
    Agent->InitialSeatAssignment.SeatNumber = Seat->SeatNumber;
    Agent->InitialSeatAssignment.bStartsSeated = true;
    if (Agent->EntityIdentity != nullptr && !EntityId.IsNone())
    {
        Agent->EntityIdentity->EntityId = EntityId;
        Agent->EntityIdentity->EntityType = TEXT("Agent");
    }

    UGameplayStatics::FinishSpawningActor(Agent, Seat->GetApproachWorldTransform());
    if (!Seat->IsOccupied() || Seat->GetOccupyingAgent() != Agent)
    {
        Agent->Destroy();
        return nullptr;
    }
    SpawnedAgents.Add(Agent);
    return Agent;
}

int32 ATMOPGrandAudienceTestSpawner::SpawnTestAudience()
{
    if (GetWorld() == nullptr || GetSpawnedAudienceCount() > 0) return 0;
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPCinemaSeatSubsystem* Seats = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>() : nullptr;
    if (Seats == nullptr) return 0;
    Seats->DiscoverSeatsInWorld();

    TArray<FString> Errors;
    if (!ValidateAudienceConfiguration(Errors))
    {
        for (const FString& Error : Errors)
            UE_LOG(LogTemp, Error, TEXT("TMOP Grand audience: %s"), *Error);
        return 0;
    }

    int32 Spawned = 0;
    for (const FTMOPGrandAudienceEntry& Entry : AudienceEntries)
    {
        if (Spawned >= MaximumAgents) break;
        Spawned += SpawnAudienceMember(Entry.EntityId, Entry.DisplayName, Entry.SeatId,
            Entry.AgentClass, Entry.SourceReference) != nullptr ? 1 : 0;
    }
    for (int32 Index = 0; Index < SeatIds.Num() && Spawned < MaximumAgents; ++Index)
    {
        const FName AnonymousId(*FString::Printf(TEXT("GRAND_AMBIENT_%04d"), Index + 1));
        const FText AnonymousName = FText::FromString(FString::Printf(TEXT("Grandbesökare %d"), Index + 1));
        Spawned += SpawnAudienceMember(AnonymousId, AnonymousName, SeatIds[Index], nullptr,
            TEXT("Ambient Grand audience")) != nullptr ? 1 : 0;
    }
    UE_LOG(LogTemp, Log, TEXT("TMOP Grand audience spawned: %d"), Spawned);
    return Spawned;
}

int32 ATMOPGrandAudienceTestSpawner::ClearSpawnedAudience()
{
    int32 Cleared = 0;
    for (ATMOPHistoricalAgent* Agent : SpawnedAgents)
    {
        if (IsValid(Agent))
        {
            Agent->Destroy();
            ++Cleared;
        }
    }
    SpawnedAgents.Reset();
    return Cleared;
}

int32 ATMOPGrandAudienceTestSpawner::ResetAudience()
{
    ClearSpawnedAudience();
    return SpawnTestAudience();
}

int32 ATMOPGrandAudienceTestSpawner::GetSpawnedAudienceCount() const
{
    int32 Count = 0;
    for (const ATMOPHistoricalAgent* Agent : SpawnedAgents) Count += IsValid(Agent) ? 1 : 0;
    return Count;
}

bool ATMOPGrandAudienceTestSpawner::ValidateAudienceConfiguration(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (AgentClass == nullptr)
    {
        bool bAllNamedHaveClasses = !AudienceEntries.IsEmpty();
        for (const FTMOPGrandAudienceEntry& Entry : AudienceEntries)
            bAllNamedHaveClasses &= Entry.AgentClass != nullptr;
        if (!bAllNamedHaveClasses || !SeatIds.IsEmpty()) OutErrors.Add(TEXT("AgentClass is missing."));
    }

    TSet<FName> UsedSeats;
    TSet<FName> UsedEntities;
    for (const FTMOPGrandAudienceEntry& Entry : AudienceEntries)
    {
        if (Entry.EntityId.IsNone()) OutErrors.Add(TEXT("A named audience entry has no EntityId."));
        else if (UsedEntities.Contains(Entry.EntityId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate EntityId '%s'."), *Entry.EntityId.ToString()));
        if (Entry.SeatId.IsNone()) OutErrors.Add(TEXT("A named audience entry has no SeatId."));
        else if (UsedSeats.Contains(Entry.SeatId))
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' is assigned more than once."), *Entry.SeatId.ToString()));
        UsedEntities.Add(Entry.EntityId);
        UsedSeats.Add(Entry.SeatId);
    }
    for (const FName SeatId : SeatIds)
    {
        if (SeatId.IsNone()) OutErrors.Add(TEXT("Anonymous SeatIds contains an empty value."));
        else if (UsedSeats.Contains(SeatId))
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' is assigned more than once."), *SeatId.ToString()));
        UsedSeats.Add(SeatId);
    }
    if (AudienceEntries.Num() + SeatIds.Num() > MaximumAgents)
        OutErrors.Add(TEXT("Configured audience exceeds MaximumAgents."));
    return OutErrors.IsEmpty();
}
