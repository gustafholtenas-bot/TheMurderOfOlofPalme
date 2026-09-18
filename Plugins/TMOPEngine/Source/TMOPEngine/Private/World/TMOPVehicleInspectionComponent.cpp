#include "World/TMOPVehicleInspectionComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Components/MeshComponent.h"
#include "Observations/TMOPNotebookTypes.h"

UTMOPVehicleInspectionComponent::UTMOPVehicleInspectionComponent()
{
    bShowWorldIndicator = false;
}
FVector UTMOPVehicleInspectionComponent::GetVehicleAimPoint(const AActor* Vehicle)
{
    if (!IsValid(Vehicle)) return FVector::ZeroVector;
    FBox Bounds(ForceInit);
    TArray<UMeshComponent*> Meshes;
    Vehicle->GetComponents<UMeshComponent>(Meshes);
    for (const UMeshComponent* Mesh : Meshes)
        if (IsValid(Mesh) && Mesh->IsRegistered() && Mesh->IsVisible())
            Bounds += Mesh->Bounds.GetBox();
    return Bounds.IsValid ? Bounds.GetCenter() : Vehicle->GetActorLocation();
}
FVector UTMOPVehicleInspectionComponent::GetInteractionLocation() const
{
    return GetVehicleAimPoint(GetOwner());
}
bool UTMOPVehicleInspectionComponent::HasReadableContent() const
{
    const auto* Vehicle = Cast<ATMOPVehicleBase>(GetOwner());
    return bInteractionEnabled && IsValid(Vehicle);
}
FText UTMOPVehicleInspectionComponent::GetInspectionTitle() const
{
    const auto* V = Cast<ATMOPVehicleBase>(GetOwner());
    return V && !V->DisplayName.IsEmpty() ? V->DisplayName : NSLOCTEXT("TMOP", "UnknownObservedCar", "Okänt fordon");
}
FText UTMOPVehicleInspectionComponent::GetInspectionText() const
{
    const auto* V = Cast<ATMOPVehicleBase>(GetOwner());
    if (V && !V->RegistrationNumber.IsEmpty())
        return FText::Format(NSLOCTEXT("TMOP", "ObservedCarRegistration", "Registreringsnummer: {0}"), FText::FromString(V->RegistrationNumber));
    return NSLOCTEXT("TMOP", "ObservedCarUnknownRegistration", "Registreringsnummer saknas i observationen.");
}
FText UTMOPVehicleInspectionComponent::GetInspectionCategory() const
{ return NSLOCTEXT("TMOP", "ObservedCarCategory", "Observerat fordon"); }
FText UTMOPVehicleInspectionComponent::GetInspectionSource() const
{
    const auto* V = Cast<ATMOPVehicleBase>(GetOwner());
    return V ? FText::FromString(V->SourceDocumentNumber) : FText::GetEmpty();
}
FText UTMOPVehicleInspectionComponent::GetInspectionAction() const
{ return NSLOCTEXT("TMOP", "InspectObservedCar", "Visa fordonsakt"); }
