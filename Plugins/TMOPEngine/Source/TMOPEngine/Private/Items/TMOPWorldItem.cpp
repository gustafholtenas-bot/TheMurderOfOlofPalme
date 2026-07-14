#include "Items/TMOPWorldItem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"

ATMOPWorldItem::ATMOPWorldItem()
{
    PrimaryActorTick.bCanEverTick = false;
    InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
    SetRootComponent(InteractionCollision);
    InteractionCollision->SetSphereRadius(35.0f);
    InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(InteractionCollision);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATMOPWorldItem::BeginPlay()
{
    Super::BeginPlay();
    Quantity = FMath::Max(1, Quantity);
    RefreshVisual();
}

void ATMOPWorldItem::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RefreshVisual();
}

void ATMOPWorldItem::ConfigureWorldItem(UTMOPItemDefinition* NewItem,
    const int32 NewQuantity)
{
    ItemDefinition = NewItem;
    Quantity = FMath::Max(1, NewQuantity);
    RefreshVisual();
}

void ATMOPWorldItem::RefreshVisual()
{
    if (bUseDefinitionMesh && IsValid(ItemDefinition.Get()) && IsValid(ItemMesh.Get()))
    {
        ItemMesh->SetStaticMesh(ItemDefinition->RightHandMesh.Get());
        ItemMesh->SetRelativeTransform(ItemDefinition->RightHandTransform);
    }
}

bool ATMOPWorldItem::TryPickup(UTMOPInventoryComponent* TargetInventory)
{
    if (!IsValid(TargetInventory) || !IsValid(ItemDefinition.Get())) return false;
    const int32 Transfer = FMath::Min(Quantity,
        TargetInventory->GetRemainingCapacity(ItemDefinition.Get()));
    if (Transfer <= 0 || !TargetInventory->AddItem(ItemDefinition.Get(), Transfer)) return false;
    Quantity -= Transfer;
    if (Quantity <= 0) Destroy();
    return true;
}

bool ATMOPWorldItem::Interact_Implementation(AActor* Interactor)
{
    UTMOPInventoryComponent* TargetInventory = IsValid(Interactor)
        ? Interactor->FindComponentByClass<UTMOPInventoryComponent>() : nullptr;
    return TryPickup(TargetInventory);
}

FText ATMOPWorldItem::GetInteractionText_Implementation() const
{
    const FText Name = IsValid(ItemDefinition.Get())
        ? ItemDefinition->DisplayName : NSLOCTEXT("TMOP", "UnknownItem", "Föremål");
    return FText::Format(NSLOCTEXT("TMOP", "PickupPrompt", "Plocka upp {0}"), Name);
}
