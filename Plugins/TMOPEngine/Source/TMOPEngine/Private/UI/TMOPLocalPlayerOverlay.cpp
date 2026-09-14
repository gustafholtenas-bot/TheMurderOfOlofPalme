#include "UI/TMOPLocalPlayerOverlay.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/TextRenderComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Time/TMOPClockSubsystem.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "World/TMOPFindingActor.h"
#include "World/TMOPInspectableComponent.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
UTextRenderComponent* FindLabel(AActor* Actor)
{
    if (auto* Agent = Cast<ATMOPHistoricalAgent>(Actor)) return Agent->bShowNameLabel ? Agent->NameLabel.Get() : nullptr;
    if (auto* Vehicle = Cast<ATMOPVehicleBase>(Actor)) return Vehicle->bShowNameLabel ? Vehicle->NameLabel.Get() : nullptr;
    if (auto* Finding = Cast<ATMOPFindingActor>(Actor)) return Finding->FindingLabel;
    return nullptr;
}
}

TSharedRef<SWidget> UTMOPLocalPlayerOverlay::RebuildWidget()
{
    return SNew(SOverlay).Visibility_Lambda([this]()
        {
            const auto* Player = Cast<ATMOPPlayerCharacter>(GetOwningPlayerPawn());
            return Player && UTMOPLocalMultiplayerSubsystem::IsMultiplayer(this) &&
                Player->IsGameplayHUDVisible()
                ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
        })
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(12)
        [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0,0,0,0.65f)).Padding(8)
          [ SNew(STextBlock).Text_UObject(this, &UTMOPLocalPlayerOverlay::GetStatus)
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
            .ColorAndOpacity(FLinearColor::White) ] ];
}

FText UTMOPLocalPlayerOverlay::GetStatus() const
{
    const auto* Player = Cast<ATMOPPlayerCharacter>(GetOwningPlayerPawn());
    const auto* Clock = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!Player || !Clock) return FText::GetEmpty();
    return FText::FromString(FString::Printf(TEXT("SPELARE %d   %s%s"),
        UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(Player) + 1,
        *Clock->GetCurrentTime().ToDisplayString(),
        UGameplayStatics::IsGamePaused(this) ? TEXT("\nPAUSAT FÖR ALLA") : TEXT("")));
}

void UTMOPLocalPlayerOverlay::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    const auto* Player = Cast<ATMOPPlayerCharacter>(GetOwningPlayerPawn());
    if (!Player || !UTMOPLocalMultiplayerSubsystem::IsMultiplayer(this))
    {
        NearbyLabels.Reset();
        NearbyInspectables.Reset();
        return;
    }
    RefreshElapsed += DeltaTime;
    if (RefreshElapsed < 0.25f) return;
    RefreshElapsed = 0;
    NearbyLabels.Reset();
    NearbyInspectables.Reset();
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        if (!It->IsHidden() && FindLabel(*It) &&
            FVector::DistSquared(It->GetActorLocation(), Player->GetActorLocation()) < FMath::Square(2000.0f))
            NearbyLabels.Add(*It);
    NearbyLabels.Sort([Player](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B)
    { return FVector::DistSquared(A->GetActorLocation(), Player->GetActorLocation()) <
        FVector::DistSquared(B->GetActorLocation(), Player->GetActorLocation()); });
    if (NearbyLabels.Num() > 24) NearbyLabels.SetNum(24);

    TArray<UTMOPInspectableComponent*> Inspectables;
    UTMOPInspectableComponent::GetActiveInWorld(GetWorld(), Inspectables);
    const FVector ViewLocation = Player->GetPawnViewLocation();
    for (UTMOPInspectableComponent* Inspection : Inspectables)
        if (IsValid(Inspection) && Inspection->ShouldShowWorldIndicatorAt(ViewLocation) &&
            Inspection->IsVisibleFrom(ViewLocation, Player))
            NearbyInspectables.Add(Inspection);
    NearbyInspectables.Sort([ViewLocation](
        const TWeakObjectPtr<UTMOPInspectableComponent>& A,
        const TWeakObjectPtr<UTMOPInspectableComponent>& B)
    {
        return FVector::DistSquared(A->GetWorldIndicatorLocation(), ViewLocation) <
            FVector::DistSquared(B->GetWorldIndicatorLocation(), ViewLocation);
    });
    if (NearbyInspectables.Num() > 48) NearbyInspectables.SetNum(48);
}

int32 UTMOPLocalPlayerOverlay::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry,
    const FSlateRect& CullingRect, FSlateWindowElementList& Elements, int32 LayerId,
    const FWidgetStyle& Style, bool bParentEnabled) const
{
    const auto* Player = Cast<ATMOPPlayerCharacter>(GetOwningPlayerPawn());
    if (!Player || !UTMOPLocalMultiplayerSubsystem::IsMultiplayer(this) ||
        !Player->IsGameplayHUDVisible()) return LayerId;
    const int32 Result = Super::NativePaint(Args, Geometry, CullingRect, Elements, LayerId, Style, bParentEnabled);
    if (!Player->IsGameplayHUDVisible()) return Result;
    const float Scale = FMath::Max(0.01f, UWidgetLayoutLibrary::GetViewportScale(this));
    for (const auto& WeakActor : NearbyLabels)
    {
        AActor* Actor = WeakActor.Get();
        UTextRenderComponent* Label = FindLabel(Actor);
        if (!Actor || Actor->IsHidden() || !Label || !Label->IsVisible()) continue;
        FVector2D Position;
        if (!GetOwningPlayer()->ProjectWorldLocationToScreen(Label->GetComponentLocation(), Position, true)) continue;
        Position /= Scale;
        if (Position.X < 0 || Position.Y < 60 || Position.X > Geometry.GetLocalSize().X - 180 ||
            Position.Y > Geometry.GetLocalSize().Y - 60) continue;
        FSlateDrawElement::MakeText(Elements, Result + 1,
            Geometry.ToPaintGeometry(FVector2D(180,60), FSlateLayoutTransform(Position)),
            Label->Text, FCoreStyle::GetDefaultFontStyle("Regular", 10),
            ESlateDrawEffect::None, FLinearColor(Label->TextRenderColor));
    }
    for (const auto& WeakInspection : NearbyInspectables)
    {
        const UTMOPInspectableComponent* Inspection = WeakInspection.Get();
        if (!IsValid(Inspection) || !Inspection->ShouldShowWorldIndicatorAt(
            Player->GetPawnViewLocation())) continue;
        FVector2D Position;
        if (!GetOwningPlayer()->ProjectWorldLocationToScreen(
            Inspection->GetWorldIndicatorLocation(), Position, true)) continue;
        Position /= Scale;
        if (Position.X < 12 || Position.Y < 60 ||
            Position.X > Geometry.GetLocalSize().X - 36 ||
            Position.Y > Geometry.GetLocalSize().Y - 36) continue;
        FSlateDrawElement::MakeText(Elements, Result + 2,
            Geometry.ToPaintGeometry(FVector2D(36, 36),
                FSlateLayoutTransform(Position - FVector2D(18, 18))),
            Inspection->WorldIndicatorText,
            FCoreStyle::GetDefaultFontStyle("Bold", 22),
            ESlateDrawEffect::None,
            FLinearColor(Inspection->WorldIndicatorColor));
    }
    return Result + 2;
}
