#include "People/TMOPPersonProfileComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"

UTMOPPersonProfileComponent::UTMOPPersonProfileComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPPersonProfileComponent::BeginPlay()
{
    Super::BeginPlay();
    ResolvedEntityId = ResolveEntityId();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    if (Registry != nullptr)
    {
        Registry->RegisterActiveAgent(ResolvedEntityId, Cast<ATMOPHistoricalAgent>(GetOwner()));
        LoadProfile();
    }
}

void UTMOPPersonProfileComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    if (Registry != nullptr)
        Registry->UnregisterActiveAgent(ResolvedEntityId, Cast<ATMOPHistoricalAgent>(GetOwner()));
    Super::EndPlay(EndPlayReason);
}

FName UTMOPPersonProfileComponent::ResolveEntityId() const
{
    if (!EntityIdOverride.IsNone()) return EntityIdOverride;
    const ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    return Agent != nullptr && Agent->EntityIdentity != nullptr
        ? Agent->EntityIdentity->EntityId : NAME_None;
}

bool UTMOPPersonProfileComponent::LoadProfile()
{
    ResolvedEntityId = ResolveEntityId();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    bHasLoadedProfile = Registry != nullptr &&
        Registry->GetPersonProfile(ResolvedEntityId, Profile);
    if (bHasLoadedProfile)
        if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner()))
        {
            Agent->DisplayName = Profile.FullName;
            Agent->RefreshNameLabel();
        }
    return bHasLoadedProfile;
}
