#include "Entities/TMOPWorldEntity.h"

#include "Components/SceneComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"

ATMOPWorldEntity::ATMOPWorldEntity()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* SceneRoot =
        CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    EntityIdentity =
        CreateDefaultSubobject<UTMOPWorldEntityComponent>(TEXT("EntityIdentity"));
}
