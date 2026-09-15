#include "Addresses/TMOPAddressRegistryTypes.h"
#include "People/TMOPPersonNameLibrary.h"

FString TMOPAddressDisplay::Resident(const FTMOPAddressResident& Person)
{
    if (!Person.InGameDisplayName.TrimStartAndEnd().IsEmpty())
        return UTMOPPersonNameLibrary::FormatUnstructuredPersonName(FText::FromString(Person.InGameDisplayName)).ToString();
    if (Person.bAllowArchivalNameInGame)
        return UTMOPPersonNameLibrary::FormatUnstructuredPersonName(FText::FromString(Person.ArchivalFullName)).ToString();
    TArray<FString> Parts;
    Person.ArchivalFullName.TrimStartAndEnd().ParseIntoArrayWS(Parts);
    if (Parts.Num() < 2) return Parts.IsEmpty() ? TEXT("Okänd boende") : Parts[0];
    return Parts[0].Left(1) + TEXT(". ") + UTMOPPersonNameLibrary::GetSurnameInitial(FText::FromString(Parts.Last())).ToString();
}

FString TMOPAddressDisplay::Household(const FTMOPAddressHousehold& Home)
{
    if (Home.bConfirmedFamily && !Home.FamilySurname.TrimStartAndEnd().IsEmpty())
        return TEXT("Familjen ") + UTMOPPersonNameLibrary::GetSurnameInitial(FText::FromString(Home.FamilySurname)).ToString();
    TArray<FString> Names;
    for (const auto& Person : Home.Residents) Names.Add(Resident(Person));
    return Names.IsEmpty() ? TEXT("Inga registrerade boende") : FString::Join(Names, TEXT(", "));
}

FString TMOPAddressDisplay::Directory(const FTMOPAddressRegistryRow& Row)
{
    FString Out = FString::Printf(TEXT("%s %d%s\n"), *Row.StreetName, Row.StreetNumber, *Row.EntranceSuffix);
    if (!Row.BuildingDescription.IsEmpty()) Out += Row.BuildingDescription + TEXT("\n\n");
    if (!Row.Businesses.IsEmpty()) Out += TEXT("VERKSAMHETER\n");
    for (const auto& Business : Row.Businesses)
    {
        Out += Business.Name;
        if (!Business.FloorLabel.IsEmpty()) Out += TEXT(" — ") + Business.FloorLabel;
        Out += TEXT("\n");
        for (const FString* Text : { &Business.Description1986, &Business.History })
            if (!Text->IsEmpty()) Out += *Text + TEXT("\n");
        if (!Business.PresentDayNote.IsEmpty())
        {
            Out += TEXT("Senare uppgifter");
            if (!Business.PresentDayVerifiedDate.IsEmpty()) Out += TEXT(" (") + Business.PresentDayVerifiedDate + TEXT(")");
            Out += TEXT(": ") + Business.PresentDayNote + TEXT("\n");
        }
        if (!Business.SourceReference.IsEmpty()) Out += TEXT("Källa: ") + Business.SourceReference + TEXT("\n");
        Out += TEXT("\n");
    }
    if (!Row.Households.IsEmpty()) Out += TEXT("BOENDE\n");
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
        FString Floor = H.FloorLabel;
        if (Floor.IsEmpty()) Floor = H.FloorNumber == -1 ? TEXT("Våning okänd") : FString::Printf(TEXT("Våning %d"), H.FloorNumber);
        Out += Floor + (H.ApartmentLabel.IsEmpty() ? TEXT("") : TEXT(" / ") + H.ApartmentLabel);
        Out += TEXT(" — ") + Household(H) + TEXT("\n");
    }
    return Out;
}

