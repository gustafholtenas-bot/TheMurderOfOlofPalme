#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Player/TMOPControlSettingsSubsystem.h"
#include "Widgets/SCompoundWidget.h"
#include <initializer_list>

class ATMOPPlayerCharacter;
class STextBlock;
class SVerticalBox;

/** Reusable Slate controls editor used by the pause/settings hub. */
class STMOPControlsPanel final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPControlsPanel) {}
        SLATE_ARGUMENT(ATMOPPlayerCharacter*, PlayerCharacter)
    SLATE_END_ARGS()

    void Construct(const FArguments& Arguments);
    virtual ~STMOPControlsPanel() override;
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    virtual FReply OnMouseButtonDown(const FGeometry& Geometry,
        const FPointerEvent& Event) override;

private:
    TWeakObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;
    TWeakObjectPtr<UTMOPControlSettingsSubsystem> Controls;
    TSharedPtr<SVerticalBox> Body;
    TSharedPtr<STextBlock> Status;
    int32 SelectedPlayer = 0;
    TOptional<ETMOPControlAction> PendingAction;
    bool bPendingSecondary = false;

    void Rebuild();
    void AddActionSection(const FText& Heading,
        std::initializer_list<ETMOPControlAction> Actions);
    FReply SelectPlayer(int32 PlayerIndex);
    FReply CycleDevice();
    FReply BeginBinding(ETMOPControlAction Action, bool bSecondary);
    FReply ResetSelectedPlayer();
    void CommitBinding(FKey Key);
    void UpdateCamera(TOptional<float> SensitivityX,
        TOptional<float> SensitivityY, TOptional<bool> bInvertY,
        TOptional<float> ZoomFov);
    FText DeviceText() const;
    FText BindingText(ETMOPControlAction Action, bool bSecondary) const;
};
