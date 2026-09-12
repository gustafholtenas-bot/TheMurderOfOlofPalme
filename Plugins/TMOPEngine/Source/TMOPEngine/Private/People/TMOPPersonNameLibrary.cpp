#include "People/TMOPPersonNameLibrary.h"
#include "TMOPPersonNameFormatting.h"

namespace
{
using FNameString = std::basic_string<TCHAR>;
FNameString ToNameString(const FText& Text)
{
    const FString S = Text.ToString();
    return FNameString(*S, S.Len());
}
bool NameLetter(TCHAR Ch) { return FChar::IsAlpha(Ch); }
TCHAR NameUpper(TCHAR Ch) { return FChar::ToUpper(Ch); }
FText AsText(const FNameString& S) { return FText::FromString(FString(S.c_str())); }
}

FText UTMOPPersonNameLibrary::FormatPersonName(const FText& FullName, const FText& FirstName, const FText& LastName)
{
    return AsText(TMOPPersonNames::Format(ToNameString(FullName), ToNameString(FirstName),
        ToNameString(LastName), NameLetter, NameUpper));
}
FText UTMOPPersonNameLibrary::FormatUnstructuredPersonName(const FText& Name)
{
    return AsText(TMOPPersonNames::Fallback(ToNameString(Name), NameLetter, NameUpper));
}
FText UTMOPPersonNameLibrary::GetSurnameInitial(const FText& Surname)
{
    return AsText(TMOPPersonNames::Initial(ToNameString(Surname), NameLetter, NameUpper));
}
