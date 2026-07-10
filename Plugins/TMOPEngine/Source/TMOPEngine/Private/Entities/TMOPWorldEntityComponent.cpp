#include "Entities/TMOPWorldEntityComponent.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "World/TMOPWorldSubsystem.h"

UTMOPWorldEntityComponent::UTMOPWorldEntityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPWorldEntityComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoRegister)
    {
        RegisterEntity();
    }
}

void UTMOPWorldEntityComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    UnregisterEntity();
    Super::EndPlay(EndPlayReason);
}

bool UTMOPWorldEntityComponent::RegisterEntity()
{
    if (bRegistered)
    {
        return true;
    }

    AActor* OwnerActor = GetOwner();
    UWorld* World = GetWorld();

    if (!IsValid(OwnerActor) || World == nullptr || EntityId.IsNone())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("TMOP entity registration failed for '%s': missing owner, world, or EntityId."),
            *GetNameSafe(OwnerActor));

        OnRegistrationChanged.Broadcast(EntityId, false);
        return false;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (GameInstance == nullptr)
    {
        OnRegistrationChanged.Broadcast(EntityId, false);
        return false;
    }

    UTMOPWorldSubsystem* WorldSubsystem =
        GameInstance->GetSubsystem<UTMOPWorldSubsystem>();

    if (WorldSubsystem == nullptr)
    {
        OnRegistrationChanged.Broadcast(EntityId, false);
        return false;
    }

    bRegistered = WorldSubsystem->RegisterWorldObject(
        EntityId,
        EntityType,
        OwnerActor);

    OnRegistrationChanged.Broadcast(EntityId, bRegistered);
    return bRegistered;
}

bool UTMOPWorldEntityComponent::UnregisterEntity()
{
    if (!bRegistered)
    {
        return true;
    }

    AActor* OwnerActor = GetOwner();
    UWorld* World = GetWorld();

    if (World == nullptr || World->GetGameInstance() == nullptr)
    {
        bRegistered = false;
        OnRegistrationChanged.Broadcast(EntityId, false);
        return false;
    }

    UTMOPWorldSubsystem* WorldSubsystem =
        World->GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>();

    const bool bRemoved =
        WorldSubsystem != nullptr &&
        WorldSubsystem->UnregisterWorldObject(EntityId, OwnerActor);

    bRegistered = false;
    OnRegistrationChanged.Broadcast(EntityId, false);
    return bRemoved;
}

bool UTMOPWorldEntityComponent::IsEntityRegistered() const
{
    if (!bRegistered)
    {
        return false;
    }

    const UWorld* World = GetWorld();
    if (World == nullptr || World->GetGameInstance() == nullptr)
    {
        return false;
    }

    const UTMOPWorldSubsystem* WorldSubsystem =
        World->GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>();

    return WorldSubsystem != nullptr &&
        WorldSubsystem->FindWorldObject(EntityId) == GetOwner();
}

bool UTMOPWorldEntityComponent::SetEntityIdentity(
    const FName NewEntityId,
    const FName NewEntityType)
{
    if (bRegistered || NewEntityId.IsNone())
    {
        return false;
    }

    EntityId = NewEntityId;
    EntityType = NewEntityType.IsNone()
        ? FName(TEXT("Entity"))
        : NewEntityType;

    return true;
}
