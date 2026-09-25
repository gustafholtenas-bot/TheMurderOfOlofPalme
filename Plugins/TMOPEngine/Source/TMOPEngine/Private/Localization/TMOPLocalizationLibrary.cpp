#include "Localization/TMOPLocalizationLibrary.h"
#include "Localization/TMOPLocalization.h"

FText UTMOPLocalizationLibrary::LocalizeText(const FText& Source)
{
    return FTMOPLocalization::Text(Source);
}

FText UTMOPLocalizationLibrary::LocalizeString(const FString& Source)
{
    return FTMOPLocalization::Text(Source);
}

FText UTMOPLocalizationLibrary::LocalizeTableText(const FString& Table, FName Row, const FString& Field, const FText& Source)
{
    return FTMOPLocalization::TableText(Table, Row.ToString(), Field, Source);
}

FText UTMOPLocalizationLibrary::LocalizeTableString(const FString& Table, FName Row, const FString& Field, const FString& Source)
{
    return FTMOPLocalization::TableText(Table, Row.ToString(), Field, Source);
}
