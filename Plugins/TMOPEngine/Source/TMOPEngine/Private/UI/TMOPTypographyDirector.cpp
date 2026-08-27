#include "UI/TMOPTypographyDirector.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/TextRenderComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Engine/DataTable.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "Widgets/Text/STextBlock.h"

ATMOPTypographyDirector::ATMOPTypographyDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void ATMOPTypographyDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bAutomaticallyStyleBlueprintWidgets) return;
    RefreshAccumulator += DeltaSeconds;
    if (RefreshAccumulator >= BlueprintRefreshIntervalSeconds)
    {
        RefreshAccumulator = 0.0f;
        RefreshAllBlueprintText();
    }
}

bool ATMOPTypographyDirector::GetTypographyStyle(const FName StyleId,
    FTMOPTypographyStyleRow& OutStyle) const
{
    if (!IsValid(TypographyTable) || StyleId.IsNone()) return false;
    if (const FTMOPTypographyStyleRow* Direct =
        TypographyTable->FindRow<FTMOPTypographyStyleRow>(StyleId,
            TEXT("TMOP Typography"), false))
    {
        OutStyle = *Direct;
        return true;
    }
    for (const TPair<FName, uint8*>& Pair : TypographyTable->GetRowMap())
    {
        const FTMOPTypographyStyleRow* Row =
            reinterpret_cast<const FTMOPTypographyStyleRow*>(Pair.Value);
        if (Row != nullptr && Row->StyleId == StyleId)
        {
            OutStyle = *Row;
            return true;
        }
    }
    return false;
}

bool ATMOPTypographyDirector::ApplyTypographyStyle(UTextBlock* TextBlock,
    const FName StyleId) const
{
    if (!IsValid(TextBlock)) return false;
    FTMOPTypographyStyleRow Style;
    if (!GetTypographyStyle(StyleId, Style)) return false;
    FSlateFontInfo Font = TextBlock->GetFont();
    Font.Size = Style.Size;
    Font.TypefaceFontName = Style.Typeface;
    if (UObject* FontObject = Style.FontAsset.LoadSynchronous())
        Font.FontObject = FontObject;
    Font.OutlineSettings.OutlineSize = Style.OutlineSize;
    Font.OutlineSettings.OutlineColor = Style.OutlineColor;
    TextBlock->SetFont(Font);
    TextBlock->SetColorAndOpacity(Style.Color);
    TextBlock->SetShadowOffset(Style.ShadowOffset);
    TextBlock->SetShadowColorAndOpacity(Style.ShadowColor);
    return true;
}

bool ATMOPTypographyDirector::ApplyWorldTextStyle(
    UTextRenderComponent* TextRender, const FName StyleId) const
{
    if (!IsValid(TextRender)) return false;
    FTMOPTypographyStyleRow Style;
    if (!GetTypographyStyle(StyleId, Style)) return false;
    if (UFont* Font = Cast<UFont>(Style.FontAsset.LoadSynchronous()))
        TextRender->SetFont(Font);
    TextRender->SetWorldSize(static_cast<float>(Style.Size));
    TextRender->SetTextRenderColor(Style.Color.ToFColor(true));
    return true;
}

FName ATMOPTypographyDirector::InferStyleId(const UTextBlock* TextBlock) const
{
    if (!IsValid(TextBlock)) return TEXT("Body");
    const FName WidgetName = TextBlock->GetFName();
    if (const FName* Override = WidgetNameStyleOverrides.Find(WidgetName))
        return *Override;
    const FString Name = WidgetName.ToString().ToLower();
    if (Name.Contains(TEXT("agentname")) || Name.Contains(TEXT("nameplate")))
        return TEXT("AgentName");
    if (Name.Contains(TEXT("title")) || Name.Contains(TEXT("heading")) ||
        Name.Contains(TEXT("header"))) return TEXT("Heading");
    if (Name.Contains(TEXT("button")) || Name.Contains(TEXT("menu")))
        return TEXT("MenuButton");
    if (Name.Contains(TEXT("subtitle")) || Name.Contains(TEXT("transcript")))
        return TEXT("Subtitle");
    if (Name.Contains(TEXT("speaker"))) return TEXT("SpeakerName");
    if (Name.Contains(TEXT("target"))) return TEXT("TargetLabel");
    if (Name.Contains(TEXT("source")) || Name.Contains(TEXT("status")) ||
        Name.Contains(TEXT("details")) || Name.Contains(TEXT("help")))
        return TEXT("Caption");
    return TEXT("Body");
}

void ATMOPTypographyDirector::RefreshAllBlueprintText()
{
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets,
        UUserWidget::StaticClass(), false);
    for (UUserWidget* Widget : Widgets)
    {
        if (!IsValid(Widget) || !IsValid(Widget->WidgetTree)) continue;
        Widget->WidgetTree->ForEachWidget([this](UWidget* Child)
        {
            if (UTextBlock* Text = Cast<UTextBlock>(Child))
                ApplyTypographyStyle(Text, InferStyleId(Text));
        });
    }
    for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It)
        if (IsValid(It->NameLabel))
            ApplyWorldTextStyle(It->NameLabel, TEXT("AgentName"));
}

const ATMOPTypographyDirector* ATMOPTypographyDirector::Find(
    const UObject* WorldContext)
{
    if (!IsValid(WorldContext) || WorldContext->GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPTypographyDirector> It(WorldContext->GetWorld()); It; ++It)
        return *It;
    return nullptr;
}

FSlateFontInfo ATMOPTypographyDirector::ResolveFont(const UObject* WorldContext,
    const FName StyleId, const FSlateFontInfo& Fallback)
{
    FTMOPTypographyStyleRow Style;
    const ATMOPTypographyDirector* Director = Find(WorldContext);
    if (!IsValid(Director) || !Director->GetTypographyStyle(StyleId, Style))
        return Fallback;
    FSlateFontInfo Result = Fallback;
    Result.Size = Style.Size;
    Result.TypefaceFontName = Style.Typeface;
    if (UObject* FontObject = Style.FontAsset.LoadSynchronous())
        Result.FontObject = FontObject;
    Result.OutlineSettings.OutlineSize = Style.OutlineSize;
    Result.OutlineSettings.OutlineColor = Style.OutlineColor;
    return Result;
}

FSlateColor ATMOPTypographyDirector::ResolveColor(const UObject* WorldContext,
    const FName StyleId, const FLinearColor& Fallback)
{
    FTMOPTypographyStyleRow Style;
    const ATMOPTypographyDirector* Director = Find(WorldContext);
    return IsValid(Director) && Director->GetTypographyStyle(StyleId, Style)
        ? FSlateColor(Style.Color) : FSlateColor(Fallback);
}

void ATMOPTypographyDirector::ApplySlateStyle(const UObject* WorldContext,
    const TSharedPtr<STextBlock>& TextBlock, const FName StyleId)
{
    if (!TextBlock.IsValid()) return;
    FTMOPTypographyStyleRow Style;
    const ATMOPTypographyDirector* Director = Find(WorldContext);
    if (!IsValid(Director) || !Director->GetTypographyStyle(StyleId, Style)) return;
    FSlateFontInfo Font = TextBlock->GetFont();
    Font.Size = Style.Size;
    Font.TypefaceFontName = Style.Typeface;
    if (UObject* FontObject = Style.FontAsset.LoadSynchronous())
        Font.FontObject = FontObject;
    Font.OutlineSettings.OutlineSize = Style.OutlineSize;
    Font.OutlineSettings.OutlineColor = Style.OutlineColor;
    TextBlock->SetFont(Font);
    TextBlock->SetColorAndOpacity(Style.Color);
    TextBlock->SetShadowOffset(Style.ShadowOffset);
    TextBlock->SetShadowColorAndOpacity(Style.ShadowColor);
}
