#pragma once

#include "Engine/GameInstance.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Localization/TMOPLanguageSubsystem.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"

/** Shared selector for the start menu and the in-game settings page. */
inline TSharedRef<SWidget> MakeTMOPLanguageSelector(UGameInstance* GameInstance)
{
    const TWeakObjectPtr<UTMOPLanguageSubsystem> Language = GameInstance
        ? GameInstance->GetSubsystem<UTMOPLanguageSubsystem>() : nullptr;
    return SNew(SComboButton)
        .IsEnabled(Language.IsValid())
        .OnGetMenuContent_Lambda([Language]() -> TSharedRef<SWidget>
        {
            FMenuBuilder Menu(true, nullptr);
            auto Add = [&Menu, Language](const TCHAR* Code, const TCHAR* Label)
            {
                const FString Culture(Code);
                Menu.AddMenuEntry(FText::AsCultureInvariant(Label), FText::GetEmpty(), FSlateIcon(),
                    FUIAction(
                        FExecuteAction::CreateLambda([Language, Culture]
                        { if (Language.IsValid()) Language->SetLanguage(Culture); }),
                        FCanExecuteAction(),
                        FIsActionChecked::CreateLambda([Language, Culture]
                        { return Language.IsValid() && Language->GetLanguage() == Culture; })),
                    NAME_None, EUserInterfaceActionType::RadioButton);
            };
            Add(TEXT("sv"), TEXT("Svenska"));
            Add(TEXT("en"), TEXT("English"));
            return Menu.MakeWidget();
        })
        .ButtonContent()
        [ SNew(STextBlock).Text_Lambda([Language]
          {
              // Keep Language readable even when a player cannot read Swedish.
              return FText::AsCultureInvariant(Language.IsValid() && Language->GetLanguage() == TEXT("en")
                  ? TEXT("Language: English") : TEXT("Language: Svenska"));
          }) ];
}
