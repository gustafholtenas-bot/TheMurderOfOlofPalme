#include "TMOPEngineEditor.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Framework/Docking/TabManager.h"
#include "ScopedTransaction.h"
#include "STMOPPeopleEditor.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "TMOPEngineEditor"

const FName FTMOPEngineEditorModule::PeopleEditorTabName(
    TEXT("TMOPPeopleEditor"));

void FTMOPEngineEditorModule::StartupModule()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        PeopleEditorTabName,
        FOnSpawnTab::CreateRaw(
            this, &FTMOPEngineEditorModule::SpawnPeopleEditorTab))
        .SetDisplayName(LOCTEXT("PeopleEditorTitle", "TMOP People Editor"))
        .SetTooltipText(LOCTEXT(
            "PeopleEditorTooltip",
            "Edit DT_TMOP_People in a focused visual timeline editor."))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this, &FTMOPEngineEditorModule::RegisterMenus));
}

void FTMOPEngineEditorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PeopleEditorTabName);
}

void FTMOPEngineEditorModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);
    UToolMenu* ToolsMenu =
        UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
    FToolMenuSection& Section =
        ToolsMenu->FindOrAddSection(TEXT("TMOP"));
    Section.AddMenuEntry(
        TEXT("OpenTMOPPeopleEditor"),
        LOCTEXT("OpenPeopleEditor", "TMOP People Editor"),
        LOCTEXT("OpenPeopleEditorTooltip",
            "Open the visual person and timeline editor."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(
            this, &FTMOPEngineEditorModule::OpenPeopleEditor)));
    Section.AddMenuEntry(
        TEXT("GenerateTMOPExitChildren"),
        LOCTEXT("GenerateExitChildren",
            "Generate Exit Anchors From Selection"),
        LOCTEXT("GenerateExitChildrenTooltip",
            "Create two sidewalk exits, one car entry and one car exit for every selected historical anchor."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(
            this,
            &FTMOPEngineEditorModule::GenerateExitChildrenFromSelection)));
    Section.AddMenuEntry(
        TEXT("GenerateTMOPIntersectionCorners"),
        LOCTEXT("GenerateIntersectionCorners",
            "Generate Intersection Corners From Selection"),
        LOCTEXT("GenerateIntersectionCornersTooltip",
            "Create NW, NE, SW and SE street-corner anchors for every selected historical anchor."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(
            this,
            &FTMOPEngineEditorModule::GenerateIntersectionCornersFromSelection)));
}

void FTMOPEngineEditorModule::OpenPeopleEditor()
{
    FGlobalTabmanager::Get()->TryInvokeTab(PeopleEditorTabName);
}

void FTMOPEngineEditorModule::GenerateExitChildrenFromSelection()
{
    if (GEditor == nullptr || GEditor->GetSelectedActors() == nullptr)
    {
        return;
    }

    TArray<ATMOPHistoricalAnchor*> Parents;
    for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
    {
        if (ATMOPHistoricalAnchor* Anchor =
            Cast<ATMOPHistoricalAnchor>(*It))
        {
            if (!Anchor->GetAnchorId().IsNone())
            {
                Parents.Add(Anchor);
            }
        }
    }
    if (Parents.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP exit generator: select one or more Historical Anchors first."));
        return;
    }

    const FScopedTransaction Transaction(
        LOCTEXT("GenerateExitAnchorTransaction",
            "Generate TMOP Exit Anchors"));
    TArray<AActor*> CreatedActors;

    for (ATMOPHistoricalAnchor* Parent : Parents)
    {
        UWorld* World = Parent != nullptr ? Parent->GetWorld() : nullptr;
        if (!IsValid(Parent) || World == nullptr)
        {
            continue;
        }

        FString Stem = Parent->GetAnchorId().ToString();
        if (Stem.StartsWith(TEXT("Exit"), ESearchCase::IgnoreCase))
        {
            Stem.RightChopInline(4, EAllowShrinking::No);
        }
        else if (Stem.StartsWith(TEXT("Enter"), ESearchCase::IgnoreCase))
        {
            Stem.RightChopInline(5, EAllowShrinking::No);
        }
        while (Stem.StartsWith(TEXT("_")))
        {
            Stem.RightChopInline(1, EAllowShrinking::No);
        }

        const TArray<FString> ChildIds =
        {
            FString::Printf(TEXT("Exit%s_Sidewalk1"), *Stem),
            FString::Printf(TEXT("Exit%s_Sidewalk2"), *Stem),
            FString::Printf(TEXT("Enter%s_Car"), *Stem),
            FString::Printf(TEXT("Exit%s_Car"), *Stem)
        };

        TSet<FName> ExistingIds;
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        {
            ExistingIds.Add(It->GetAnchorId());
        }

        for (const FString& ChildIdString : ChildIds)
        {
            const FName ChildId(*ChildIdString);
            if (ExistingIds.Contains(ChildId))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("TMOP exit generator: '%s' already exists; skipped."),
                    *ChildIdString);
                continue;
            }

            FActorSpawnParameters Parameters;
            Parameters.OverrideLevel = Parent->GetLevel();
            Parameters.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ATMOPHistoricalAnchor* Child =
                World->SpawnActor<ATMOPHistoricalAnchor>(
                    ATMOPHistoricalAnchor::StaticClass(),
                    Parent->GetActorTransform(), Parameters);
            if (!IsValid(Child))
            {
                continue;
            }

            Child->Modify();
            Child->SetActorLabel(ChildIdString);
            const FString ParentFolder =
                Parent->GetFolderPath().ToString();
            const FString ChildFolder = ParentFolder.IsEmpty()
                ? Parent->GetAnchorId().ToString()
                : FString::Printf(TEXT("%s/%s"), *ParentFolder,
                    *Parent->GetAnchorId().ToString());
            Child->SetFolderPath(FName(*ChildFolder));
            Child->AnchorCategory = ETMOPAnchorCategory::MapExit;
            Child->ParentAnchorId = Parent->GetAnchorId();
            Child->DisplayName = FText::FromString(ChildIdString);
            Child->bCanBeUsedForRouting = true;
            if (IsValid(Child->EntityIdentity))
            {
                Child->EntityIdentity->SetEntityIdentity(
                    ChildId, TEXT("Anchor"));
            }
            Child->PostEditChange();
            Child->MarkPackageDirty();
            ExistingIds.Add(ChildId);
            CreatedActors.Add(Child);
        }
    }

    GEditor->SelectNone(false, true, false);
    for (AActor* Actor : CreatedActors)
    {
        GEditor->SelectActor(Actor, true, false, true);
    }
    GEditor->NoteSelectionChange();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP exit generator: created %d anchor(s) from %d selected parent(s)."),
        CreatedActors.Num(), Parents.Num());
}

void FTMOPEngineEditorModule::GenerateIntersectionCornersFromSelection()
{
    if (GEditor == nullptr || GEditor->GetSelectedActors() == nullptr)
    {
        return;
    }

    TArray<ATMOPHistoricalAnchor*> Parents;
    for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
    {
        if (ATMOPHistoricalAnchor* Anchor =
            Cast<ATMOPHistoricalAnchor>(*It))
        {
            if (!Anchor->GetAnchorId().IsNone())
            {
                Parents.Add(Anchor);
            }
        }
    }
    if (Parents.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP intersection generator: select one or more Historical Anchors first."));
        return;
    }

    const FScopedTransaction Transaction(
        LOCTEXT("GenerateIntersectionCornerTransaction",
            "Generate TMOP Intersection Corners"));
    TArray<AActor*> CreatedActors;

    for (ATMOPHistoricalAnchor* Parent : Parents)
    {
        UWorld* World = Parent != nullptr ? Parent->GetWorld() : nullptr;
        if (!IsValid(Parent) || World == nullptr)
        {
            continue;
        }

        const FString Stem = Parent->GetAnchorId().ToString();
        const TArray<FString> ChildIds =
        {
            Stem + TEXT("_NW"),
            Stem + TEXT("_NE"),
            Stem + TEXT("_SW"),
            Stem + TEXT("_SE")
        };

        TSet<FName> ExistingIds;
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        {
            ExistingIds.Add(It->GetAnchorId());
        }

        for (const FString& ChildIdString : ChildIds)
        {
            const FName ChildId(*ChildIdString);
            if (ExistingIds.Contains(ChildId))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("TMOP intersection generator: '%s' already exists; skipped."),
                    *ChildIdString);
                continue;
            }

            FActorSpawnParameters Parameters;
            Parameters.OverrideLevel = Parent->GetLevel();
            Parameters.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ATMOPHistoricalAnchor* Child =
                World->SpawnActor<ATMOPHistoricalAnchor>(
                    ATMOPHistoricalAnchor::StaticClass(),
                    Parent->GetActorTransform(), Parameters);
            if (!IsValid(Child))
            {
                continue;
            }

            Child->Modify();
            Child->SetActorLabel(ChildIdString);
            const FString ParentFolder =
                Parent->GetFolderPath().ToString();
            const FString ChildFolder = ParentFolder.IsEmpty()
                ? Parent->GetAnchorId().ToString()
                : FString::Printf(TEXT("%s/%s"), *ParentFolder,
                    *Parent->GetAnchorId().ToString());
            Child->SetFolderPath(FName(*ChildFolder));
            Child->AnchorCategory = ETMOPAnchorCategory::StreetCorner;
            Child->ParentAnchorId = Parent->GetAnchorId();
            Child->DisplayName = FText::FromString(ChildIdString);
            Child->bCanBeUsedForRouting = true;
            Child->SurfacePreference =
                ETMOPRouteSurfacePreference::SidewalkPreferred;
            Child->bProjectPlacementToNavMesh = true;
            if (IsValid(Child->EntityIdentity))
            {
                Child->EntityIdentity->SetEntityIdentity(
                    ChildId, TEXT("Anchor"));
            }
            Child->PostEditChange();
            Child->MarkPackageDirty();
            ExistingIds.Add(ChildId);
            CreatedActors.Add(Child);
        }
    }

    GEditor->SelectNone(false, true, false);
    for (AActor* Actor : CreatedActors)
    {
        GEditor->SelectActor(Actor, true, false, true);
    }
    GEditor->NoteSelectionChange();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP intersection generator: created %d corner anchor(s) from %d selected parent(s)."),
        CreatedActors.Num(), Parents.Num());
}

TSharedRef<SDockTab> FTMOPEngineEditorModule::SpawnPeopleEditorTab(
    const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(STMOPPeopleEditor)
        ];
}

IMPLEMENT_MODULE(FTMOPEngineEditorModule, TMOPEngineEditor)

#undef LOCTEXT_NAMESPACE
