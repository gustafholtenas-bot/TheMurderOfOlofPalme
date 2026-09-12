#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Player/TMOPControlSettingsSubsystem.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"

inline const UTMOPControlSettingsSubsystem* TMOPGetControls(const UUserWidget* Widget,
    int32& OutPlayerIndex)
{
    OutPlayerIndex = INDEX_NONE;
    const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
    const ATMOPPlayerCharacter* Player = PC ? Cast<ATMOPPlayerCharacter>(PC->GetPawn()) : nullptr;
    if (!Player || !Player->bUseControlProfiles) return nullptr;
    OutPlayerIndex = UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(Player);
    const UGameInstance* GI = Widget ? Widget->GetGameInstance() : nullptr;
    return GI ? GI->GetSubsystem<UTMOPControlSettingsSubsystem>() : nullptr;
}

inline bool TMOPMatchesControl(const UUserWidget* Widget, const FKey Key,
    const ETMOPControlAction Action)
{
    int32 PlayerIndex = INDEX_NONE;
    const UTMOPControlSettingsSubsystem* Controls = TMOPGetControls(Widget, PlayerIndex);
    return Controls && PlayerIndex != INDEX_NONE &&
        (Controls->GetKey(PlayerIndex, Action, false) == Key ||
         Controls->GetKey(PlayerIndex, Action, true) == Key);
}

inline bool TMOPHasControlProfiles(const UUserWidget* Widget)
{
    int32 PlayerIndex = INDEX_NONE;
    return TMOPGetControls(Widget, PlayerIndex) != nullptr && PlayerIndex != INDEX_NONE;
}

inline FText TMOPControlDisplayText(const UUserWidget* Widget,
    const ETMOPControlAction Action, const FText& Fallback)
{
    int32 PlayerIndex = INDEX_NONE;
    const UTMOPControlSettingsSubsystem* Controls = TMOPGetControls(Widget, PlayerIndex);
    return Controls && PlayerIndex != INDEX_NONE
        ? Controls->GetKeyDisplayText(PlayerIndex, Action) : Fallback;
}
