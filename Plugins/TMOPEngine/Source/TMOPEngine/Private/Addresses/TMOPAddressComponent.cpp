#include "Addresses/TMOPAddressComponent.h"
#include "Addresses/TMOPAddressRegistryTypes.h"
#include "Engine/DataTable.h"

UTMOPAddressComponent::UTMOPAddressComponent()
{
    InteractionOffset = FVector(0.0f, 0.0f, 140.0f);
}
FText UTMOPAddressComponent::GetResidentDirectory() const
{
    const auto* Row = FindAddress();
    return Row ? FText::FromString(TMOPAddressDisplay::Directory(*Row)) : FText::GetEmpty();
}

const FTMOPAddressRegistryRow* UTMOPAddressComponent::FindAddress() const
{
    if (!IsValid(Registry.Get()) || RowName.IsNone() ||
        Registry->GetRowStruct() != FTMOPAddressRegistryRow::StaticStruct()) return nullptr;
    return Registry->FindRow<FTMOPAddressRegistryRow>(RowName, TEXT("Address directory"), false);
}

bool UTMOPAddressComponent::HasValidAddress() const
{
    return bInteractionEnabled && FindAddress() != nullptr;
}

FText UTMOPAddressComponent::GetAddressTitle() const
{
    const auto* Row = FindAddress();
    return Row ? FText::FromString(FString::Printf(TEXT("%s %d%s"),
        *Row->StreetName, Row->StreetNumber, *Row->EntranceSuffix)) : FText::GetEmpty();
}

bool UTMOPAddressComponent::HasReadableContent() const
{
    return HasValidAddress();
}

FText UTMOPAddressComponent::GetInspectionTitle() const
{
    return GetAddressTitle();
}

FText UTMOPAddressComponent::GetInspectionText() const
{
    FString Text = GetResidentDirectory().ToString();
    const FString Heading = GetAddressTitle().ToString() + TEXT("\n");
    if (Text.StartsWith(Heading)) Text.RightChopInline(Heading.Len());
    return Text.TrimStartAndEnd().IsEmpty()
        ? NSLOCTEXT("TMOP", "NoRegisteredResidents", "Inga registrerade boende.")
        : FText::FromString(Text);
}

FText UTMOPAddressComponent::GetInspectionCategory() const
{
    return NSLOCTEXT("TMOP", "AddressResidents", "Boende");
}

FText UTMOPAddressComponent::GetInspectionAction() const
{
    return NSLOCTEXT("TMOP", "ReadAddressDirectory", "Läs boendeförteckning");
}
