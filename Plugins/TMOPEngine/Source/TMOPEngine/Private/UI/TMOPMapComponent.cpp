#include "UI/TMOPMapComponent.h"

UTMOPMapComponent::UTMOPMapComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FVector2D UTMOPMapComponent::WorldToMapUV(const FVector WorldLocation) const
{
    const FVector2D Size = WorldMaximum - WorldMinimum;
    FVector2D UV(
        FMath::IsNearlyZero(Size.X) ? 0.5f : (WorldLocation.X - WorldMinimum.X) / Size.X,
        FMath::IsNearlyZero(Size.Y) ? 0.5f : (WorldLocation.Y - WorldMinimum.Y) / Size.Y);
    if (bInvertImageY) UV.Y = 1.0f - UV.Y;
    return UV;
}

FVector UTMOPMapComponent::MapUVToWorld(FVector2D MapUV, const float WorldZ) const
{
    if (bInvertImageY) MapUV.Y = 1.0f - MapUV.Y;
    const FVector2D XY = WorldMinimum + MapUV * (WorldMaximum - WorldMinimum);
    return FVector(XY.X, XY.Y, WorldZ);
}

void UTMOPMapComponent::AddOrUpdateMarker(const FTMOPMapMarker& Marker)
{
    if (Marker.MarkerId.IsNone()) return;
    const int32 Index = Markers.IndexOfByPredicate([&Marker](const FTMOPMapMarker& Existing)
        { return Existing.MarkerId == Marker.MarkerId; });
    if (Index == INDEX_NONE) Markers.Add(Marker);
    else Markers[Index] = Marker;
}

bool UTMOPMapComponent::SetMarkerDiscovered(const FName MarkerId, const bool bDiscovered)
{
    FTMOPMapMarker* Marker = Markers.FindByPredicate([MarkerId](const FTMOPMapMarker& Existing)
        { return Existing.MarkerId == MarkerId; });
    if (Marker == nullptr) return false;
    Marker->bDiscovered = bDiscovered;
    return true;
}

FVector UTMOPMapComponent::GetTrackedWorldLocation() const
{
    return GetOwner() != nullptr ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

float UTMOPMapComponent::GetTrackedMapYawDegrees() const
{
    const float WorldYaw = GetOwner() != nullptr ? GetOwner()->GetActorRotation().Yaw : 0.0f;
    return FMath::FindDeltaAngleDegrees(MapNorthYawDegrees, WorldYaw);
}
