#include "Traffic/TMOPTrafficSpawner.h"

#include "Engine/World.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleModelData.h"

ATMOPTrafficSpawner::ATMOPTrafficSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPTrafficSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (bSpawnAutomatically) SpawnTraffic();
}

int32 ATMOPTrafficSpawner::SpawnTraffic()
{
    if (GetWorld() == nullptr || !SpawnedVehicles.IsEmpty()) return 0;
    TArray<FString> Errors;
    if (!ValidateSpawnEntries(Errors))
    {
        for (const FString& Error : Errors) UE_LOG(LogTemp, Error, TEXT("TMOP Traffic Spawn: %s"), *Error);
        return 0;
    }
    int32 Spawned = 0;
    for (const FTMOPTrafficSpawnEntry& Entry : SpawnEntries)
    {
        UTMOPVehicleModelData* VehicleModel = Entry.VehicleModel.Get();
        ATMOPVehicleBase* Vehicle = GetWorld()->SpawnActor<ATMOPVehicleBase>(
            Entry.VehicleClass, FTransform::Identity);
        if (!IsValid(Vehicle)) continue;
        Vehicle->VehicleId = Entry.VehicleId;
        if (IsValid(Entry.VehicleModel))
        {
            ATMOPConfiguredVehicle* Configured = Cast<ATMOPConfiguredVehicle>(Vehicle);
            if (!IsValid(Configured) || !IsValid(VehicleModel))
            {
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP traffic '%s' has catalog data but VehicleClass is not a configured vehicle or model is missing."),
                    *Entry.VehicleId.ToString());
                Vehicle->Destroy();
                continue;
            }
            Configured->VehicleModel = VehicleModel;
            if (!Configured->ApplyConfiguration())
            {
                Vehicle->Destroy();
                continue;
            }
        }
        UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        if (!IsValid(Movement)) { Vehicle->Destroy(); continue; }
        Movement->PlannedLaneIds = Entry.PlannedLaneIds;
        if (!Movement->InitializeOnLane(Entry.InitialLaneId, Entry.StartDistance))
        {
            Vehicle->Destroy();
            continue;
        }
        Movement->StartDriving();
        SpawnedVehicles.Add(Vehicle);
        ++Spawned;
    }
    return Spawned;
}

int32 ATMOPTrafficSpawner::ClearSpawnedTraffic()
{
    int32 Cleared = 0;
    for (ATMOPVehicleBase* Vehicle : SpawnedVehicles)
        if (IsValid(Vehicle)) { Vehicle->Destroy(); ++Cleared; }
    SpawnedVehicles.Reset();
    return Cleared;
}

bool ATMOPTrafficSpawner::ValidateSpawnEntries(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    TSet<FName> VehicleIds;
    for (int32 Index = 0; Index < SpawnEntries.Num(); ++Index)
    {
        const FTMOPTrafficSpawnEntry& Entry = SpawnEntries[Index];
        if (Entry.VehicleId.IsNone() || VehicleIds.Contains(Entry.VehicleId))
            OutErrors.Add(FString::Printf(TEXT("Entry %d has missing or duplicate VehicleId."), Index));
        VehicleIds.Add(Entry.VehicleId);
        if (Entry.VehicleClass == nullptr) OutErrors.Add(FString::Printf(TEXT("Entry %d has no VehicleClass."), Index));
        if (IsValid(Entry.VehicleModel) &&
            Entry.VehicleClass != nullptr &&
            !Entry.VehicleClass.Get()->IsChildOf(ATMOPConfiguredVehicle::StaticClass()))
            OutErrors.Add(FString::Printf(
                TEXT("Entry %d uses catalog assets but VehicleClass does not inherit TMOPConfiguredVehicle."), Index));
        if (Network == nullptr || !IsValid(Network->FindLane(Entry.InitialLaneId)))
            OutErrors.Add(FString::Printf(TEXT("Entry %d references a missing initial lane."), Index));
        for (int32 OtherIndex = Index + 1; OtherIndex < SpawnEntries.Num(); ++OtherIndex)
        {
            const FTMOPTrafficSpawnEntry& Other = SpawnEntries[OtherIndex];
            if (Entry.InitialLaneId == Other.InitialLaneId &&
                FMath::Abs(Entry.StartDistance - Other.StartDistance) < MinimumInitialCenterSpacingCm)
                OutErrors.Add(FString::Printf(TEXT("Entries %d and %d overlap on lane '%s'."),
                    Index, OtherIndex, *Entry.InitialLaneId.ToString()));
        }
    }
    return OutErrors.IsEmpty();
}
