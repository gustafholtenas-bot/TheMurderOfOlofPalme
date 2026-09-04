#include "TMOPNewspaperFolderImporter.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "ContentBrowserModule.h"
#include "Engine/Texture2D.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"
#include "ObjectTools.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "TMOPNewspaperFolderImporter"

namespace
{
struct FImportResult
{
    int32 Created = 0;
    int32 Updated = 0;
    int32 Skipped = 0;
    TArray<FString> Warnings;
    TArray<UPackage*> PackagesToSave;
};

bool IsDigit(const TCHAR Character)
{
    return Character >= TEXT('0') && Character <= TEXT('9');
}

FString TrimLeadingZeros(const FString& Number)
{
    int32 FirstNonZero = 0;
    while (FirstNonZero < Number.Len() && Number[FirstNonZero] == TEXT('0'))
    {
        ++FirstNonZero;
    }
    return FirstNonZero == Number.Len()
        ? TEXT("0")
        : Number.Mid(FirstNonZero);
}

bool NaturalLess(const FString& Left, const FString& Right)
{
    int32 LeftIndex = 0;
    int32 RightIndex = 0;
    while (LeftIndex < Left.Len() && RightIndex < Right.Len())
    {
        if (IsDigit(Left[LeftIndex]) && IsDigit(Right[RightIndex]))
        {
            int32 LeftEnd = LeftIndex;
            int32 RightEnd = RightIndex;
            while (LeftEnd < Left.Len() && IsDigit(Left[LeftEnd]))
            {
                ++LeftEnd;
            }
            while (RightEnd < Right.Len() && IsDigit(Right[RightEnd]))
            {
                ++RightEnd;
            }

            const FString LeftNumber = Left.Mid(LeftIndex, LeftEnd - LeftIndex);
            const FString RightNumber = Right.Mid(RightIndex, RightEnd - RightIndex);
            const FString LeftTrimmed = TrimLeadingZeros(LeftNumber);
            const FString RightTrimmed = TrimLeadingZeros(RightNumber);
            if (LeftTrimmed.Len() != RightTrimmed.Len())
            {
                return LeftTrimmed.Len() < RightTrimmed.Len();
            }
            const int32 NumericCompare = LeftTrimmed.Compare(
                RightTrimmed, ESearchCase::CaseSensitive);
            if (NumericCompare != 0)
            {
                return NumericCompare < 0;
            }
            if (LeftNumber.Len() != RightNumber.Len())
            {
                return LeftNumber.Len() < RightNumber.Len();
            }

            LeftIndex = LeftEnd;
            RightIndex = RightEnd;
            continue;
        }

        const TCHAR LeftCharacter = FChar::ToLower(Left[LeftIndex]);
        const TCHAR RightCharacter = FChar::ToLower(Right[RightIndex]);
        if (LeftCharacter != RightCharacter)
        {
            return LeftCharacter < RightCharacter;
        }
        ++LeftIndex;
        ++RightIndex;
    }
    return Left.Len() < Right.Len();
}

int32 PagePositionRank(const FString& AssetName)
{
    const FString LowerName = AssetName.ToLower();
    if (LowerName.Contains(TEXT("front")) ||
        LowerName.Contains(TEXT("cover")) ||
        LowerName.Contains(TEXT("framsida")))
    {
        return 0;
    }
    if (LowerName.Contains(TEXT("back")) ||
        LowerName.Contains(TEXT("baksida")))
    {
        return 2;
    }
    return 1;
}

int32 ExtractLastNumber(const FString& Text)
{
    int32 Result = INDEX_NONE;
    int32 Index = 0;
    while (Index < Text.Len())
    {
        if (!IsDigit(Text[Index]))
        {
            ++Index;
            continue;
        }

        int32 End = Index;
        while (End < Text.Len() && IsDigit(Text[End]))
        {
            ++End;
        }
        LexTryParseString(Result, *Text.Mid(Index, End - Index));
        Index = End;
    }
    return Result;
}

FString MakeDisplayName(const FString& FolderName)
{
    FString Result = FolderName;
    Result.ReplaceInline(TEXT("_"), TEXT(" "));
    Result.ReplaceInline(TEXT("-"), TEXT(" "));
    return Result.TrimStartAndEnd();
}

FString MakeItemId(const FString& FolderPath)
{
    FString Result = FolderPath;
    Result.RemoveFromStart(TEXT("/Game/"), ESearchCase::IgnoreCase);
    Result.ReplaceInline(TEXT("/"), TEXT("_"));
    Result.ReplaceInline(TEXT(" "), TEXT("_"));
    Result = ObjectTools::SanitizeObjectName(Result).ToUpper();
    return FString::Printf(TEXT("NEWSPAPER_%s"), *Result);
}

/**
 * GetSelectedPathViewFolders may return a Content Browser virtual path. The
 * Asset Registry only accepts mounted package paths such as /Game/..., so
 * strip the virtual All/Content roots before building FARFilters.
 */
FString ToPackageFolderPath(FString FolderPath)
{
    FolderPath.ReplaceInline(TEXT("\\"), TEXT("/"));

    if (FolderPath.Equals(TEXT("/All/Game"), ESearchCase::IgnoreCase) ||
        FolderPath.Equals(TEXT("/All/Content"), ESearchCase::IgnoreCase) ||
        FolderPath.Equals(TEXT("/Content"), ESearchCase::IgnoreCase))
    {
        return TEXT("/Game");
    }
    if (FolderPath.StartsWith(TEXT("/All/Game/"), ESearchCase::IgnoreCase))
    {
        return FolderPath.Mid(4);
    }
    if (FolderPath.StartsWith(TEXT("/All/Content/"), ESearchCase::IgnoreCase))
    {
        return FString(TEXT("/Game/")) + FolderPath.Mid(13);
    }
    if (FolderPath.StartsWith(TEXT("/Content/"), ESearchCase::IgnoreCase))
    {
        return FString(TEXT("/Game/")) + FolderPath.Mid(9);
    }
    return FolderPath;
}

FString InferPublicationDate(const FString& Text)
{
    for (int32 Index = 0; Index + 10 <= Text.Len(); ++Index)
    {
        if (IsDigit(Text[Index]) && IsDigit(Text[Index + 1]) &&
            IsDigit(Text[Index + 2]) && IsDigit(Text[Index + 3]) &&
            (Text[Index + 4] == TEXT('-') || Text[Index + 4] == TEXT('_')) &&
            IsDigit(Text[Index + 5]) && IsDigit(Text[Index + 6]) &&
            (Text[Index + 7] == TEXT('-') || Text[Index + 7] == TEXT('_')) &&
            IsDigit(Text[Index + 8]) && IsDigit(Text[Index + 9]))
        {
            FString Result = Text.Mid(Index, 10);
            Result.ReplaceInline(TEXT("_"), TEXT("-"));
            return Result;
        }
    }

    for (int32 Index = 0; Index + 8 <= Text.Len(); ++Index)
    {
        bool bAllDigits = true;
        for (int32 DigitIndex = 0; DigitIndex < 8; ++DigitIndex)
        {
            bAllDigits &= IsDigit(Text[Index + DigitIndex]);
        }
        if (bAllDigits)
        {
            const FString Digits = Text.Mid(Index, 8);
            return FString::Printf(TEXT("%s-%s-%s"),
                *Digits.Left(4), *Digits.Mid(4, 2), *Digits.Right(2));
        }
    }
    return FString();
}

ETMOPNewspaperPublication InferPublication(const FString& Text)
{
    FString Lower = Text.ToLower();
    Lower.ReplaceInline(TEXT("_"), TEXT(""));
    Lower.ReplaceInline(TEXT("-"), TEXT(""));
    Lower.ReplaceInline(TEXT(" "), TEXT(""));

    if (Lower.Contains(TEXT("aftonbladet")))
    {
        return ETMOPNewspaperPublication::Aftonbladet;
    }
    if (Lower.Contains(TEXT("dagensnyheter")) || Lower.StartsWith(TEXT("dn")))
    {
        return ETMOPNewspaperPublication::DagensNyheter;
    }
    if (Lower.Contains(TEXT("expressen")))
    {
        return ETMOPNewspaperPublication::Expressen;
    }
    if (Lower.Contains(TEXT("svenskadagbladet")) || Lower.StartsWith(TEXT("svd")))
    {
        return ETMOPNewspaperPublication::SvenskaDagbladet;
    }
    if (Lower.Contains(TEXT("goteborgsposten")) ||
        Lower.Contains(TEXT("göteborgsposten")) || Lower.StartsWith(TEXT("gp")))
    {
        return ETMOPNewspaperPublication::GoteborgsPosten;
    }
    if (Lower.Contains(TEXT("dagensindustri")) || Lower.StartsWith(TEXT("di")))
    {
        return ETMOPNewspaperPublication::DagensIndustri;
    }
    if (Lower.Contains(TEXT("arbetet")))
    {
        return ETMOPNewspaperPublication::Arbetet;
    }
    return ETMOPNewspaperPublication::Other;
}

void FindAssetsInFolder(
    IAssetRegistry& AssetRegistry,
    const FString& FolderPath,
    const FTopLevelAssetPath& ClassPath,
    TArray<FAssetData>& OutAssets)
{
    FARFilter Filter;
    Filter.PackagePaths.Add(FName(*FolderPath));
    Filter.ClassPaths.Add(ClassPath);
    Filter.bRecursivePaths = false;
    Filter.bRecursiveClasses = true;
    AssetRegistry.GetAssets(Filter, OutAssets);
}

UTMOPNewspaperItemDefinition* FindOrCreateNewspaper(
    IAssetRegistry& AssetRegistry,
    const FString& FolderPath,
    const FString& FolderName,
    bool& bOutCreated,
    FString& OutWarning)
{
    bOutCreated = false;
    TArray<FAssetData> ExistingAssets;
    FindAssetsInFolder(AssetRegistry, FolderPath,
        UTMOPNewspaperItemDefinition::StaticClass()->GetClassPathName(),
        ExistingAssets);

    const FString GeneratedAssetName = ObjectTools::SanitizeObjectName(
        FString::Printf(TEXT("DA_TMOP_Newspaper_%s"), *FolderName));
    for (const FAssetData& Asset : ExistingAssets)
    {
        if (Asset.AssetName.ToString().Equals(
            GeneratedAssetName, ESearchCase::IgnoreCase))
        {
            return Cast<UTMOPNewspaperItemDefinition>(Asset.GetAsset());
        }
    }
    if (ExistingAssets.Num() == 1)
    {
        return Cast<UTMOPNewspaperItemDefinition>(ExistingAssets[0].GetAsset());
    }
    if (ExistingAssets.Num() > 1)
    {
        OutWarning = FString::Printf(
            TEXT("%s: multiple newspaper assets exist; rename the intended one to %s."),
            *FolderPath, *GeneratedAssetName);
        return nullptr;
    }

    FString PackageName = FPaths::Combine(FolderPath, GeneratedAssetName);
    PackageName.ReplaceInline(TEXT("\\"), TEXT("/"));
    UPackage* Package = CreatePackage(*PackageName);
    if (Package == nullptr)
    {
        OutWarning = FString::Printf(TEXT("%s: could not create package."),
            *FolderPath);
        return nullptr;
    }

    UTMOPNewspaperItemDefinition* Newspaper =
        NewObject<UTMOPNewspaperItemDefinition>(Package,
            *GeneratedAssetName, RF_Public | RF_Standalone | RF_Transactional);
    if (Newspaper != nullptr)
    {
        FAssetRegistryModule::AssetCreated(Newspaper);
        bOutCreated = true;
    }
    return Newspaper;
}

void AddPageOrderWarnings(
    const FString& FolderPath,
    const TArray<FAssetData>& Textures,
    TArray<FString>& OutWarnings)
{
    TMap<int32, FString> NumberOwners;
    int32 NumberedTextureCount = 0;
    int32 MinimumNumber = MAX_int32;
    int32 MaximumNumber = MIN_int32;

    for (const FAssetData& Texture : Textures)
    {
        const int32 Number = ExtractLastNumber(Texture.AssetName.ToString());
        if (Number == INDEX_NONE)
        {
            continue;
        }
        ++NumberedTextureCount;
        MinimumNumber = FMath::Min(MinimumNumber, Number);
        MaximumNumber = FMath::Max(MaximumNumber, Number);
        if (const FString* ExistingOwner = NumberOwners.Find(Number))
        {
            OutWarnings.Add(FString::Printf(
                TEXT("%s: page number %d occurs in both %s and %s."),
                *FolderPath, Number, **ExistingOwner,
                *Texture.AssetName.ToString()));
        }
        else
        {
            NumberOwners.Add(Number, Texture.AssetName.ToString());
        }
    }

    if (NumberedTextureCount == 0)
    {
        OutWarnings.Add(FString::Printf(
            TEXT("%s: filenames contain no page numbers; alphabetical order was used."),
            *FolderPath));
        return;
    }
    if (NumberedTextureCount != Textures.Num())
    {
        OutWarnings.Add(FString::Printf(
            TEXT("%s: %d of %d filenames have no page number; check the generated order."),
            *FolderPath, Textures.Num() - NumberedTextureCount, Textures.Num()));
    }
    if (MinimumNumber != MAX_int32 && MaximumNumber - MinimumNumber < 1000)
    {
        for (int32 Number = MinimumNumber; Number <= MaximumNumber; ++Number)
        {
            if (!NumberOwners.Contains(Number))
            {
                OutWarnings.Add(FString::Printf(
                    TEXT("%s: page number %d is missing."), *FolderPath, Number));
            }
        }
    }
}

bool ImportFolder(
    IAssetRegistry& AssetRegistry,
    const FString& FolderPath,
    FImportResult& OutResult)
{
    TArray<FAssetData> Textures;
    FindAssetsInFolder(AssetRegistry, FolderPath,
        UTexture2D::StaticClass()->GetClassPathName(), Textures);
    if (Textures.IsEmpty())
    {
        return false;
    }

    Textures.Sort([](const FAssetData& Left, const FAssetData& Right)
    {
        const FString LeftName = Left.AssetName.ToString();
        const FString RightName = Right.AssetName.ToString();
        const int32 LeftRank = PagePositionRank(LeftName);
        const int32 RightRank = PagePositionRank(RightName);
        return LeftRank == RightRank
            ? NaturalLess(LeftName, RightName)
            : LeftRank < RightRank;
    });
    AddPageOrderWarnings(FolderPath, Textures, OutResult.Warnings);

    const FString FolderName = FPackageName::GetShortName(FolderPath);
    bool bCreated = false;
    FString Warning;
    UTMOPNewspaperItemDefinition* Newspaper = FindOrCreateNewspaper(
        AssetRegistry, FolderPath, FolderName, bCreated, Warning);
    if (Newspaper == nullptr)
    {
        ++OutResult.Skipped;
        OutResult.Warnings.Add(Warning.IsEmpty()
            ? FString::Printf(TEXT("%s: newspaper asset could not be loaded."),
                *FolderPath)
            : Warning);
        return true;
    }

    Newspaper->Modify();
    const FString DisplayName = MakeDisplayName(FolderName);
    Newspaper->ItemId = FName(*MakeItemId(FolderPath));
    Newspaper->DisplayName = FText::FromString(DisplayName);
    Newspaper->EditionName = FText::FromString(DisplayName);
    Newspaper->Publication = InferPublication(FolderPath);
    const FString InferredDate = InferPublicationDate(FolderPath);
    if (!InferredDate.IsEmpty())
    {
        Newspaper->PublicationDate = InferredDate;
    }
    Newspaper->bAutomaticallyNumberPages = true;
    Newspaper->Pages.Reset(Textures.Num());
    for (int32 PageIndex = 0; PageIndex < Textures.Num(); ++PageIndex)
    {
        FTMOPNewspaperPage& Page = Newspaper->Pages.AddDefaulted_GetRef();
        Page.PageImage = TSoftObjectPtr<UTexture2D>(Textures[PageIndex].ToSoftObjectPath());
        Page.PrintedPageNumber = PageIndex + 1;
        Page.PageLabel = FText::Format(
            LOCTEXT("GeneratedPageLabel", "Sida {0}"), PageIndex + 1);
    }
    Newspaper->Icon = Cast<UTexture2D>(Textures[0].GetAsset());
    Newspaper->PostEditChange();
    Newspaper->MarkPackageDirty();
    OutResult.PackagesToSave.AddUnique(Newspaper->GetOutermost());
    bCreated ? ++OutResult.Created : ++OutResult.Updated;
    return true;
}
}

void FTMOPNewspaperFolderImporter::ImportSelectedFolders()
{
    FContentBrowserModule& ContentBrowserModule =
        FModuleManager::LoadModuleChecked<FContentBrowserModule>(
            TEXT("ContentBrowser"));
    TArray<FString> SelectedFolders;
    ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedFolders);
    if (SelectedFolders.IsEmpty())
    {
        FMessageDialog::Open(EAppMsgType::Ok,
            LOCTEXT("NoFoldersSelected",
                "Select one or more newspaper folders in the Content Browser first."));
        return;
    }

    IAssetRegistry& AssetRegistry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
            TEXT("AssetRegistry")).Get();
    FImportResult Result;
    TSet<FString> ProcessedFolders;

    for (const FString& SelectedFolder : SelectedFolders)
    {
        const FString PackageFolder = ToPackageFolderPath(SelectedFolder);
        if (ProcessedFolders.Contains(PackageFolder))
        {
            continue;
        }
        ProcessedFolders.Add(PackageFolder);

        const bool bSelectedFolderHadTextures =
            ImportFolder(AssetRegistry, PackageFolder, Result);
        if (!bSelectedFolderHadTextures)
        {
            TArray<FString> ChildFolders;
            AssetRegistry.GetSubPaths(PackageFolder, ChildFolders, false);
            ChildFolders.Sort([](const FString& Left, const FString& Right)
            {
                return NaturalLess(Left, Right);
            });
            for (const FString& ChildFolder : ChildFolders)
            {
                if (!ProcessedFolders.Contains(ChildFolder))
                {
                    ProcessedFolders.Add(ChildFolder);
                    ImportFolder(AssetRegistry, ChildFolder, Result);
                }
            }
        }
    }

    if (Result.PackagesToSave.IsEmpty())
    {
        FMessageDialog::Open(EAppMsgType::Ok,
            LOCTEXT("NoNewspaperTextures",
                "No Texture2D page images were found directly in the selected folders or their direct child folders."));
        return;
    }

    const bool bSaved = FEditorFileUtils::PromptForCheckoutAndSave(
        Result.PackagesToSave, false, false) ==
        FEditorFileUtils::EPromptReturnCode::PR_Success;

    FString Summary = FString::Printf(
        TEXT("Newspapers finished. Created: %d, updated: %d, skipped: %d.\nSaved: %s."),
        Result.Created, Result.Updated, Result.Skipped,
        bSaved ? TEXT("yes") : TEXT("no/cancelled"));
    if (!Result.Warnings.IsEmpty())
    {
        Summary += TEXT("\n\nCheck these page-order warnings:\n- ");
        Summary += FString::Join(Result.Warnings, TEXT("\n- "));
    }
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Summary));
}

#undef LOCTEXT_NAMESPACE
