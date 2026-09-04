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
#include "Vehicles/TMOPVehicleBase.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
FName TypographyFallback(const FName StyleId)
{
    const FString Id = StyleId.ToString();
    if (Id.EndsWith(TEXT("3D"))) return TEXT("AgentName");
    if (Id.Contains(TEXT("Speaker"))) return TEXT("SpeakerName");
    if (Id.Contains(TEXT("Subtitle"))) return TEXT("Subtitle");
    if (Id.Contains(TEXT("Button")) || Id.Contains(TEXT("Navigation")))
        return TEXT("MenuButton");
    if (Id.Contains(TEXT("Heading")) || Id.Contains(TEXT("Title")) ||
        Id.Contains(TEXT("Name"))) return TEXT("Heading");
    if (Id.Contains(TEXT("Caption")) || Id.Contains(TEXT("Hint")) ||
        Id.Contains(TEXT("Details")) || Id.Contains(TEXT("Status")) ||
        Id.Contains(TEXT("PageNumber"))) return TEXT("Caption");
    if (Id.Contains(TEXT("Target")) || Id.Contains(TEXT("Interaction")))
        return TEXT("TargetLabel");
    return TEXT("Body");
}
}

ATMOPTypographyDirector::ATMOPTypographyDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    const auto AddUsage = [this](const TCHAR* Id, const TCHAR* Usage)
    {
        FTMOPTypographyUsageReference& Entry = StyleUsageReference.AddDefaulted_GetRef();
        Entry.StyleId = FName(Id);
        Entry.UsedBy = FText::FromString(FString(Usage));
    };
    AddUsage(TEXT("MainMenuButton"), TEXT("Startmenyn: Starta nytt, Ladda, Inställningar och Stäng av"));
    AddUsage(TEXT("MainMenuLoadHeading"), TEXT("Startmenyn: rubriken Ladda spel"));
    AddUsage(TEXT("MainMenuLoadStatus"), TEXT("Startmenyn: status- och felmeddelanden vid laddning"));
    AddUsage(TEXT("MainMenuSaveTitle"), TEXT("Startmenyn: sparfilens namn och klockslag"));
    AddUsage(TEXT("MainMenuSaveDetails"), TEXT("Startmenyn: plats, nivå och sparningsdatum"));
    AddUsage(TEXT("IntroCardHeading"), TEXT("Introsekvensens centrerade kortrubrik"));
    AddUsage(TEXT("IntroCardBody"), TEXT("Introsekvensens centrerade korttext"));
    AddUsage(TEXT("IntroSkipButton"), TEXT("SKIP-knappen nere till höger under bilintrot"));
    AddUsage(TEXT("PauseMenuMainTitle"), TEXT("Pausmenyn: THE MURDER OF OLOF PALME"));
    AddUsage(TEXT("PauseMenuNavigation"), TEXT("Pausmenyns vänstra navigationsknappar"));
    AddUsage(TEXT("PauseMenuSectionTitle"), TEXT("Pausmenyn: aktuell sidas stora rubrik"));
    AddUsage(TEXT("PauseMenuSectionHeading"), TEXT("Pausmenyn: underrubriker inne på en sida"));
    AddUsage(TEXT("PauseMenuBody"), TEXT("Pausmenyn: vanlig sidtext"));
    AddUsage(TEXT("PauseMenuStatus"), TEXT("Pausmenyn: status- och felmeddelanden"));
    AddUsage(TEXT("PauseMenuSourceHeading"), TEXT("Källor/Uppslag: uppslagets ID och titel"));
    AddUsage(TEXT("PauseMenuSourceDetails"), TEXT("Källor/Uppslag: status, datum och webbadress"));
    AddUsage(TEXT("PauseMenuSaveTitle"), TEXT("Spara/Ladda: sparfilens namn och speltid"));
    AddUsage(TEXT("PauseMenuSaveDetails"), TEXT("Spara/Ladda: plats, nivå och sparningsdatum"));
    AddUsage(TEXT("DialogSpeakerName"), TEXT("Dialog och radio: talarens namn"));
    AddUsage(TEXT("DialogBody"), TEXT("Dialogrutan: repliken"));
    AddUsage(TEXT("Subtitle"), TEXT("Undertexter i världen och filmsekvenser"));
    AddUsage(TEXT("InteractionReticle"), TEXT("Siktpunkt vid interaktion"));
    AddUsage(TEXT("InteractionTargetName"), TEXT("Namnet på objektet spelaren tittar på"));
    AddUsage(TEXT("InteractionHint"), TEXT("Extra instruktion under interaktionsnamnet"));
    AddUsage(TEXT("InteractionPrompt"), TEXT("Tryck E och liknande interaktionsuppmaningar"));
    AddUsage(TEXT("QuickInventoryHeading"), TEXT("Snabbinventariets mittrubrik"));
    AddUsage(TEXT("QuickInventoryItem"), TEXT("Föremålsnamn i snabbinventariet"));
    AddUsage(TEXT("NewspaperTitle"), TEXT("Tidningsläsaren: tidningens namn och datum"));
    AddUsage(TEXT("NewspaperPageNumber"), TEXT("Tidningsläsaren: uppslag och sidnummer"));
    AddUsage(TEXT("NewspaperHint"), TEXT("Tidningsläsaren: kontroller och sidbeskrivning"));
    AddUsage(TEXT("SpeechBubble"), TEXT("Pratbubblor ovanför personer"));
    AddUsage(TEXT("AgentInfoHeading"), TEXT("Personkort: sektionsrubriker"));
    AddUsage(TEXT("AgentInfoName"), TEXT("Personkort: personens namn"));
    AddUsage(TEXT("AgentInfoIdentity"), TEXT("Personkort: yrke, ålder och uppslag"));
    AddUsage(TEXT("AgentInfoStatus"), TEXT("Personkort: förhörsstatus"));
    AddUsage(TEXT("AgentInfoBody"), TEXT("Personkort: tidslinje och observationer"));
    AddUsage(TEXT("AgentInfoSources"), TEXT("Personkort: källhänvisningar"));
    AddUsage(TEXT("MapMarkerLabel"), TEXT("Stora kartan: namn vid platsmarkörer"));
    AddUsage(TEXT("MapLegendHeading"), TEXT("Stora kartan: rubriken Teckenförklaring"));
    AddUsage(TEXT("MapLegendBody"), TEXT("Stora kartan: teckenförklaring och filter"));
    AddUsage(TEXT("MapFixedLabel"), TEXT("Stora kartan: Du är här och Olof Palme"));
    AddUsage(TEXT("MapHint"), TEXT("Stora kartan: kontroller och hjälptext"));
    AddUsage(TEXT("HUDClock"), TEXT("Blueprint HUD: spelets klockslag"));
    AddUsage(TEXT("HUDDate"), TEXT("Blueprint HUD: datum"));
    AddUsage(TEXT("HUDCountdown"), TEXT("Blueprint HUD: tid till eller efter mordet"));
    AddUsage(TEXT("HUDObjective"), TEXT("Blueprint HUD: aktuellt mål eller instruktion"));
    AddUsage(TEXT("AgentName3D"), TEXT("3D-namn ovanför personer; använd endast legacy offline UFont om World Font aktiveras"));
    AddUsage(TEXT("VehicleName3D"), TEXT("3D-namn ovanför historiska fordon"));
}

const FTMOPTypographyStyleRow* ATMOPTypographyDirector::FindExactStyle(
    const FName StyleId) const
{
    if (!IsValid(TypographyTable) || StyleId.IsNone()) return nullptr;
    if (const FTMOPTypographyStyleRow* Direct =
        TypographyTable->FindRow<FTMOPTypographyStyleRow>(StyleId,
            TEXT("TMOP Typography"), false)) return Direct;
    for (const TPair<FName, uint8*>& Pair : TypographyTable->GetRowMap())
    {
        const FTMOPTypographyStyleRow* Row =
            reinterpret_cast<const FTMOPTypographyStyleRow*>(Pair.Value);
        if (Row != nullptr && Row->StyleId == StyleId) return Row;
    }
    return nullptr;
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
    const FTMOPTypographyStyleRow* Style = FindExactStyle(StyleId);
    if (Style == nullptr)
    {
        const FName FallbackId = TypographyFallback(StyleId);
        if (FallbackId != StyleId) Style = FindExactStyle(FallbackId);
    }
    if (Style == nullptr) return false;
    OutStyle = *Style;
    return true;
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
    const bool bWasVisible = TextRender->IsVisible();
    if (Style.bOverrideWorldFont)
        if (UFont* Font = Cast<UFont>(Style.FontAsset.LoadSynchronous()))
            TextRender->SetFont(Font);
    if (Style.bOverrideWorldSize && Style.Size > 0)
        TextRender->SetWorldSize(static_cast<float>(Style.Size));
    if (Style.bOverrideWorldColor && Style.Color.A > KINDA_SMALL_NUMBER)
        TextRender->SetTextRenderColor(Style.Color.ToFColor(true));
    TextRender->SetVisibility(bWasVisible, true);
    return true;
}

FName ATMOPTypographyDirector::InferStyleId(const UTextBlock* TextBlock) const
{
    if (!IsValid(TextBlock)) return TEXT("Body");
    const FName WidgetName = TextBlock->GetFName();
    if (const FName* Override = WidgetNameStyleOverrides.Find(WidgetName))
        return *Override;
    const FString Name = WidgetName.ToString().ToLower();
    if (Name.Contains(TEXT("countdown")) || Name.Contains(TEXT("murdertime")))
        return TEXT("HUDCountdown");
    if (Name.Contains(TEXT("clock")) || Name.Contains(TEXT("gametime")))
        return TEXT("HUDClock");
    if (Name.Contains(TEXT("date"))) return TEXT("HUDDate");
    if (Name.Contains(TEXT("objective")) || Name.Contains(TEXT("mission")))
        return TEXT("HUDObjective");
    if (Name.Contains(TEXT("agentname")) || Name.Contains(TEXT("nameplate")))
        return TEXT("AgentName");
    if (Name.Contains(TEXT("mainmenu")) && Name.Contains(TEXT("button")))
        return TEXT("MainMenuButton");
    if (Name.Contains(TEXT("pause")) && Name.Contains(TEXT("title")))
        return TEXT("PauseMenuSectionTitle");
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
        {
            It->RefreshNameLabel();
            ApplyWorldTextStyle(It->NameLabel, TEXT("AgentName3D"));
        }
    for (TActorIterator<ATMOPVehicleBase> It(GetWorld()); It; ++It)
        if (IsValid(It->NameLabel))
        {
            It->RefreshNameLabel();
            ApplyWorldTextStyle(It->NameLabel, TEXT("VehicleName3D"));
        }
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

FTMOPMenuColorPalette ATMOPTypographyDirector::ResolveMenuColors(
    const UObject* WorldContext)
{
    const ATMOPTypographyDirector* Director = Find(WorldContext);
    return IsValid(Director) ? Director->MenuColors : FTMOPMenuColorPalette();
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
