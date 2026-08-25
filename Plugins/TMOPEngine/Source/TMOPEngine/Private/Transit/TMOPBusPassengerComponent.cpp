#include "Transit/TMOPBusPassengerComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Components/BoxComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Transit/TMOPBusPassengerManifest.h"
#include "Transit/TMOPBusServiceComponent.h"
#include "Transit/TMOPBusStopComponent.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "World/TMOPWorldSubsystem.h"

namespace
{
int32 GetBusSeatNumber(const UTMOPVehicleSeatComponent* Seat)
{
    if (!IsValid(Seat)) return INDEX_NONE;
    const FString Name = Seat->GetName();
    if (Name.Equals(TEXT("SeatFrontPassenger"), ESearchCase::IgnoreCase)) return 1;
    if (Name.Equals(TEXT("SeatRearLeft"), ESearchCase::IgnoreCase)) return 2;
    if (Name.Equals(TEXT("SeatRearCenter"), ESearchCase::IgnoreCase)) return 3;
    if (Name.Equals(TEXT("SeatRearRight"), ESearchCase::IgnoreCase)) return 4;

    int32 FirstDigit = Name.Len();
    while (FirstDigit > 0 && FChar::IsDigit(Name[FirstDigit - 1])) --FirstDigit;
    return FirstDigit < Name.Len() ? FCString::Atoi(*Name.Mid(FirstDigit)) : INDEX_NONE;
}
}

UTMOPBusPassengerComponent::UTMOPBusPassengerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTMOPBusPassengerComponent::BeginPlay()
{
    Super::BeginPlay();
    VehicleMovement = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>() : nullptr;
    if (IsValid(VehicleMovement)) AddTickPrerequisiteComponent(VehicleMovement);
    if (UTMOPBusServiceComponent* BusService = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPBusServiceComponent>() : nullptr)
        AddTickPrerequisiteComponent(BusService);
    ValidateAndRepairSeats();
    CreatePassengerVolume();
    PreviousOwnerTransform = GetOwner() != nullptr
        ? GetOwner()->GetActorTransform() : FTransform::Identity;
    bPreviousTransformInitialized = GetOwner() != nullptr;
    SpawnDriver();
    if (IsValid(PassengerManifest)) InitializePassengerManifest(PassengerManifest, ServiceRunId);
}

void UTMOPBusPassengerComponent::ValidateAndRepairSeats()
{
    RuntimePassengerSeatCount = 0;
    RuntimeDriverSeatCount = 0;
    if (GetOwner() == nullptr) return;

    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    Seats.RemoveAll([](const UTMOPVehicleSeatComponent* Seat) { return !IsValid(Seat); });
    if (Seats.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': no vehicle seat components exist."),
            *GetOwner()->GetName());
        return;
    }

    UTMOPVehicleSeatComponent* DriverSeat = nullptr;
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (Seat->GetFName() == TEXT("SeatDriver"))
        {
            DriverSeat = Seat;
            break;
        }
    if (!IsValid(DriverSeat))
        for (UTMOPVehicleSeatComponent* Seat : Seats)
            if (Seat->SeatRole == ETMOPVehicleSeatRole::Driver)
            {
                DriverSeat = Seat;
                break;
            }

    int32 Repairs = 0;
    if (!IsValid(DriverSeat))
    {
        DriverSeat = Seats[0];
        ++Repairs;
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP bus '%s': no driver seat was marked; using '%s'."),
            *GetOwner()->GetName(), *DriverSeat->GetName());
    }
    for (UTMOPVehicleSeatComponent* Seat : Seats)
    {
        const ETMOPVehicleSeatRole DesiredRole = Seat == DriverSeat
            ? ETMOPVehicleSeatRole::Driver
            : (Seat->SeatRole == ETMOPVehicleSeatRole::Driver
                ? ETMOPVehicleSeatRole::OtherPassenger : Seat->SeatRole);
        if (Seat->SeatRole != DesiredRole)
        {
            Seat->SeatRole = DesiredRole;
            ++Repairs;
        }
    }
    if (DriverSeat->SeatId != TEXT("DRIVER_01"))
    {
        DriverSeat->SeatId = TEXT("DRIVER_01");
        ++Repairs;
    }

    TArray<UTMOPVehicleSeatComponent*> PassengerSeats;
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (Seat != DriverSeat) PassengerSeats.Add(Seat);
    PassengerSeats.Sort([](const UTMOPVehicleSeatComponent& Left,
        const UTMOPVehicleSeatComponent& Right)
    {
        const int32 LeftNumber = GetBusSeatNumber(&Left);
        const int32 RightNumber = GetBusSeatNumber(&Right);
        if (LeftNumber != RightNumber)
            return (LeftNumber == INDEX_NONE ? MAX_int32 : LeftNumber) <
                (RightNumber == INDEX_NONE ? MAX_int32 : RightNumber);
        return Left.GetName() < Right.GetName();
    });

    for (int32 Index = 0; Index < PassengerSeats.Num(); ++Index)
    {
        const FName DesiredId(*FString::Printf(TEXT("SEAT_%02d"), Index + 1));
        if (PassengerSeats[Index]->SeatId != DesiredId)
        {
            PassengerSeats[Index]->SeatId = DesiredId;
            ++Repairs;
        }
    }
    RuntimeDriverSeatCount = 1;
    RuntimePassengerSeatCount = PassengerSeats.Num();

    if (RuntimePassengerSeatCount != ExpectedPassengerSeatCount)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus '%s': expected %d passenger seats but found %d (plus one driver)."),
            *GetOwner()->GetName(), ExpectedPassengerSeatCount, RuntimePassengerSeatCount);
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("TMOP bus '%s': validated DRIVER_01 and SEAT_01-SEAT_%02d (%d repair(s))."),
            *GetOwner()->GetName(), RuntimePassengerSeatCount, Repairs);
    }
}

void UTMOPBusPassengerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    FreePassengers.Reset();
    if (IsValid(SpawnedDriver)) SpawnedDriver->Destroy();
    SpawnedDriver = nullptr;
    Super::EndPlay(EndPlayReason);
}

void UTMOPBusPassengerComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (GetOwner() == nullptr || !IsValid(PassengerVolume)) return;
    const FTransform CurrentTransform = GetOwner()->GetActorTransform();
    if (bPreviousTransformInitialized)
        CarryRegisteredPassengers(PreviousOwnerTransform, CurrentTransform);
    PreviousOwnerTransform = CurrentTransform;
    bPreviousTransformInitialized = true;
    DiscoverNewPassengers();
    ProcessPassengerManifest();
}

void UTMOPBusPassengerComponent::CreatePassengerVolume()
{
    if (GetOwner() == nullptr || GetOwner()->GetRootComponent() == nullptr) return;
    TArray<UBoxComponent*> ExistingBoxes;
    GetOwner()->GetComponents<UBoxComponent>(ExistingBoxes);
    for (UBoxComponent* Box : ExistingBoxes)
        if (IsValid(Box) && Box->GetFName() == DoorwayVolumeComponentName)
        {
            DoorwayVolume = Box;
            DoorwayVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DoorwayVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
            DoorwayVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
            DoorwayVolume->SetGenerateOverlapEvents(true);
        }
        else if (IsValid(Box) && Box->GetFName() == PassengerVolumeComponentName)
        {
            PassengerVolume = Box;
        }
    if (!IsValid(PassengerVolume))
    {
        PassengerVolume = NewObject<UBoxComponent>(GetOwner(), TEXT("TMOPPassengerVolume"));
        if (!IsValid(PassengerVolume)) return;
        GetOwner()->AddInstanceComponent(PassengerVolume);
        PassengerVolume->SetupAttachment(GetOwner()->GetRootComponent());
        PassengerVolume->SetRelativeLocation(InteriorVolumeCenter);
        PassengerVolume->SetBoxExtent(InteriorVolumeExtent);
        PassengerVolume->RegisterComponent();
    }
    if (!IsValid(PassengerVolume)) return;
    PassengerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PassengerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    PassengerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PassengerVolume->SetGenerateOverlapEvents(true);
    UE_LOG(LogTemp, Display, TEXT("TMOP passenger system '%s': using interior volume '%s'."),
        *GetOwner()->GetName(), *PassengerVolume->GetName());
}

bool UTMOPBusPassengerComponent::IsDoorwayClear() const
{
    if (!IsValid(DoorwayVolume)) return true;
    TArray<AActor*> OverlappingActors;
    DoorwayVolume->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
    for (const AActor* Actor : OverlappingActors)
        if (IsValid(Actor) && Actor != SpawnedDriver) return false;
    return true;
}

void UTMOPBusPassengerComponent::DiscoverNewPassengers()
{
    TArray<AActor*> OverlappingActors;
    PassengerVolume->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
    for (AActor* Actor : OverlappingActors)
        if (ACharacter* Character = Cast<ACharacter>(Actor);
            IsValid(Character) && Character != SpawnedDriver &&
            !Character->IsA<ATMOPHistoricalAgent>() &&
            Character->GetAttachParentActor() != GetOwner())
            RegisterFreePassenger(Character);
}

void UTMOPBusPassengerComponent::CarryRegisteredPassengers(
    const FTransform& PreviousBusTransform, const FTransform& CurrentBusTransform)
{
    for (int32 Index = FreePassengers.Num() - 1; Index >= 0; --Index)
    {
        ACharacter* Character = FreePassengers[Index];
        if (!IsValid(Character) || Character->GetAttachParentActor() == GetOwner())
        {
            FreePassengers.RemoveAtSwap(Index);
            continue;
        }

        const FVector LocalPosition = PreviousBusTransform.InverseTransformPosition(
            Character->GetActorLocation());
        const FQuat LocalRotation = PreviousBusTransform.GetRotation().Inverse() *
            Character->GetActorQuat();
        Character->SetActorLocationAndRotation(
            CurrentBusTransform.TransformPosition(LocalPosition),
            CurrentBusTransform.GetRotation() * LocalRotation,
            false, nullptr, ETeleportType::TeleportPhysics);

        const FVector VolumeLocal = PassengerVolume->GetComponentTransform().InverseTransformPosition(
            Character->GetActorLocation());
        const FVector Allowed = PassengerVolume->GetUnscaledBoxExtent() + FVector(ExitMarginCm);
        if (FMath::Abs(VolumeLocal.X) > Allowed.X ||
            FMath::Abs(VolumeLocal.Y) > Allowed.Y ||
            FMath::Abs(VolumeLocal.Z) > Allowed.Z)
            FreePassengers.RemoveAtSwap(Index);
    }
}

void UTMOPBusPassengerComponent::SpawnDriver()
{
    if (!bSpawnDriverAutomatically || DriverClass == nullptr || GetWorld() == nullptr ||
        GetOwner() == nullptr) return;
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    UTMOPVehicleSeatComponent* DriverSeat = nullptr;
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->SeatRole == ETMOPVehicleSeatRole::Driver)
        {
            DriverSeat = Seat;
            break;
        }
    if (!IsValid(DriverSeat))
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP passenger system '%s': Driver Class is set but no Driver seat exists."),
            *GetOwner()->GetName());
        return;
    }
    FActorSpawnParameters Parameters;
    Parameters.Owner = GetOwner();
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnedDriver = GetWorld()->SpawnActor<ACharacter>(DriverClass,
        DriverSeat->GetComponentTransform(), Parameters);
    if (IsValid(SpawnedDriver))
    {
        DriverSeat->EnterCharacterSeat(SpawnedDriver);
        UE_LOG(LogTemp, Display, TEXT("TMOP passenger system '%s': driver '%s' spawned in seat '%s'."),
            *GetOwner()->GetName(), *SpawnedDriver->GetName(), *DriverSeat->SeatId.ToString());
    }
}

bool UTMOPBusPassengerComponent::IsCharacterAboard(const ACharacter* Character) const
{
    if (!IsValid(Character)) return false;
    for (const ACharacter* Passenger : FreePassengers)
        if (Passenger == Character) return true;
    TArray<UTMOPVehicleSeatComponent*> Seats;
    if (GetOwner() != nullptr) GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    for (const UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->GetOccupantCharacter() == Character) return true;
    return false;
}

void UTMOPBusPassengerComponent::RegisterFreePassenger(ACharacter* Character)
{
    if (IsValid(Character) && Character != SpawnedDriver)
        FreePassengers.AddUnique(Character);
}

void UTMOPBusPassengerComponent::UnregisterFreePassenger(ACharacter* Character)
{
    FreePassengers.Remove(Character);
}

UTMOPVehicleSeatComponent* UTMOPBusPassengerComponent::FindAvailablePassengerSeat(
    const FName PreferredSeatId) const
{
    if (GetOwner() == nullptr) return nullptr;
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    Seats.Sort([](const UTMOPVehicleSeatComponent& Left, const UTMOPVehicleSeatComponent& Right)
        { return Left.SeatId.LexicalLess(Right.SeatId); });
    if (!PreferredSeatId.IsNone())
        for (UTMOPVehicleSeatComponent* Seat : Seats)
            if (IsValid(Seat) && Seat->SeatId == PreferredSeatId &&
                Seat->SeatRole != ETMOPVehicleSeatRole::Driver && !Seat->IsOccupied()) return Seat;
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->SeatRole != ETMOPVehicleSeatRole::Driver &&
            !Seat->IsOccupied()) return Seat;
    return nullptr;
}

bool UTMOPBusPassengerComponent::SeatCharacter(ACharacter* Character,
    const FName PreferredSeatId)
{
    UTMOPVehicleSeatComponent* Seat = FindAvailablePassengerSeat(PreferredSeatId);
    if (!IsValid(Seat) || !IsValid(Character)) return false;
    UnregisterFreePassenger(Character);
    return Seat->EnterCharacterSeat(Character);
}

bool UTMOPBusPassengerComponent::UnseatCharacter(ACharacter* Character)
{
    if (GetOwner() == nullptr || !IsValid(Character)) return false;
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->GetOccupantCharacter() == Character)
        {
            const bool bExited = Seat->ExitCharacterSeat(Character);
            if (bExited) RegisterFreePassenger(Character);
            return bExited;
        }
    return false;
}

bool UTMOPBusPassengerComponent::ToggleNearestSeat(ACharacter* Character)
{
    if (GetOwner() == nullptr || !IsValid(Character)) return false;
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->GetOccupantCharacter() == Character)
            return UnseatCharacter(Character);

    UTMOPVehicleSeatComponent* Nearest = nullptr;
    float NearestSquared = FMath::Square(PlayerSeatInteractionRadiusCm);
    for (UTMOPVehicleSeatComponent* Seat : Seats)
    {
        if (!IsValid(Seat) || Seat->IsOccupied() ||
            Seat->SeatRole == ETMOPVehicleSeatRole::Driver) continue;
        const float DistanceSquared = FVector::DistSquared(
            Character->GetActorLocation(), Seat->GetComponentLocation());
        if (DistanceSquared < NearestSquared ||
            (FMath::IsNearlyEqual(DistanceSquared, NearestSquared) &&
                IsValid(Nearest) && Seat->SeatId.LexicalLess(Nearest->SeatId)))
        {
            Nearest = Seat;
            NearestSquared = DistanceSquared;
        }
    }
    if (!IsValid(Nearest)) return false;
    UnregisterFreePassenger(Character);
    return Nearest->EnterCharacterSeat(Character);
}

bool UTMOPBusPassengerComponent::InitializePassengerManifest(
    UTMOPBusPassengerManifest* Manifest, const FName RunId)
{
    PassengerManifest = Manifest;
    ServiceRunId = RunId;
    JourneyStates.Reset();
    MissingAgentWarnings.Reset();
    if (!IsValid(PassengerManifest)) return true;
    TArray<FString> Errors;
    if (!PassengerManifest->ValidateManifest(Errors))
    {
        for (const FString& Error : Errors)
            UE_LOG(LogTemp, Error, TEXT("TMOP bus manifest '%s': %s"),
                *PassengerManifest->GetName(), *Error);
        return false;
    }
    JourneyStates.Init(ETMOPBusPassengerJourneyState::WaitingToBoard,
        PassengerManifest->Journeys.Num());
    AssignManifestDriver();
    UE_LOG(LogTemp, Display, TEXT("TMOP bus '%s': manifest '%s' initialized with %d documented passenger(s)."),
        *ServiceRunId.ToString(), *PassengerManifest->GetName(), JourneyStates.Num());
    return true;
}

bool UTMOPBusPassengerComponent::AssignScheduledDriver(
    const FName DriverEntityId, const FName RunId)
{
    ServiceRunId = RunId;
    if (DriverEntityId.IsNone()) return true;

    ATMOPHistoricalAgent* Driver = ResolveHistoricalAgent(DriverEntityId);
    if (!IsValid(Driver))
    {
        Driver = Cast<ATMOPHistoricalAgent>(SpawnedDriver);
        if (!IsValid(Driver))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': DriverClass must derive from TMOPHistoricalAgent "
                     "to use scheduled driver '%s'."),
                *RunId.ToString(), *DriverEntityId.ToString());
            return false;
        }

        if (!IsValid(Driver->EntityIdentity) ||
            !Driver->EntityIdentity->SetEntityIdentity(DriverEntityId, TEXT("Person")))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': failed to assign EntityId '%s' to driver."),
                *RunId.ToString(), *DriverEntityId.ToString());
            return false;
        }

        UTMOPPersonProfileComponent* Profile =
            Driver->FindComponentByClass<UTMOPPersonProfileComponent>();
        if (!IsValid(Profile))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': scheduled DriverClass lacks TMOPPersonProfileComponent."),
                *RunId.ToString());
            return false;
        }
        Profile->EntityIdOverride = DriverEntityId;
        if (!Profile->LoadProfile())
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': driver '%s' is missing from DT_TMOP_People."),
                *RunId.ToString(), *DriverEntityId.ToString());
            return false;
        }

        UGameInstance* GameInstance = GetWorld() != nullptr
            ? GetWorld()->GetGameInstance() : nullptr;
        UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
            ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
        if (Registry == nullptr ||
            !Registry->RegisterActiveAgent(DriverEntityId, Driver))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': driver '%s' could not be registered."),
                *RunId.ToString(), *DriverEntityId.ToString());
            return false;
        }
    }

    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->SeatRole == ETMOPVehicleSeatRole::Driver)
        {
            if (Seat->GetOccupantCharacter() == Driver)
            {
                ManifestDriver = Driver;
                return true;
            }
            if (Seat->IsOccupied() && IsValid(SpawnedDriver))
            {
                Seat->ExitCharacterSeat(SpawnedDriver);
                if (SpawnedDriver != Driver) SpawnedDriver->Destroy();
                SpawnedDriver = Driver;
            }
            if (!Seat->EnterCharacterSeat(Driver)) return false;
            ManifestDriver = Driver;
            UE_LOG(LogTemp, Display,
                TEXT("TMOP bus '%s': scheduled driver '%s' seated."),
                *RunId.ToString(), *DriverEntityId.ToString());
            return true;
        }

    UE_LOG(LogTemp, Error,
        TEXT("TMOP bus '%s': no driver seat exists for scheduled driver '%s'."),
        *RunId.ToString(), *DriverEntityId.ToString());
    return false;
}

ATMOPHistoricalAgent* UTMOPBusPassengerComponent::ResolveHistoricalAgent(
    const FName EntityId) const
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPWorldSubsystem* World = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPWorldSubsystem>() : nullptr;
    return World != nullptr ? Cast<ATMOPHistoricalAgent>(World->FindWorldObject(EntityId)) : nullptr;
}

void UTMOPBusPassengerComponent::AssignManifestDriver()
{
    if (!IsValid(PassengerManifest) || PassengerManifest->DriverEntityId.IsNone() ||
        GetOwner() == nullptr) return;
    ATMOPHistoricalAgent* Agent = ResolveHistoricalAgent(PassengerManifest->DriverEntityId);
    if (!IsValid(Agent))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': documented driver '%s' is not registered; no replacement was created."),
            *ServiceRunId.ToString(), *PassengerManifest->DriverEntityId.ToString());
        return;
    }
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetOwner()->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->SeatRole == ETMOPVehicleSeatRole::Driver)
        {
            if (Seat->IsOccupied() && IsValid(SpawnedDriver))
            {
                Seat->ExitCharacterSeat(SpawnedDriver);
                SpawnedDriver->Destroy();
                SpawnedDriver = nullptr;
            }
            if (Seat->EnterCharacterSeat(Agent)) ManifestDriver = Agent;
            return;
        }
    UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': documented driver exists but Driver seat is missing."),
        *ServiceRunId.ToString());
}

void UTMOPBusPassengerComponent::ProcessPassengerManifest()
{
    if (!IsValid(PassengerManifest) || JourneyStates.Num() != PassengerManifest->Journeys.Num() ||
        GetOwner() == nullptr) return;
    UTMOPBusServiceComponent* Service = GetOwner()->FindComponentByClass<UTMOPBusServiceComponent>();
    if (!IsValid(Service) || !Service->bDoorsOpen) return;
    UTMOPBusStopComponent* Stop = Service->GetCurrentTargetStop();
    if (!IsValid(Stop)) return;

    for (int32 Index = 0; Index < JourneyStates.Num(); ++Index)
        if (JourneyStates[Index] == ETMOPBusPassengerJourneyState::Aboard &&
            PassengerManifest->Journeys[Index].AlightingStopId == Stop->StopId)
            AlightManifestPassenger(Index, Stop);
    for (int32 Index = 0; Index < JourneyStates.Num(); ++Index)
        if (JourneyStates[Index] == ETMOPBusPassengerJourneyState::WaitingToBoard &&
            PassengerManifest->Journeys[Index].BoardingStopId == Stop->StopId)
            BoardManifestPassenger(Index, Stop);
}

bool UTMOPBusPassengerComponent::BoardManifestPassenger(
    const int32 JourneyIndex, UTMOPBusStopComponent* Stop)
{
    if (!PassengerManifest->Journeys.IsValidIndex(JourneyIndex) || !IsValid(Stop)) return false;
    const FTMOPBusPassengerJourney& Journey = PassengerManifest->Journeys[JourneyIndex];
    ATMOPHistoricalAgent* Agent = ResolveHistoricalAgent(Journey.PassengerEntityId);
    if (!IsValid(Agent))
    {
        if (!MissingAgentWarnings.Contains(JourneyIndex))
        {
            MissingAgentWarnings.Add(JourneyIndex);
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': documented passenger '%s' is not registered at boarding stop '%s'; no replacement was created."),
                *ServiceRunId.ToString(), *Journey.PassengerEntityId.ToString(), *Stop->StopId.ToString());
        }
        return false;
    }
    if (FVector::DistSquared2D(Agent->GetActorLocation(), Stop->GetBoardingWorldLocation()) >
        FMath::Square(Stop->PassengerWaitingRadiusCm)) return false;
    if (AController* Controller = Agent->GetController()) Controller->StopMovement();

    bool bBoarded = false;
    if (Journey.Placement == ETMOPBusPassengerPlacement::Standing)
    {
        Agent->SetActorLocation(PassengerVolume->GetComponentLocation(), false, nullptr,
            ETeleportType::TeleportPhysics);
        Agent->SetActivityState(ETMOPAgentActivityState::RidingVehicle);
        RegisterFreePassenger(Agent);
        bBoarded = true;
    }
    else
    {
        const FName PreferredSeat = Journey.Placement == ETMOPBusPassengerPlacement::AssignedSeat
            ? Journey.AssignedSeatId : NAME_None;
        bBoarded = SeatCharacter(Agent, PreferredSeat);
    }
    if (!bBoarded)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': passenger '%s' could not occupy the documented placement."),
            *ServiceRunId.ToString(), *Journey.PassengerEntityId.ToString());
        JourneyStates[JourneyIndex] = ETMOPBusPassengerJourneyState::Error;
        return false;
    }
    JourneyStates[JourneyIndex] = ETMOPBusPassengerJourneyState::Aboard;
    UE_LOG(LogTemp, Display, TEXT("TMOP bus '%s': historical agent '%s' boarded at '%s'."),
        *ServiceRunId.ToString(), *Journey.PassengerEntityId.ToString(), *Stop->StopId.ToString());
    return true;
}

bool UTMOPBusPassengerComponent::AlightManifestPassenger(
    const int32 JourneyIndex, UTMOPBusStopComponent* Stop)
{
    if (!PassengerManifest->Journeys.IsValidIndex(JourneyIndex) || !IsValid(Stop)) return false;
    const FTMOPBusPassengerJourney& Journey = PassengerManifest->Journeys[JourneyIndex];
    ATMOPHistoricalAgent* Agent = ResolveHistoricalAgent(Journey.PassengerEntityId);
    if (!IsValid(Agent)) return false;
    UnseatCharacter(Agent);
    UnregisterFreePassenger(Agent);
    Agent->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Agent->SetActorEnableCollision(true);
    Agent->SetActorLocation(Stop->GetAlightingWorldLocation(), false, nullptr,
        ETeleportType::TeleportPhysics);
    Agent->SetActivityState(ETMOPAgentActivityState::Standing);
    JourneyStates[JourneyIndex] = ETMOPBusPassengerJourneyState::Completed;
    UE_LOG(LogTemp, Display, TEXT("TMOP bus '%s': historical agent '%s' alighted at '%s'."),
        *ServiceRunId.ToString(), *Journey.PassengerEntityId.ToString(), *Stop->StopId.ToString());
    return true;
}
