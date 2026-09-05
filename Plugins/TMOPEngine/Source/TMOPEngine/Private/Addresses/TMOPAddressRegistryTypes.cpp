#include "Addresses/TMOPAddressRegistryTypes.h"

FString TMOPAddressDisplay::Resident(const FTMOPAddressResident& Person)
{
    if (!Person.InGameDisplayName.TrimStartAndEnd().IsEmpty()) return Person.InGameDisplayName;
    if (Person.bAllowArchivalNameInGame) return Person.ArchivalFullName;
    TArray<FString> Parts;
    Person.ArchivalFullName.TrimStartAndEnd().ParseIntoArrayWS(Parts);
    if (Parts.Num() < 2) return Parts.IsEmpty() ? TEXT("Okänd boende") : Parts[0];
    return Parts[0].Left(1) + TEXT(". ") + Parts.Last();
}

FString TMOPAddressDisplay::Household(const FTMOPAddressHousehold& Home)
{
    if (Home.bConfirmedFamily && !Home.FamilySurname.TrimStartAndEnd().IsEmpty())
        return TEXT("Familjen ") + Home.FamilySurname.TrimStartAndEnd();
    TArray<FString> Names;
    for (const auto& Person : Home.Residents) Names.Add(Resident(Person));
    return Names.IsEmpty() ? TEXT("Inga registrerade boende") : FString::Join(Names, TEXT(", "));
}

FString TMOPAddressDisplay::Directory(const FTMOPAddressRegistryRow& Row)
{
    FString Out = FString::Printf(TEXT("%s %d%s\n"), *Row.StreetName, Row.StreetNumber, *Row.EntranceSuffix);
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
