#include "World/TMOPInspectableComponent.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UTMOPInspectableComponent::UTMOPInspectableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPInspectableComponent::HasReadableContent() const { return false; }
FText UTMOPInspectableComponent::GetInspectionTitle() const { return FText::GetEmpty(); }
FText UTMOPInspectableComponent::GetInspectionText() const { return FText::GetEmpty(); }
FText UTMOPInspectableComponent::GetInspectionSource() const { return FText::GetEmpty(); }
FText UTMOPInspectableComponent::GetInspectionCategory() const
{
    return NSLOCTEXT("TMOP", "InspectionInformation", "Information");
}
FText UTMOPInspectableComponent::GetInspectionAction() const
{
    return NSLOCTEXT("TMOP", "ReadInformation", "Läs information");
}

FVector UTMOPInspectableComponent::GetInteractionLocation() const
{
    return IsValid(GetOwner())
        ? GetOwner()->GetActorTransform().TransformPositionNoScale(InteractionOffset)
        : FVector::ZeroVector;
}

bool UTMOPInspectableComponent::IsVisibleFrom(const FVector& ViewLocation, const AActor* Viewer) const
{
    if (!HasReadableContent() || !GetWorld() || !IsValid(GetOwner())) return false;
    const FVector End = GetInteractionLocation();
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPInspectionVisibility), false, Viewer);
    // Tolerate a reading point slightly embedded in its surface; intervening walls occlude it.
    const FVector Surface = End + (ViewLocation - End).GetSafeNormal() *
        FMath::Min<double>(FMath::Clamp(InteractionRadiusCm, 5.0f, 100.0f), FVector::Dist(ViewLocation, End));
    return !GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, Surface,
        ECC_Visibility, Params) || Hit.GetActor() == GetOwner();
}

void UTMOPInspectableComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->GetRootComponent()) return;
    // Keep the query shape even before a timed point becomes active. Availability
    // is evaluated when targeted and when reading, including after a clock seek.
    InteractionShape = NewObject<USphereComponent>(Owner, NAME_None, RF_Transient);
    InteractionShape->SetMobility(EComponentMobility::Movable);
    InteractionShape->SetupAttachment(Owner->GetRootComponent());
    InteractionShape->SetAbsolute(false, false, true);
    InteractionShape->InitSphereRadius(FMath::Clamp(InteractionRadiusCm, 5.0f, 100.0f));
    InteractionShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionShape->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionShape->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionShape->SetGenerateOverlapEvents(false);
    InteractionShape->SetCanEverAffectNavigation(false);
    InteractionShape->SetHiddenInGame(true);
    InteractionShape->RegisterComponent();
    InteractionShape->SetWorldLocation(GetInteractionLocation());
}

void UTMOPInspectableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(InteractionShape.Get())) InteractionShape->DestroyComponent();
    InteractionShape = nullptr;
    Super::EndPlay(EndPlayReason);
}
