#include "Localization/TMOPLanguageSubsystem.h"
#include "Localization/TMOPLocalization.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/PolyglotTextData.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Misc/ConfigCacheIni.h"

void UTMOPLanguageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterMenuTranslations();
    FTMOPLocalization::LoadLanguagePacks();
    FString Language = TEXT("sv");
    GConfig->GetString(TEXT("TMOP.Language"), TEXT("Language"), Language, GGameUserSettingsIni);
    // An absent/obsolete preference always starts in Swedish, regardless of OS language.
    SetLanguage(IsSupportedLanguage(Language) ? Language : TEXT("sv"), false);
}

bool UTMOPLanguageSubsystem::IsSupportedLanguage(const FString& Language)
{
    return Language == TEXT("sv") || Language == TEXT("en");
}

FString UTMOPLanguageSubsystem::GetLanguage() const
{
    return FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName();
}

bool UTMOPLanguageSubsystem::SetLanguage(const FString& Language, const bool bSave)
{
    if (!IsSupportedLanguage(Language)) return false;
    // Only change the display language: historical times and number formatting keep their locale.
    if (!FInternationalization::Get().SetCurrentLanguage(Language)) return false;
    if (bSave)
    {
        GConfig->SetString(TEXT("TMOP.Language"), TEXT("Language"), *Language, GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
    }
    return true;
}

void UTMOPLanguageSubsystem::RegisterMenuTranslations()
{
    // Built into the runtime module, so packaged builds need no editor gather/compile step.
    // Stable NSLOCTEXT identities also allow a future full localization target.
    static bool bRegistered = false;
    if (bRegistered) return;
    bRegistered = true;
    auto Add = [](const TCHAR* Key, const TCHAR* Swedish, const TCHAR* English)
    {
        FPolyglotTextData Text(ELocalizedTextSourceCategory::Game,
            TEXT("TMOP"), Key, Swedish, TEXT("sv"));
        Text.AddLocalizedString(TEXT("en"), English);
        FTextLocalizationManager::Get().RegisterPolyglotTextData(Text, true);
    };
#include "TMOPMenuTranslations.inl"
}
