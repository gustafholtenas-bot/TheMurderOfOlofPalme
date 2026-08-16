#include "TMOPPersonTimelineEntryCustomization.h"

#include "DetailWidgetRow.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "People/TMOPPersonProfileTypes.h"
#include "PropertyHandle.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "TMOPPersonTimelineEntryCustomization"

TSharedRef<IPropertyTypeCustomization>
FTMOPPersonTimelineEntryCustomization::MakeInstance()
{
    return MakeShared<FTMOPPersonTimelineEntryCustomization>();
}

void FTMOPPersonTimelineEntryCustomization::CustomizeHeader(
    const TSharedRef<IPropertyHandle> StructPropertyHandle,
    FDetailWidgetRow& HeaderRow,
    IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    StructHandle = StructPropertyHandle;
    const TSharedPtr<IPropertyHandle> EntryIdHandle =
        StructPropertyHandle->GetChildHandle(
            GET_MEMBER_NAME_CHECKED(FTMOPPersonTimelineEntry, EntryId));

    HeaderRow
        .NameContent()
        [
            EntryIdHandle.IsValid()
                ? EntryIdHandle->CreatePropertyNameWidget()
                : StructPropertyHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        .MinDesiredWidth(250.0f)
        [
            SNew(SBorder)
            .Padding(FMargin(2.0f))
            .BorderImage(FAppStyle::GetBrush("NoBorder"))
            .OnMouseButtonDown(
                this,
                &FTMOPPersonTimelineEntryCustomization::HandleHeaderMouseButtonDown)
            [
                EntryIdHandle.IsValid()
                    ? EntryIdHandle->CreatePropertyValueWidget()
                    : StructPropertyHandle->CreatePropertyValueWidget()
            ]
        ];
}

void FTMOPPersonTimelineEntryCustomization::CustomizeChildren(
    const TSharedRef<IPropertyHandle> StructPropertyHandle,
    IDetailChildrenBuilder& StructBuilder,
    IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    ActionHandle = StructPropertyHandle->GetChildHandle(
        GET_MEMBER_NAME_CHECKED(FTMOPPersonTimelineEntry, Action));
    LocationTypeHandle = StructPropertyHandle->GetChildHandle(
        GET_MEMBER_NAME_CHECKED(FTMOPPersonTimelineEntry, LocationType));
    ActivityStateHandle = StructPropertyHandle->GetChildHandle(
        GET_MEMBER_NAME_CHECKED(FTMOPPersonTimelineEntry, ActivityState));

    if (ActionHandle.IsValid())
        ActionHandle->SetOnPropertyValueChanged(
            FSimpleDelegate::CreateSP(
                this,
                &FTMOPPersonTimelineEntryCustomization::HandleActionChanged));

    uint32 ChildCount = 0;
    StructPropertyHandle->GetNumChildren(ChildCount);
    for (uint32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
        if (const TSharedPtr<IPropertyHandle> ChildHandle =
            StructPropertyHandle->GetChildHandle(ChildIndex))
            StructBuilder.AddProperty(ChildHandle.ToSharedRef());
}

void FTMOPPersonTimelineEntryCustomization::HandleActionChanged()
{
    if (!ActionHandle.IsValid()) return;
    uint8 ActionValue = 0;
    if (ActionHandle->GetValue(ActionValue) != FPropertyAccess::Success ||
        static_cast<ETMOPPersonTimelineAction>(ActionValue) !=
            ETMOPPersonTimelineAction::MoveToAnchor)
        return;

    if (LocationTypeHandle.IsValid())
        LocationTypeHandle->SetValue(static_cast<uint8>(
            ETMOPPersonLocationType::Anchor));
    if (ActivityStateHandle.IsValid())
        ActivityStateHandle->SetValue(static_cast<uint8>(
            ETMOPAgentActivityState::Walking));
}

FReply FTMOPPersonTimelineEntryCustomization::HandleHeaderMouseButtonDown(
    const FGeometry& Geometry,
    const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::RightMouseButton)
        return FReply::Unhandled();

    FMenuBuilder MenuBuilder(true, nullptr);
    MenuBuilder.AddMenuEntry(
        LOCTEXT("DeleteTimelineEntry", "Delete Timeline Entry"),
        LOCTEXT("DeleteTimelineEntryTooltip",
            "Deletes only this timeline entry."),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateSP(
                this,
                &FTMOPPersonTimelineEntryCustomization::DeleteTimelineEntry),
            FCanExecuteAction::CreateSP(
                this,
                &FTMOPPersonTimelineEntryCustomization::CanDeleteTimelineEntry)));

    const TSharedPtr<SWindow> ParentWindow =
        FSlateApplication::Get().GetActiveTopLevelWindow();
    if (!ParentWindow.IsValid()) return FReply::Unhandled();

    FSlateApplication::Get().PushMenu(
        ParentWindow.ToSharedRef(),
        FWidgetPath(),
        MenuBuilder.MakeWidget(),
        MouseEvent.GetScreenSpacePosition(),
        FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
    return FReply::Handled();
}

void FTMOPPersonTimelineEntryCustomization::DeleteTimelineEntry()
{
    if (!StructHandle.IsValid()) return;
    const TSharedPtr<IPropertyHandle> ParentHandle =
        StructHandle->GetParentHandle();
    const TSharedPtr<IPropertyHandleArray> ArrayHandle =
        ParentHandle.IsValid() ? ParentHandle->AsArray() : nullptr;
    const int32 ArrayIndex = StructHandle->GetIndexInArray();
    if (ArrayHandle.IsValid() && ArrayIndex != INDEX_NONE)
        ArrayHandle->DeleteItem(ArrayIndex);
}

bool FTMOPPersonTimelineEntryCustomization::CanDeleteTimelineEntry() const
{
    return StructHandle.IsValid() &&
        StructHandle->GetIndexInArray() != INDEX_NONE;
}

#undef LOCTEXT_NAMESPACE
