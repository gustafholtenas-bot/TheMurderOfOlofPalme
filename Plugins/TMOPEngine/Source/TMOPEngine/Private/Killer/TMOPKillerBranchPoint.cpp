#include "Killer/TMOPKillerBranchPoint.h"
#include "Components/BillboardComponent.h"

ATMOPKillerBranchPoint::ATMOPKillerBranchPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("Corner"));
    SetRootComponent(EditorIcon);
    EditorIcon->SetHiddenInGame(true);
    EditorIcon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    EditorIcon->SetCanEverAffectNavigation(false);
    SetActorEnableCollision(false);
}
