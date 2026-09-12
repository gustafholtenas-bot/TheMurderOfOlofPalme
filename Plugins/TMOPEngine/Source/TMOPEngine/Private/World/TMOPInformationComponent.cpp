#include "World/TMOPInformationComponent.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Time/TMOPClockSubsystem.h"
#include "TMOPInformationTimeWindow.h"

namespace
{
bool ValidClockTime(const FTMOPTime& Time)
{
    return Time.Hour >= 0 && Time.Hour < 24 && Time.Minute >= 0 &&
        Time.Minute < 60 && Time.Second >= 0 && Time.Second < 60;
}
}

bool UTMOPInformationComponent::IsAvailableAtTime(FTMOPTime Time) const
{
    if (!bUseTimeWindow) return true;
    return ValidClockTime(Time) && ValidClockTime(VisibleFrom) && ValidClockTime(VisibleUntil) &&
        TMOPInformationTimeWindow::Contains(Time.ToSecondsFromMidnight(),
            VisibleFrom.ToSecondsFromMidnight(), VisibleUntil.ToSecondsFromMidnight());
}

bool UTMOPInformationComponent::HasReadableContent() const
{
    if (!bInteractionEnabled || Title.ToString().TrimStartAndEnd().IsEmpty() ||
        Description.ToString().TrimStartAndEnd().IsEmpty()) return false;
    if (!bUseTimeWindow) return true;
    const UGameInstance* Instance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    const auto* Clock = Instance ? Instance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    // No wall-clock timers: seeking backwards or restarting the loop works immediately.
    return Clock && IsAvailableAtTime(Clock->GetCurrentTime());
}

FText UTMOPInformationComponent::GetInspectionTitle() const { return Title; }
FText UTMOPInformationComponent::GetInspectionText() const { return Description; }
FText UTMOPInformationComponent::GetInspectionSource() const { return SourceReference; }
FText UTMOPInformationComponent::GetInspectionCategory() const
{
    return CategoryLabel.ToString().TrimStartAndEnd().IsEmpty()
        ? Super::GetInspectionCategory() : CategoryLabel;
}
