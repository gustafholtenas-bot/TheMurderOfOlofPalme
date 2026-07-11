#include "People/TMOPPersonRegistryDirector.h"

#include "EngineUtils.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"

ATMOPPersonRegistryDirector::ATMOPPersonRegistryDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPPersonRegistryDirector::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    if (Registry != nullptr && Registry->ConfigureProfileTable(PersonProfileTable))
        RefreshAllActiveProfiles();
    else UE_LOG(LogTemp, Error, TEXT("TMOP Person Registry Director has no valid profile table."));
}

int32 ATMOPPersonRegistryDirector::RefreshAllActiveProfiles()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    int32 Loaded = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPPersonProfileComponent*> Components;
        It->GetComponents<UTMOPPersonProfileComponent>(Components);
        for (UTMOPPersonProfileComponent* Component : Components)
            Loaded += IsValid(Component) && Component->LoadProfile() ? 1 : 0;
    }
    return Loaded;
}
