#pragma once

#include "IPropertyTypeCustomization.h"

class IPropertyHandle;

class FTMOPPersonTimelineEntryCustomization final
    : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    virtual void CustomizeHeader(
        TSharedRef<IPropertyHandle> StructPropertyHandle,
        FDetailWidgetRow& HeaderRow,
        IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    virtual void CustomizeChildren(
        TSharedRef<IPropertyHandle> StructPropertyHandle,
        IDetailChildrenBuilder& StructBuilder,
        IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
    void HandleActionChanged();
    FReply HandleHeaderMouseButtonDown(
        const FGeometry& Geometry,
        const FPointerEvent& MouseEvent);
    void DeleteTimelineEntry();
    bool CanDeleteTimelineEntry() const;

    TSharedPtr<IPropertyHandle> StructHandle;
    TSharedPtr<IPropertyHandle> ActionHandle;
    TSharedPtr<IPropertyHandle> LocationTypeHandle;
    TSharedPtr<IPropertyHandle> ActivityStateHandle;
    TSharedPtr<IPropertyHandle> TimeIsArrivalHandle;
};
