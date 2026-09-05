#include "Addresses/TMOPAddressComponent.h"
#include "Addresses/TMOPAddressRegistryTypes.h"
#include "Engine/DataTable.h"

UTMOPAddressComponent::UTMOPAddressComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}
FText UTMOPAddressComponent::GetResidentDirectory() const
{
    if (!Registry || Registry->GetRowStruct() != FTMOPAddressRegistryRow::StaticStruct()) return FText::GetEmpty();
    const auto* Row = Registry->FindRow<FTMOPAddressRegistryRow>(RowName, TEXT("Address directory"), false);
    return Row ? FText::FromString(TMOPAddressDisplay::Directory(*Row)) : FText::GetEmpty();
}
