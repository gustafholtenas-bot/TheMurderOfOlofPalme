#include "UI/TMOPControlsPanel.h"
#include "Localization/TMOPLocalization.h"

#include "Framework/Application/SlateApplication.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

void STMOPControlsPanel::Construct(const FArguments& Arguments)
{
    PlayerCharacter = Arguments._PlayerCharacter;
    if (PlayerCharacter.IsValid() && PlayerCharacter->GetGameInstance())
        Controls = PlayerCharacter->GetGameInstance()->GetSubsystem<UTMOPControlSettingsSubsystem>();
    const int32 OwnerSlot = UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(PlayerCharacter.Get());
    SelectedPlayer = OwnerSlot == INDEX_NONE ? 0 : OwnerSlot;
    ChildSlot[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.035f, 0.035f, 0.045f, 0.96f)).Padding(12.0f)
        [SAssignNew(Body, SVerticalBox)]];
    Rebuild();
}

STMOPControlsPanel::~STMOPControlsPanel()
{
    if (PendingAction.IsSet() && Controls.IsValid()) Controls->EndBindingCapture();
}

void STMOPControlsPanel::Rebuild()
{
    if (!Body.IsValid()) return;
    Body->ClearChildren();
    Body->AddSlot().AutoHeight().Padding(2.0f, 4.0f)
    [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.abd50257833743d3", "Välj spelare och klicka på en bindning. Nästa tangent eller handkontrollsknapp sparas automatiskt. Samma tangent kan aldrig styra två tangentbordsspelare."))) ];
    Body->AddSlot().AutoHeight().Padding(2.0f, 4.0f)
    [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.0b4e603b4fbaf3b1", "Handkontroll: extra sprint och separat tittzoom är från början obundna för att undvika dubbla handlingar. Frigör en knapp med × och bind den här. Vanlig sprint och föremålets sekundärhandling finns kvar.")))];

    TSharedRef<SUniformGridPanel> Players = SNew(SUniformGridPanel).SlotPadding(FMargin(4.0f));
    for (int32 Index = 0; Index < 4; ++Index)
        Players->AddSlot(Index, 0)
        [ SNew(SButton)
          .Text(FTMOPLocalization::Text(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPControlsPanel.90885fa27397775b", "SPELARE {0}{1}"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Index + 1)), FTMOPLocalization::Text(FString(Index == SelectedPlayer ? TEXT(" ✓") : TEXT(""))))))
          .OnClicked(this, &STMOPControlsPanel::SelectPlayer, Index) ];
    Body->AddSlot().AutoHeight().Padding(2.0f, 8.0f)[Players];

    Body->AddSlot().AutoHeight().Padding(2.0f, 4.0f)
    [ SNew(SButton).Text(FTMOPLocalization::Bind([this]() { return DeviceText(); }))
      .OnClicked(this, &STMOPControlsPanel::CycleDevice) ];

    const FTMOPPlayerControlProfile P = Controls.IsValid()
        ? Controls->GetProfile(SelectedPlayer) : FTMOPPlayerControlProfile();
    Body->AddSlot().AutoHeight().Padding(2.0f, 9.0f)
    [ SNew(SVerticalBox)
      + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.84410938be3eae5a", "Kamera X-känslighet")))]
      + SVerticalBox::Slot().AutoHeight()[SNew(SSlider)
        .Value((P.LookSensitivityX - 0.1f) / 2.9f)
        .OnValueChanged_Lambda([this](float V) { UpdateCamera(TOptional<float>(0.1f + V * 2.9f),
            TOptional<float>(), TOptional<bool>(), TOptional<float>()); })]
      + SVerticalBox::Slot().AutoHeight().Padding(0,5)[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.ceb0039426a5b0e5", "Kamera Y-känslighet")))]
      + SVerticalBox::Slot().AutoHeight()[SNew(SSlider)
        .Value((P.LookSensitivityY - 0.1f) / 2.9f)
        .OnValueChanged_Lambda([this](float V) { UpdateCamera(TOptional<float>(),
            TOptional<float>(0.1f + V * 2.9f), TOptional<bool>(), TOptional<float>()); })]
      + SVerticalBox::Slot().AutoHeight().Padding(0,7)
        [SNew(SCheckBox).IsChecked(P.bInvertLookY ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
         .OnCheckStateChanged_Lambda([this](ECheckBoxState State)
            { UpdateCamera(TOptional<float>(), TOptional<float>(),
                TOptional<bool>(State == ECheckBoxState::Checked), TOptional<float>()); })
         [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.4c1bb5393e1e3807", "Invertera kamera Y")))]]
      + SVerticalBox::Slot().AutoHeight().Padding(0,5)[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.2da990157196e04d", "Zoom-FOV (20–80°)")))]
      + SVerticalBox::Slot().AutoHeight()[SNew(SSlider)
        .Value((P.CameraZoomFov - 20.0f) / 60.0f)
        .OnValueChanged_Lambda([this](float V) { UpdateCamera(TOptional<float>(),
            TOptional<float>(), TOptional<bool>(), TOptional<float>(20.0f + V * 60.0f)); })] ];

    AddActionSection(NSLOCTEXT("TMOP", "TMOPControlsPanel.43d65f779d40a600", "TILL FOTS"), {
        ETMOPControlAction::MoveForward, ETMOPControlAction::MoveBackward,
        ETMOPControlAction::MoveLeft, ETMOPControlAction::MoveRight,
        ETMOPControlAction::LookUp, ETMOPControlAction::LookDown,
        ETMOPControlAction::LookLeft, ETMOPControlAction::LookRight,
        ETMOPControlAction::Jump, ETMOPControlAction::Sprint,
        ETMOPControlAction::ExtraSprint, ETMOPControlAction::Interact,
        ETMOPControlAction::PrimaryAction, ETMOPControlAction::SecondaryAction,
        ETMOPControlAction::Crouch, ETMOPControlAction::Kick,
        ETMOPControlAction::ShoulderSwap, ETMOPControlAction::DropItem,
        ETMOPControlAction::LookZoom, ETMOPControlAction::TogglePerspective });
    AddActionSection(NSLOCTEXT("TMOP", "TMOPControlsPanel.73c9d4bdd121171e", "FORDON"), {
        ETMOPControlAction::VehicleAccelerate, ETMOPControlAction::VehicleReverse,
        ETMOPControlAction::VehicleLeft, ETMOPControlAction::VehicleRight,
        ETMOPControlAction::VehicleBrake, ETMOPControlAction::VehicleHandbrake,
        ETMOPControlAction::VehicleExit, ETMOPControlAction::VehicleHighSpeed,
        ETMOPControlAction::VehicleTakeover });
    AddActionSection(NSLOCTEXT("TMOP", "TMOPControlsPanel.2e9cce12bd91e2ff", "MENY / KARTA / INVENTARIE"), {
        ETMOPControlAction::Pause, ETMOPControlAction::WorldMap,
        ETMOPControlAction::QuickInventory, ETMOPControlAction::InventoryPrevious,
        ETMOPControlAction::InventoryNext, ETMOPControlAction::Cancel,
        ETMOPControlAction::MenuUp, ETMOPControlAction::MenuDown,
        ETMOPControlAction::MenuLeft, ETMOPControlAction::MenuRight,
        ETMOPControlAction::MenuConfirm, ETMOPControlAction::MenuZoomIn,
        ETMOPControlAction::MenuZoomOut, ETMOPControlAction::MenuReset,
        ETMOPControlAction::MenuPreviousPage, ETMOPControlAction::MenuNextPage,
        ETMOPControlAction::MenuBack });

    Body->AddSlot().AutoHeight().Padding(2.0f, 10.0f)
    [ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.ad2d4ef1e5035520", "ÅTERSTÄLL SPELARENS STANDARDKONTROLLER")))
      .OnClicked(this, &STMOPControlsPanel::ResetSelectedPlayer) ];
    Body->AddSlot().AutoHeight().Padding(2.0f, 8.0f)
    [ SAssignNew(Status, STextBlock).AutoWrapText(true) ];
}

void STMOPControlsPanel::AddActionSection(const FText& Heading,
    std::initializer_list<ETMOPControlAction> Actions)
{
    Body->AddSlot().AutoHeight().Padding(2.0f, 14.0f, 2.0f, 5.0f)
    [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).Text(FTMOPLocalization::Text(Heading)) ];
    for (const ETMOPControlAction Action : Actions)
        Body->AddSlot().AutoHeight().Padding(2.0f)
        [ SNew(SHorizontalBox)
          + SHorizontalBox::Slot().FillWidth(0.36f).VAlign(VAlign_Center)
            [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true).Text(FTMOPLocalization::Text(UTMOPControlSettingsSubsystem::GetActionDisplayName(Action)))]
          + SHorizontalBox::Slot().FillWidth(0.27f).Padding(3.0f)
            [SNew(SBox).MinDesiredHeight(32.0f)[SNew(SButton)
              .Text(FTMOPLocalization::Bind([this, Action]() { return BindingText(Action, false); }))
              .OnClicked(this, &STMOPControlsPanel::BeginBinding, Action, false)]]
          + SHorizontalBox::Slot().FillWidth(0.27f).Padding(3.0f)
            [SNew(SBox).MinDesiredHeight(32.0f)[SNew(SButton)
              .Text(FTMOPLocalization::Bind([this, Action]() { return BindingText(Action, true); }))
              .OnClicked(this, &STMOPControlsPanel::BeginBinding, Action, true)]]
          + SHorizontalBox::Slot().AutoWidth()
            [SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("× 1"))))
              .OnClicked_Lambda([this, Action]() { FText Error;
                  if (Controls.IsValid()) Controls->Rebind(SelectedPlayer, Action, false, FKey(), Error);
                  return FReply::Handled(); })]
          + SHorizontalBox::Slot().AutoWidth()
            [SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("× 2"))))
              .OnClicked_Lambda([this, Action]() { FText Error;
                  if (Controls.IsValid()) Controls->Rebind(SelectedPlayer, Action, true, FKey(), Error);
                  return FReply::Handled(); })] ];
}

FReply STMOPControlsPanel::SelectPlayer(const int32 PlayerIndex)
{
    if (PendingAction.IsSet() && Controls.IsValid()) Controls->EndBindingCapture();
    SelectedPlayer = FMath::Clamp(PlayerIndex, 0, 3);
    PendingAction.Reset();
    Rebuild();
    return FReply::Handled();
}

FReply STMOPControlsPanel::CycleDevice()
{
    if (!Controls.IsValid()) return FReply::Handled();
    if (PendingAction.IsSet()) Controls->EndBindingCapture();
    PendingAction.Reset();
    const ETMOPControlDevice Current = Controls->GetProfile(SelectedPlayer).Device;
    ETMOPControlDevice Next = ETMOPControlDevice::Gamepad;
    if (SelectedPlayer == 0)
        Next = Current == ETMOPControlDevice::KeyboardMouse
            ? ETMOPControlDevice::Gamepad : ETMOPControlDevice::KeyboardMouse;
    else if (SelectedPlayer == 1)
        Next = Current == ETMOPControlDevice::KeyboardOnly
            ? ETMOPControlDevice::Gamepad : ETMOPControlDevice::KeyboardOnly;
    FText Error;
    const bool bChanged = Controls->SetDevice(SelectedPlayer, Next, Error);
    Rebuild();
    if (!bChanged && Status.IsValid()) Status->SetText(FTMOPLocalization::Text(Error));
    return FReply::Handled();
}

FReply STMOPControlsPanel::BeginBinding(const ETMOPControlAction Action,
    const bool bSecondary)
{
    if (Controls.IsValid() && Controls->IsBindingCaptureActive())
    {
        if (Status.IsValid()) Status->SetText(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.49b58a85f472be81", "En annan ombindning pågår. Slutför den först.")));
        return FReply::Handled();
    }
    PendingAction = Action;
    bPendingSecondary = bSecondary;
    if (Controls.IsValid())
    {
        const int32 OwnerSlot = UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(PlayerCharacter.Get());
        Controls->BeginBindingCapture(OwnerSlot == INDEX_NONE
            ? 0u : Controls->GetSlateUserForPlayer(OwnerSlot));
    }
    if (Status.IsValid()) Status->SetText(FTMOPLocalization::Text(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPControlsPanel.1e018b9549de7164", "Tryck ny knapp för {0}. Escape avbryter."),
        UTMOPControlSettingsSubsystem::GetActionDisplayName(Action))));
    if (Controls.IsValid() && Controls->GetBindingCaptureSlateUser() != MAX_uint32)
        FSlateApplication::Get().SetUserFocus(Controls->GetBindingCaptureSlateUser(),
            SharedThis(this), EFocusCause::SetDirectly);
    return FReply::Handled();
}

void STMOPControlsPanel::CommitBinding(const FKey Key)
{
    if (!PendingAction.IsSet() || !Controls.IsValid()) return;
    FText Error;
    const bool bOk = Controls->Rebind(SelectedPlayer, PendingAction.GetValue(),
        bPendingSecondary, Key, Error);
    if (Status.IsValid()) Status->SetText(FTMOPLocalization::Text(bOk
        ? FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPControlsPanel.f2089c0fdb60d470", "Sparad: {0}"), Key.GetDisplayName(false))
        : Error));
    if (bOk)
    {
        PendingAction.Reset();
        Controls->EndBindingCapture();
    }
}

FReply STMOPControlsPanel::OnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (PendingAction.IsSet())
    {
        if (Event.GetKey() == EKeys::Escape)
        {
            PendingAction.Reset();
            if (Controls.IsValid()) Controls->EndBindingCapture();
            if (Status.IsValid()) Status->SetText(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPControlsPanel.2018c5bf88ad3497", "Ombindning avbruten.")));
        }
        else CommitBinding(Event.GetKey());
        return FReply::Handled();
    }
    return SCompoundWidget::OnKeyDown(Geometry, Event);
}

FReply STMOPControlsPanel::OnMouseButtonDown(const FGeometry& Geometry,
    const FPointerEvent& Event)
{
    if (PendingAction.IsSet())
    {
        CommitBinding(Event.GetEffectingButton());
        return FReply::Handled();
    }
    return SCompoundWidget::OnMouseButtonDown(Geometry, Event);
}

FReply STMOPControlsPanel::ResetSelectedPlayer()
{
    if (PendingAction.IsSet() && Controls.IsValid()) Controls->EndBindingCapture();
    FText Error;
    const bool bReset = Controls.IsValid() && Controls->TryResetPlayerToDefaults(SelectedPlayer, Error);
    PendingAction.Reset();
    Rebuild();
    if (!bReset && Status.IsValid()) Status->SetText(FTMOPLocalization::Text(Error));
    return FReply::Handled();
}

void STMOPControlsPanel::UpdateCamera(const TOptional<float> SensitivityX,
    const TOptional<float> SensitivityY, const TOptional<bool> bInvertY,
    const TOptional<float> ZoomFov)
{
    if (!Controls.IsValid()) return;
    const FTMOPPlayerControlProfile P = Controls->GetProfile(SelectedPlayer);
    Controls->SetCameraSettings(SelectedPlayer,
        SensitivityX.IsSet() ? SensitivityX.GetValue() : P.LookSensitivityX,
        SensitivityY.IsSet() ? SensitivityY.GetValue() : P.LookSensitivityY,
        bInvertY.IsSet() ? bInvertY.GetValue() : P.bInvertLookY,
        ZoomFov.IsSet() ? ZoomFov.GetValue() : P.CameraZoomFov);
}

FText STMOPControlsPanel::DeviceText() const
{
    if (!Controls.IsValid()) return NSLOCTEXT("TMOP", "TMOPControlsPanel.84e3c24c8e3a51f0", "Styrning saknas");
    switch (Controls->GetProfile(SelectedPlayer).Device)
    {
    case ETMOPControlDevice::KeyboardMouse: return NSLOCTEXT("TMOP", "TMOPControlsPanel.7973e3fcabfa75f0", "ENHET: TANGENTBORD + MUS");
    case ETMOPControlDevice::KeyboardOnly: return NSLOCTEXT("TMOP", "TMOPControlsPanel.f4ed477696116d69", "ENHET: DELAT TANGENTBORD");
    default: return NSLOCTEXT("TMOP", "TMOPControlsPanel.88481e4ff052c434", "ENHET: HANDKONTROLL");
    }
}

FText STMOPControlsPanel::BindingText(const ETMOPControlAction Action,
    const bool bSecondary) const
{
    if (!Controls.IsValid()) return FText::GetEmpty();
    const FKey Key = Controls->GetKey(SelectedPlayer, Action, bSecondary);
    return Key.IsValid() ? Key.GetDisplayName(false)
        : FText::FromString(bSecondary ? NSLOCTEXT("TMOP", "TMOPControlsPanel.44e491e95e8a500e", "+ Lägg till").ToString() : NSLOCTEXT("TMOP", "TMOPControlsPanel.64007e128bb403d4", "Ej bunden").ToString());
}
