#include "Items/TMOPWorldItem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Player/TMOPPlayerCharacter.h"

ATMOPWorldItem::ATMOPWorldItem()
{
    PrimaryActorTick.bCanEverTick = true;
    InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
    SetRootComponent(InteractionCollision);
    InteractionCollision->SetSphereRadius(35.0f);
    InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(InteractionCollision);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    WorldPrompt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("WorldPrompt"));
    WorldPrompt->SetupAttachment(InteractionCollision);
    WorldPrompt->SetHorizontalAlignment(EHTA_Center);
    WorldPrompt->SetVerticalAlignment(EVRTA_TextCenter);
    WorldPrompt->SetWorldSize(24.0f);
    WorldPrompt->SetTextRenderColor(FColor(255, 220, 120));
    WorldPrompt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WorldPrompt->SetVisibility(false);
}

void ATMOPWorldItem::BeginPlay()
{
    Super::BeginPlay();
    Quantity = FMath::Max(1, Quantity);
    RefreshVisual();
    UpdateWorldPrompt();
}

void ATMOPWorldItem::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateWorldPrompt();
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

void ATMOPWorldItem::UpdateWorldPrompt()
{
    if (!IsValid(WorldPrompt.Get()) || GetWorld() == nullptr) return;
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    ATMOPPlayerCharacter* Player = IsValid(PC)
        ? Cast<ATMOPPlayerCharacter>(PC->GetPawn()) : nullptr;
    if (!IsValid(Player))
    {
        WorldPrompt->SetVisibility(false);
        return;
    }

    const float Distance = FVector::Dist(Player->GetActorLocation(), GetActorLocation());
    const bool bShow = Distance >= PromptMinimumDistanceCm
        && Distance <= PromptMaximumDistanceCm && IsValid(ItemDefinition.Get());
    WorldPrompt->SetVisibility(bShow);
    if (!bShow) return;

    WorldPrompt->SetRelativeLocation(FVector(0.0f, 0.0f, PromptHeightCm));
    WorldPrompt->SetText(FText::Format(
        NSLOCTEXT("TMOP", "WorldPickupPrompt", "[{0}] Plocka upp {1}"),
        Player->GetInteractKeyDisplayText(), ItemDefinition->DisplayName));
    const FVector ToPlayer = Player->GetPawnViewLocation() - WorldPrompt->GetComponentLocation();
    WorldPrompt->SetWorldRotation(ToPlayer.Rotation());
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
