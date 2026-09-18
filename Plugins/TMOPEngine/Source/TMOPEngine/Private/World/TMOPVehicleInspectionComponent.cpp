#include "World/TMOPVehicleInspectionComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Observations/TMOPNotebookTypes.h"

UTMOPVehicleInspectionComponent::UTMOPVehicleInspectionComponent()
{
    bShowWorldIndicator = false;
}
bool UTMOPVehicleInspectionComponent::HasReadableContent() const
{
    const auto* Vehicle = Cast<ATMOPVehicleBase>(GetOwner());
    return bInteractionEnabled && Vehicle && !Vehicle->VehicleId.IsNone() &&
        TMOPNotebook::IsVehicleEligible(Vehicle->VehicleId.ToString(), Vehicle->VehicleCategoryId.ToString());
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
