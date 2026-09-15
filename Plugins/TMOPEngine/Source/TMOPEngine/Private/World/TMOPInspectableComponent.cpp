#include "World/TMOPInspectableComponent.h"

#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"

namespace
{
TArray<TWeakObjectPtr<UTMOPInspectableComponent>> ActiveInspectableComponents;
}

UTMOPInspectableComponent::UTMOPInspectableComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

FText UTMOPInspectableComponent::GetWorldIndicatorTextAt(const FVector& ViewLocation) const
{ return WorldIndicatorText; }
float UTMOPInspectableComponent::GetWorldIndicatorSizeAt(const FVector& ViewLocation) const
{ return WorldIndicatorSize; }
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

FVector UTMOPInspectableComponent::GetWorldIndicatorLocation() const
{
    return GetInteractionLocation() + (IsValid(GetOwner())
        ? GetOwner()->GetActorTransform().TransformVectorNoScale(WorldIndicatorOffset)
        : WorldIndicatorOffset);
}

bool UTMOPInspectableComponent::ShouldShowWorldIndicatorAt(
    const FVector& ViewLocation) const
{
    return bShowWorldIndicator && HasReadableContent() &&
        FVector::DistSquared(ViewLocation, GetWorldIndicatorLocation()) <=
        FMath::Square(FMath::Max(WorldIndicatorMaxDistanceCm, 100.0f));
}

void UTMOPInspectableComponent::GetActiveInWorld(const UWorld* World,
    TArray<UTMOPInspectableComponent*>& OutComponents)
{
    OutComponents.Reset();
    ActiveInspectableComponents.RemoveAll([](const auto& WeakComponent)
    {
        return !WeakComponent.IsValid();
    });
    if (!World) return;
    for (const auto& WeakComponent : ActiveInspectableComponents)
        if (UTMOPInspectableComponent* Component = WeakComponent.Get();
            IsValid(Component) && Component->GetWorld() == World)
            OutComponents.Add(Component);
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
    ActiveInspectableComponents.AddUnique(this);
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
    // Object-overlap target queries must be able to discover otherwise
    // collision-free address and information anchors.
    InteractionShape->SetCollisionResponseToAllChannels(ECR_Overlap);
    InteractionShape->SetGenerateOverlapEvents(false);
    InteractionShape->SetCanEverAffectNavigation(false);
    InteractionShape->SetHiddenInGame(true);
    InteractionShape->RegisterComponent();
    InteractionShape->SetWorldLocation(GetInteractionLocation());

    WorldIndicator = NewObject<UTextRenderComponent>(Owner, NAME_None, RF_Transient);
    WorldIndicator->SetMobility(EComponentMobility::Movable);
    WorldIndicator->SetupAttachment(Owner->GetRootComponent());
    WorldIndicator->SetAbsolute(false, true, true);
    WorldIndicator->SetHorizontalAlignment(EHTA_Center);
    WorldIndicator->SetVerticalAlignment(EVRTA_TextCenter);
    WorldIndicator->SetWorldSize(WorldIndicatorSize);
    WorldIndicator->SetTextRenderColor(WorldIndicatorColor);
    WorldIndicator->SetText(WorldIndicatorText);
    WorldIndicator->SetCastShadow(false);
    WorldIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WorldIndicator->SetGenerateOverlapEvents(false);
    WorldIndicator->SetCanEverAffectNavigation(false);
    WorldIndicator->SetVisibility(false, true);
    WorldIndicator->RegisterComponent();
    WorldIndicator->SetWorldLocation(GetWorldIndicatorLocation());
}

void UTMOPInspectableComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!IsValid(WorldIndicator.Get()) || !GetWorld()) return;

    // Multiplayer redraws this marker in each player's own Slate overlay.
    if (UTMOPLocalMultiplayerSubsystem::IsMultiplayer(this))
    {
        WorldIndicator->SetVisibility(false, true);
        return;
    }

    const APlayerController* Controller = GetWorld()->GetFirstPlayerController();
    const APlayerCameraManager* Camera = IsValid(Controller)
        ? Controller->PlayerCameraManager : nullptr;
    const bool bVisible = IsValid(Camera) &&
        ShouldShowWorldIndicatorAt(Camera->GetCameraLocation());
    WorldIndicator->SetVisibility(bVisible, true);
    if (!bVisible) return;

    WorldIndicator->SetWorldLocation(GetWorldIndicatorLocation());
    WorldIndicator->SetWorldSize(GetWorldIndicatorSizeAt(Camera->GetCameraLocation()));
    WorldIndicator->SetTextRenderColor(WorldIndicatorColor);
    WorldIndicator->SetText(FText::FromString(GetWorldIndicatorTextAt(Camera->GetCameraLocation()).ToString().Replace(TEXT("\n"), TEXT("<br>"))));
    const FVector ToCamera = Camera->GetCameraLocation() -
        WorldIndicator->GetComponentLocation();
    if (!ToCamera.IsNearlyZero()) WorldIndicator->SetWorldRotation(ToCamera.Rotation());
}

void UTMOPInspectableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ActiveInspectableComponents.RemoveAll([this](const auto& WeakComponent)
    {
        return WeakComponent.Get() == this;
    });
    if (IsValid(InteractionShape.Get())) InteractionShape->DestroyComponent();
    InteractionShape = nullptr;
    if (IsValid(WorldIndicator.Get())) WorldIndicator->DestroyComponent();
    WorldIndicator = nullptr;
    Super::EndPlay(EndPlayReason);
}

