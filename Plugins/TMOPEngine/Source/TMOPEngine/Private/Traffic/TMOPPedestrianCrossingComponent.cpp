#include "Traffic/TMOPPedestrianCrossingComponent.h"

UTMOPPedestrianCrossingComponent::UTMOPPedestrianCrossingComponent()
{
    // Constructor: initialize dimensions without creating/updating a physics BodySetup.
    InitBoxExtent(FVector(750, 200, 200));
    PrimaryComponentTick.bCanEverTick = false;
}
void UTMOPPedestrianCrossingComponent::OnRegister()
{
    // Runtime setters can create a BodySetup UObject. Never call them from the
    // constructor/CDO creation path. OnRegister runs after object construction.
    // Set flags before base registration to avoid enabling collision/navigation.
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetGenerateOverlapEvents(false);
    SetCanEverAffectNavigation(false);
    Super::OnRegister();
}
bool UTMOPPedestrianCrossingComponent::ContainsPedestrian(const FVector& Position) const
{
    const FVector P = GetComponentTransform().InverseTransformPosition(Position);
    const FVector E = GetUnscaledBoxExtent();
    return FMath::Abs(P.X) < E.X && FMath::Abs(P.Y) <= E.Y && FMath::Abs(P.Z) <= E.Z;
}
bool UTMOPPedestrianCrossingComponent::CrossesEntrance(const FVector& From, const FVector& To, float MarginCm) const
{
    const FVector A = GetComponentTransform().InverseTransformPosition(From);
    const FVector B = GetComponentTransform().InverseTransformPosition(To);
    const FVector E = GetUnscaledBoxExtent();
    if (FMath::Abs(A.X) < E.X) return false; // Never trap somebody already crossing.
    if (FMath::Abs(B.X - A.X) < KINDA_SMALL_NUMBER) return false;
    const double Side = A.X >= 0 ? 1.0 : -1.0;
    const double T = (Side * E.X - A.X) / (B.X - A.X);
    if (T < 0 || T > 1) return false;
    const FVector Hit = FMath::Lerp(A, B, T);
    const double ScaleY = FMath::Max(0.001, FMath::Abs(GetComponentScale().Y));
    return FMath::Abs(Hit.Y) <= E.Y + MarginCm / ScaleY && FMath::Abs(Hit.Z) <= E.Z;
}
