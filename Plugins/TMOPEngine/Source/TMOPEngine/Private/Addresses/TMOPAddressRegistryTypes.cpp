#include "Addresses/TMOPAddressRegistryTypes.h"
#include "Localization/TMOPLocalization.h"
#include "People/TMOPPersonNameLibrary.h"

FString TMOPAddressDisplay::Resident(const FTMOPAddressResident& Person)
{
    if (!Person.InGameDisplayName.TrimStartAndEnd().IsEmpty())
        return UTMOPPersonNameLibrary::FormatUnstructuredPersonName(FText::FromString(Person.InGameDisplayName)).ToString();
    if (Person.bAllowArchivalNameInGame)
        return UTMOPPersonNameLibrary::FormatUnstructuredPersonName(FText::FromString(Person.ArchivalFullName)).ToString();
    TArray<FString> Parts;
    Person.ArchivalFullName.TrimStartAndEnd().ParseIntoArrayWS(Parts);
    if (Parts.Num() < 2) return Parts.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.32449c311d7fcd64", "Okänd boende").ToString() : Parts[0];
    return Parts[0].Left(1) + TEXT(". ") + UTMOPPersonNameLibrary::GetSurnameInitial(FText::FromString(Parts.Last())).ToString();
}

FString TMOPAddressDisplay::Household(const FTMOPAddressHousehold& Home)
{
    if (Home.bConfirmedFamily && !Home.FamilySurname.TrimStartAndEnd().IsEmpty())
        return NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.3e3e0499180f52ad", "Familjen ").ToString() + UTMOPPersonNameLibrary::GetSurnameInitial(FText::FromString(Home.FamilySurname)).ToString();
    TArray<FString> Names;
    for (const auto& Person : Home.Residents) Names.Add(Resident(Person));
    return Names.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.d8afaf0a791075b7", "Inga registrerade boende").ToString() : FString::Join(Names, TEXT(", "));
}

FString TMOPAddressDisplay::Directory(const FTMOPAddressRegistryRow& Row)
{
    FString Out = FString::Printf(TEXT("%s %d%s\n"), *Row.StreetName, Row.StreetNumber, *Row.EntranceSuffix);
    if (!Row.BuildingDescription.IsEmpty()) Out += FTMOPLocalization::String(Row.BuildingDescription) + TEXT("\n\n");
    if (!Row.Businesses.IsEmpty()) Out += NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.e099dceaf7e211ea", "VERKSAMHETER\n").ToString();
    for (const auto& Business : Row.Businesses)
    {
        Out += Business.Name;
        if (!Business.FloorLabel.IsEmpty()) Out += TEXT(" — ") + FTMOPLocalization::String(Business.FloorLabel);
        Out += TEXT("\n");
        for (const FString* Text : { &Business.Description1986, &Business.History })
            if (!Text->IsEmpty()) Out += FTMOPLocalization::String(*Text) + TEXT("\n");
        if (!Business.PresentDayNote.IsEmpty())
        {
            Out += NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.fc77d1a613780af0", "Senare uppgifter").ToString();
            if (!Business.PresentDayVerifiedDate.IsEmpty()) Out += TEXT(" (") + Business.PresentDayVerifiedDate + TEXT(")");
            Out += TEXT(": ") + FTMOPLocalization::String(Business.PresentDayNote) + TEXT("\n");
        }
        if (!Business.SourceReference.IsEmpty()) Out += NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.8e9a038ea09697ea", "Källa: ").ToString() + Business.SourceReference + TEXT("\n");
        Out += TEXT("\n");
    }
    if (!Row.Households.IsEmpty()) Out += NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.29a6ac71928b6f31", "BOENDE\n").ToString();
    TArray<int32> Order;
    for (int32 I = 0; I < Row.Households.Num(); ++I) Order.Add(I);
    Order.StableSort([&](int32 A, int32 B) {
        const int32 FA = Row.Households[A].FloorNumber;
        const int32 FB = Row.Households[B].FloorNumber;
        return (FA == -1 ? MAX_int32 : FA) < (FB == -1 ? MAX_int32 : FB);
    });
    for (int32 I : Order)
    {
        const auto& H = Row.Households[I];
        FString Floor = FTMOPLocalization::String(H.FloorLabel);
        if (Floor.IsEmpty()) Floor = H.FloorNumber == -1 ? NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.307968e4710163be", "Våning okänd").ToString() : FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPAddressRegistryTypes.8cd4c3a00d40883e", "Våning {0}"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), H.FloorNumber))).ToString();
        Out += Floor + (H.ApartmentLabel.IsEmpty() ? TEXT("") : TEXT(" / ") + H.ApartmentLabel);
        Out += TEXT(" — ") + Household(H) + TEXT("\n");
    }
    return Out;
}

