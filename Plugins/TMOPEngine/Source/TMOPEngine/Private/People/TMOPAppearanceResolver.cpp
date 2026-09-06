#include "People/TMOPAppearanceResolver.h"

#include "Engine/DataTable.h"

namespace
{
void AddTagIfContains(const FString& Text, const TCHAR* Needle,
    const FName Tag, TSet<FName>& OutTags)
{
    if (Text.Contains(Needle, ESearchCase::IgnoreCase)) OutTags.Add(Tag);
}

void AddFreeTextTags(const FString& Source, TSet<FName>& OutTags)
{
    FString Text = Source.ToLower();
    Text.ReplaceInline(TEXT("-"), TEXT(" "));

    // Colours. Specific dark colours deliberately also receive the generic
    // Dark tag, giving them a stronger score than a merely blue/grey asset.
    AddTagIfContains(Text, TEXT("mörkblå"), TEXT("DarkBlue"), OutTags);
    AddTagIfContains(Text, TEXT("mörk blå"), TEXT("DarkBlue"), OutTags);
    AddTagIfContains(Text, TEXT("marinblå"), TEXT("DarkBlue"), OutTags);
    AddTagIfContains(Text, TEXT("navy"), TEXT("DarkBlue"), OutTags);
    AddTagIfContains(Text, TEXT("svart"), TEXT("Black"), OutTags);
    AddTagIfContains(Text, TEXT("black"), TEXT("Black"), OutTags);
    AddTagIfContains(Text, TEXT("mörkgrå"), TEXT("DarkGrey"), OutTags);
    AddTagIfContains(Text, TEXT("mörk grå"), TEXT("DarkGrey"), OutTags);
    AddTagIfContains(Text, TEXT("grå"), TEXT("Grey"), OutTags);
    AddTagIfContains(Text, TEXT("grey"), TEXT("Grey"), OutTags);
    AddTagIfContains(Text, TEXT("gray"), TEXT("Grey"), OutTags);
    AddTagIfContains(Text, TEXT("mörkbrun"), TEXT("DarkBrown"), OutTags);
    AddTagIfContains(Text, TEXT("mörk brun"), TEXT("DarkBrown"), OutTags);
    AddTagIfContains(Text, TEXT("brun"), TEXT("Brown"), OutTags);
    AddTagIfContains(Text, TEXT("brown"), TEXT("Brown"), OutTags);
    AddTagIfContains(Text, TEXT("beige"), TEXT("Beige"), OutTags);
    AddTagIfContains(Text, TEXT("oliv"), TEXT("Olive"), OutTags);
    AddTagIfContains(Text, TEXT("röd"), TEXT("Red"), OutTags);
    AddTagIfContains(Text, TEXT("red"), TEXT("Red"), OutTags);
    AddTagIfContains(Text, TEXT("vit"), TEXT("White"), OutTags);
    AddTagIfContains(Text, TEXT("white"), TEXT("White"), OutTags);
    AddTagIfContains(Text, TEXT("blå"), TEXT("Blue"), OutTags);
    AddTagIfContains(Text, TEXT("blue"), TEXT("Blue"), OutTags);
    if (Text.Contains(TEXT("mörk")) || Text.Contains(TEXT("dark")) ||
        OutTags.Contains(TEXT("Black")) || OutTags.Contains(TEXT("DarkBlue")) ||
        OutTags.Contains(TEXT("DarkGrey")) || OutTags.Contains(TEXT("DarkBrown")))
        OutTags.Add(TEXT("Dark"));

    // Garment types and common 1986 witness wording.
    AddTagIfContains(Text, TEXT("täckjack"), TEXT("Jacket"), OutTags);
    AddTagIfContains(Text, TEXT("jack"), TEXT("Jacket"), OutTags);
    AddTagIfContains(Text, TEXT("jacket"), TEXT("Jacket"), OutTags);
    AddTagIfContains(Text, TEXT("skinnjack"), TEXT("LeatherJacket"), OutTags);
    AddTagIfContains(Text, TEXT("läderjack"), TEXT("LeatherJacket"), OutTags);
    AddTagIfContains(Text, TEXT("rock"), TEXT("Coat"), OutTags);
    AddTagIfContains(Text, TEXT("kappa"), TEXT("Coat"), OutTags);
    AddTagIfContains(Text, TEXT("coat"), TEXT("Coat"), OutTags);
    AddTagIfContains(Text, TEXT("parkas"), TEXT("Parka"), OutTags);
    AddTagIfContains(Text, TEXT("parka"), TEXT("Parka"), OutTags);
    AddTagIfContains(Text, TEXT("seglarjack"), TEXT("SailingJacket"), OutTags);
    AddTagIfContains(Text, TEXT("tröja"), TEXT("Sweater"), OutTags);
    AddTagIfContains(Text, TEXT("pullover"), TEXT("Sweater"), OutTags);
    AddTagIfContains(Text, TEXT("skjorta"), TEXT("Shirt"), OutTags);
    AddTagIfContains(Text, TEXT("byxor"), TEXT("Trousers"), OutTags);
    AddTagIfContains(Text, TEXT("byxa"), TEXT("Trousers"), OutTags);
    AddTagIfContains(Text, TEXT("jeans"), TEXT("Jeans"), OutTags);
    AddTagIfContains(Text, TEXT("kostymbyx"), TEXT("DressTrousers"), OutTags);
    AddTagIfContains(Text, TEXT("sko"), TEXT("Shoes"), OutTags);
    AddTagIfContains(Text, TEXT("stövel"), TEXT("Boots"), OutTags);
    AddTagIfContains(Text, TEXT("käng"), TEXT("Boots"), OutTags);
    AddTagIfContains(Text, TEXT("handsk"), TEXT("Gloves"), OutTags);
    AddTagIfContains(Text, TEXT("vante"), TEXT("Gloves"), OutTags);
    AddTagIfContains(Text, TEXT("stickad mössa"), TEXT("KnitCap"), OutTags);
    AddTagIfContains(Text, TEXT("yllemössa"), TEXT("KnitCap"), OutTags);
    AddTagIfContains(Text, TEXT("pälsmössa"), TEXT("FurHat"), OutTags);
    AddTagIfContains(Text, TEXT("keps"), TEXT("Cap"), OutTags);
    AddTagIfContains(Text, TEXT("hatt"), TEXT("Hat"), OutTags);
    AddTagIfContains(Text, TEXT("mössa"), TEXT("Cap"), OutTags);
    AddTagIfContains(Text, TEXT("huva"), TEXT("Hood"), OutTags);

    AddTagIfContains(Text, TEXT("lång"), TEXT("Long"), OutTags);
    AddTagIfContains(Text, TEXT("long"), TEXT("Long"), OutTags);
    AddTagIfContains(Text, TEXT("kort"), TEXT("Short"), OutTags);
    AddTagIfContains(Text, TEXT("short"), TEXT("Short"), OutTags);
    AddTagIfContains(Text, TEXT("ylle"), TEXT("Wool"), OutTags);
    AddTagIfContains(Text, TEXT("ull"), TEXT("Wool"), OutTags);
    AddTagIfContains(Text, TEXT("skinn"), TEXT("Leather"), OutTags);
    AddTagIfContains(Text, TEXT("läder"), TEXT("Leather"), OutTags);

    // Face descriptions used in Swedish witness statements.
    AddTagIfContains(Text, TEXT("rektangul"), TEXT("RectangularFace"), OutTags);
    AddTagIfContains(Text, TEXT("fyrkant"), TEXT("SquareFace"), OutTags);
    AddTagIfContains(Text, TEXT("runt ansikte"), TEXT("RoundFace"), OutTags);
    AddTagIfContains(Text, TEXT("rund ansikts"), TEXT("RoundFace"), OutTags);
    AddTagIfContains(Text, TEXT("ovalt"), TEXT("OvalFace"), OutTags);
    AddTagIfContains(Text, TEXT("smalt ansikte"), TEXT("NarrowFace"), OutTags);
    AddTagIfContains(Text, TEXT("brett ansikte"), TEXT("WideFace"), OutTags);
    AddTagIfContains(Text, TEXT("markerade kind"), TEXT("MarkedCheekbones"), OutTags);
    AddTagIfContains(Text, TEXT("höga kind"), TEXT("MarkedCheekbones"), OutTags);
    AddTagIfContains(Text, TEXT("framskjutet hak"), TEXT("ProjectedJaw"), OutTags);
    AddTagIfContains(Text, TEXT("kraftigt hak"), TEXT("StrongJaw"), OutTags);
    AddTagIfContains(Text, TEXT("stor näsa"), TEXT("LargeNose"), OutTags);
    AddTagIfContains(Text, TEXT("lång näsa"), TEXT("LongNose"), OutTags);
    AddTagIfContains(Text, TEXT("smal näsa"), TEXT("NarrowNose"), OutTags);
    AddTagIfContains(Text, TEXT("bred näsa"), TEXT("WideNose"), OutTags);
    AddTagIfContains(Text, TEXT("liten näsa"), TEXT("SmallNose"), OutTags);
    AddTagIfContains(Text, TEXT("tunna läpp"), TEXT("ThinLips"), OutTags);
    AddTagIfContains(Text, TEXT("smala läpp"), TEXT("ThinLips"), OutTags);
    AddTagIfContains(Text, TEXT("fylliga läpp"), TEXT("FullLips"), OutTags);
    AddTagIfContains(Text, TEXT("raka ögonbryn"), TEXT("StraightBrows"), OutTags);
    AddTagIfContains(Text, TEXT("stirrande blick"), TEXT("StaringEyes"), OutTags);

    AddTagIfContains(Text, TEXT("blond"), TEXT("Blond"), OutTags);
    AddTagIfContains(Text, TEXT("ljushår"), TEXT("Blond"), OutTags);
    AddTagIfContains(Text, TEXT("brunhår"), TEXT("Brown"), OutTags);
    AddTagIfContains(Text, TEXT("mörkhår"), TEXT("Dark"), OutTags);
    AddTagIfContains(Text, TEXT("svarthår"), TEXT("Black"), OutTags);
    AddTagIfContains(Text, TEXT("gråhår"), TEXT("Grey"), OutTags);
    AddTagIfContains(Text, TEXT("vithår"), TEXT("White"), OutTags);
    AddTagIfContains(Text, TEXT("rödhår"), TEXT("Red"), OutTags);
    AddTagIfContains(Text, TEXT("halvlång"), TEXT("MediumLength"), OutTags);
    AddTagIfContains(Text, TEXT("lockigt"), TEXT("Curly"), OutTags);
    AddTagIfContains(Text, TEXT("krulligt"), TEXT("Curly"), OutTags);
    AddTagIfContains(Text, TEXT("vågigt"), TEXT("Wavy"), OutTags);
    AddTagIfContains(Text, TEXT("rakt hår"), TEXT("Straight"), OutTags);
    AddTagIfContains(Text, TEXT("tunnhår"), TEXT("Receding"), OutTags);
    AddTagIfContains(Text, TEXT("hårfäste"), TEXT("Receding"), OutTags);
    AddTagIfContains(Text, TEXT("mustasch"), TEXT("Mustache"), OutTags);
    AddTagIfContains(Text, TEXT("skägg"), TEXT("Beard"), OutTags);
    AddTagIfContains(Text, TEXT("stubb"), TEXT("Stubble"), OutTags);
    AddTagIfContains(Text, TEXT("glasögon"), TEXT("Glasses"), OutTags);
    AddTagIfContains(Text, TEXT("solglasögon"), TEXT("Sunglasses"), OutTags);
    AddTagIfContains(Text, TEXT("metallbåge"), TEXT("MetalFrame"), OutTags);
    AddTagIfContains(Text, TEXT("svart båge"), TEXT("BlackFrame"), OutTags);
    AddTagIfContains(Text, TEXT("halsduk"), TEXT("Scarf"), OutTags);
    AddTagIfContains(Text, TEXT("sjal"), TEXT("Scarf"), OutTags);
}

FName EnumLeafTag(const FString& QualifiedName)
{
    FString Prefix;
    FString Leaf;
    return QualifiedName.Split(TEXT("::"), &Prefix, &Leaf,
        ESearchCase::CaseSensitive, ESearchDir::FromEnd)
        ? FName(*Leaf) : FName(*QualifiedName);
}

float ConfidenceObscurity(const ETMOPHistoricalConfidence Confidence)
{
    switch (Confidence)
    {
    case ETMOPHistoricalConfidence::Reconstructed: return 0.15f;
    case ETMOPHistoricalConfidence::Inferred: return 0.30f;
    case ETMOPHistoricalConfidence::Speculative: return 0.55f;
    default: return 0.0f;
    }
}

bool IsCompatible(const FTMOPAppearanceAssetRow& Asset,
    const FTMOPPersonProfileRow& Profile, const ETMOPBodyBuild Build)
{
    if (Asset.Gender != ETMOPPersonGender::Unknown &&
        Profile.Gender != ETMOPPersonGender::Unknown &&
        Asset.Gender != Profile.Gender) return false;
    if (!Asset.CompatibleBodyBuilds.IsEmpty() &&
        !Asset.CompatibleBodyBuilds.Contains(Build)) return false;
    if (Profile.AgeAtEvent > 0 &&
        (Profile.AgeAtEvent < Asset.MinimumAge ||
         (Asset.MaximumAge > 0 && Profile.AgeAtEvent > Asset.MaximumAge))) return false;
    return Asset.EarliestYear <= 1986 && Asset.LatestYear >= 1986;
}

void AddEvidenceTags(const FTMOPAppearanceSlot& Slot, TSet<FName>& OutTags)
{
    if (!Slot.NormalizedValue.IsNone()) OutTags.Add(Slot.NormalizedValue);
    for (const FName Tag : Slot.Tags) if (!Tag.IsNone()) OutTags.Add(Tag);
    AddFreeTextTags(Slot.OriginalText, OutTags);
}

FTMOPResolvedFaceMorphs GenerateFaceMorphs(
    const FTMOPPersonProfileRow& Profile, const int32 Seed)
{
    FTMOPResolvedFaceMorphs Result;
    if (Profile.AppearanceProfile.GenerationMode ==
        ETMOPAppearanceGenerationMode::MetaHuman)
        return Result;
    const bool bHasEvidence =
        UTMOPAppearanceResolver::IsAppearanceSlotKnown(Profile.FaceShape) ||
        UTMOPAppearanceResolver::IsAppearanceSlotKnown(Profile.Nose) ||
        !Profile.AppearanceProfile.Face.CatalogId.IsNone() ||
        !Profile.AppearanceProfile.Face.MeshOverride.IsNull();
    const bool bGenerateNeutral = Profile.AppearanceProfile.UnknownPartStyle ==
        ETMOPUnknownAppearanceStyle::Neutral;
    if (!bHasEvidence && !bGenerateNeutral) return Result;

    FRandomStream FaceRandom(Seed ^ 0x4f1bbcdc);
    auto Variation = [&FaceRandom]()
    {
        return FaceRandom.FRandRange(-0.18f, 0.18f);
    };
    Result.FaceWidth = Variation();
    Result.JawWidth = Variation();
    Result.JawProjection = Variation();
    Result.CheekboneProminence = Variation();
    Result.NoseWidth = Variation();
    Result.NoseLength = Variation();
    Result.BrowHeight = Variation();
    Result.EyeSpacing = Variation();
    Result.LipThickness = Variation();
    Result.Age = Profile.AgeAtEvent > 0
        ? FMath::Clamp((Profile.AgeAtEvent - 20.0f) / 60.0f, 0.0f, 1.0f)
        : 0.30f;

    TSet<FName> Tags;
    AddEvidenceTags(Profile.FaceShape, Tags);
    AddEvidenceTags(Profile.Nose, Tags);
    if (Tags.Contains(TEXT("RectangularFace")))
    { Result.FaceWidth = 0.18f; Result.JawWidth = 0.25f; }
    if (Tags.Contains(TEXT("SquareFace")))
    { Result.FaceWidth = 0.28f; Result.JawWidth = 0.42f; }
    if (Tags.Contains(TEXT("RoundFace")))
    { Result.FaceWidth = 0.38f; Result.JawWidth = 0.08f; }
    if (Tags.Contains(TEXT("OvalFace")))
    { Result.FaceWidth = -0.08f; Result.JawWidth = -0.12f; }
    if (Tags.Contains(TEXT("NarrowFace"))) Result.FaceWidth = -0.38f;
    if (Tags.Contains(TEXT("WideFace"))) Result.FaceWidth = 0.38f;
    if (Tags.Contains(TEXT("StrongJaw"))) Result.JawWidth = 0.48f;
    if (Tags.Contains(TEXT("ProjectedJaw"))) Result.JawProjection = 0.48f;
    if (Tags.Contains(TEXT("MarkedCheekbones")))
        Result.CheekboneProminence = 0.50f;
    if (Tags.Contains(TEXT("LargeNose")))
    { Result.NoseWidth = 0.38f; Result.NoseLength = 0.28f; }
    if (Tags.Contains(TEXT("LongNose"))) Result.NoseLength = 0.42f;
    if (Tags.Contains(TEXT("NarrowNose"))) Result.NoseWidth = -0.35f;
    if (Tags.Contains(TEXT("WideNose"))) Result.NoseWidth = 0.40f;
    if (Tags.Contains(TEXT("SmallNose")))
    { Result.NoseWidth = -0.30f; Result.NoseLength = -0.22f; }
    if (Tags.Contains(TEXT("ThinLips"))) Result.LipThickness = -0.48f;
    if (Tags.Contains(TEXT("FullLips"))) Result.LipThickness = 0.45f;
    return Result;
}

void CopyAsset(const FTMOPAppearanceAssetRow& Asset,
    FTMOPResolvedAppearancePart& Out)
{
    Out.CatalogId = Asset.CatalogId;
    Out.Mesh = Asset.Mesh;
    Out.StaticMesh = Asset.StaticMesh;
    Out.AttachmentSocket = Asset.AttachmentSocket;
    Out.AttachmentTransform = Asset.AttachmentTransform;
    Out.Material = Asset.Material;
    Out.PrimaryColor = Asset.DefaultPrimaryColor;
    Out.SecondaryColor = Asset.DefaultSecondaryColor;
    Out.bUsesObscuredFallback = Asset.bObscuredFallback;
    Out.HiddenBodyRegions = Asset.HiddenBodyRegions;
}

const FTMOPAppearanceAssetRow* PickWeighted(
    const TArray<const FTMOPAppearanceAssetRow*>& Candidates,
    FRandomStream& Random)
{
    float Total = 0.0f;
    for (const FTMOPAppearanceAssetRow* Candidate : Candidates)
        if (Candidate != nullptr) Total += FMath::Max(0.01f, Candidate->SelectionWeight);
    float Choice = Random.FRandRange(0.0f, FMath::Max(0.01f, Total));
    for (const FTMOPAppearanceAssetRow* Candidate : Candidates)
        if (Candidate != nullptr)
        {
            Choice -= FMath::Max(0.01f, Candidate->SelectionWeight);
            if (Choice <= 0.0f) return Candidate;
        }
    return Candidates.IsEmpty() ? nullptr : Candidates.Last();
}

FName BodyBuildTag(const ETMOPBodyBuild Build)
{
    switch (Build)
    {
    case ETMOPBodyBuild::Thin: return TEXT("Thin");
    case ETMOPBodyBuild::Slim: return TEXT("Slim");
    case ETMOPBodyBuild::Average: return TEXT("Average");
    case ETMOPBodyBuild::Athletic: return TEXT("Athletic");
    case ETMOPBodyBuild::Strong: return TEXT("Strong");
    case ETMOPBodyBuild::Heavy: return TEXT("Heavy");
    default: return NAME_None;
    }
}
}

bool UTMOPAppearanceResolver::IsAppearanceSlotKnown(
    const FTMOPAppearanceSlot& Slot)
{
    return !Slot.OriginalText.TrimStartAndEnd().IsEmpty() ||
        !Slot.NormalizedValue.IsNone() || !Slot.Tags.IsEmpty();
}

TArray<FName> UTMOPAppearanceResolver::GetNormalizedEvidenceTags(
    const FTMOPAppearanceSlot& Slot)
{
    TSet<FName> UniqueTags;
    AddEvidenceTags(Slot, UniqueTags);
    TArray<FName> Result = UniqueTags.Array();
    Result.Sort(FNameLexicalLess());
    return Result;
}

FTMOPResolvedAppearancePart UTMOPAppearanceResolver::ResolvePart(
    const FTMOPPersonProfileRow& Profile, UDataTable* AssetCatalog,
    const ETMOPAppearancePartType PartType,
    const FTMOPAppearancePartChoice& Override,
    const TArray<FTMOPAppearanceSlot>& Evidence,
    const FName UnknownCatalogId, const bool bKnownAbsent,
    FRandomStream& Random, TArray<FString>& Diagnostics)
{
    FTMOPResolvedAppearancePart Result;
    Result.PartType = PartType;
    if (bKnownAbsent)
    {
        Result.bIntentionallyEmpty = true;
        return Result;
    }

    bool bKnown = false;
    float EvidenceObscurity = 0.0f;
    TSet<FName> EvidenceTags;
    for (const FTMOPAppearanceSlot& Slot : Evidence)
    {
        bKnown |= IsAppearanceSlotKnown(Slot);
        if (IsAppearanceSlotKnown(Slot))
            EvidenceObscurity = FMath::Max(
                EvidenceObscurity, ConfidenceObscurity(Slot.Confidence));
        AddEvidenceTags(Slot, EvidenceTags);
    }
    Result.bSourceWasUnknown = !bKnown;
    Result.ObscurityAmount = bKnown ? EvidenceObscurity : 1.0f;
    if (!bKnown && PartType != ETMOPAppearancePartType::Body &&
        Profile.AppearanceProfile.UnknownPartStyle ==
            ETMOPUnknownAppearanceStyle::Hidden)
    {
        Result.bIntentionallyEmpty = true;
        Result.ObscurityAmount = 0.0f;
        return Result;
    }

    if (!Override.CatalogId.IsNone() || !Override.MeshOverride.IsNull() ||
        !Override.StaticMeshOverride.IsNull())
    {
        Result.CatalogId = Override.CatalogId;
        Result.Mesh = Override.MeshOverride;
        Result.StaticMesh = Override.StaticMeshOverride;
        Result.AttachmentSocket = Override.AttachmentSocket;
        Result.AttachmentTransform = Override.AttachmentTransform;
        Result.Material = Override.MaterialOverride;
        Result.PrimaryColor = Override.PrimaryColor;
        Result.SecondaryColor = Override.SecondaryColor;
    }

    const bool bNeedsCatalogLookup = Result.Mesh.IsNull() &&
        Result.StaticMesh.IsNull();
    if (IsValid(AssetCatalog) &&
        AssetCatalog->GetRowStruct() == FTMOPAppearanceAssetRow::StaticStruct() &&
        bNeedsCatalogLookup)
    {
        if (!Result.CatalogId.IsNone())
            if (const FTMOPAppearanceAssetRow* Exact =
                AssetCatalog->FindRow<FTMOPAppearanceAssetRow>(
                    Result.CatalogId, TEXT("TMOP exact appearance lookup"), false))
                if (Exact->PartType == PartType)
                    CopyAsset(*Exact, Result);

        if (Result.CatalogId.IsNone() && bKnown)
        {
            int32 BestScore = INDEX_NONE;
            TArray<const FTMOPAppearanceAssetRow*> Best;
            for (const FName RowName : AssetCatalog->GetRowNames())
            {
                const FTMOPAppearanceAssetRow* Asset =
                    AssetCatalog->FindRow<FTMOPAppearanceAssetRow>(
                        RowName, TEXT("TMOP appearance matching"), false);
                if (Asset == nullptr || Asset->PartType != PartType ||
                    Asset->bObscuredFallback ||
                    !IsCompatible(*Asset, Profile, Profile.GetResolvedBodyBuild())) continue;
                int32 Score = 0;
                for (const FName Tag : Asset->Tags)
                    if (EvidenceTags.Contains(Tag)) ++Score;
                if (Score > BestScore) { BestScore = Score; Best.Reset(); Best.Add(Asset); }
                else if (Score == BestScore) Best.Add(Asset);
            }
            if (!Best.IsEmpty() && BestScore > 0)
                if (const FTMOPAppearanceAssetRow* Picked = PickWeighted(Best, Random))
                    CopyAsset(*Picked, Result);
        }
    }

    if (Result.CatalogId.IsNone() && Result.Mesh.IsNull() &&
        Result.StaticMesh.IsNull())
    {
        Result.CatalogId = UnknownCatalogId;
        Result.bUsesObscuredFallback = true;
        if (IsValid(AssetCatalog) && !UnknownCatalogId.IsNone())
            if (const FTMOPAppearanceAssetRow* Unknown =
                AssetCatalog->FindRow<FTMOPAppearanceAssetRow>(
                    UnknownCatalogId, TEXT("TMOP unknown appearance lookup"), false))
                CopyAsset(*Unknown, Result);
        Diagnostics.Add(FString::Printf(TEXT("%s uses fallback '%s'."),
            *UEnum::GetValueAsString(PartType), *UnknownCatalogId.ToString()));
    }
    if (Result.bUsesObscuredFallback)
        Result.ObscurityAmount = Result.bSourceWasUnknown
            ? 1.0f : FMath::Max(Result.ObscurityAmount, 0.65f);
    if (Profile.AppearanceProfile.UnknownPartStyle ==
        ETMOPUnknownAppearanceStyle::Neutral && Result.bSourceWasUnknown)
    {
        Result.bUsesObscuredFallback = false;
        Result.ObscurityAmount = 0.0f;
    }
    return Result;
}

bool UTMOPAppearanceResolver::ResolveAppearance(
    const FTMOPPersonProfileRow& Profile, UDataTable* AssetCatalog,
    FTMOPResolvedAppearance& OutAppearance)
{
    OutAppearance = FTMOPResolvedAppearance();
    OutAppearance.HeightCentimeters = Profile.GetResolvedHeightCentimeters();
    OutAppearance.Gender = Profile.Gender;
    OutAppearance.BodyBuild = Profile.GetResolvedBodyBuild();
    OutAppearance.ResolvedSeed = Profile.AppearanceProfile.AppearanceSeed > 0
        ? Profile.AppearanceProfile.AppearanceSeed
        : FMath::Max(1, static_cast<int32>(GetTypeHash(Profile.EntityId) & 0x7fffffff));
    FRandomStream Random(OutAppearance.ResolvedSeed);
    const FTMOPAppearanceProfile& A = Profile.AppearanceProfile;
    FTMOPAppearanceSlot BodyEvidence = Profile.BodyBuild;
    if (BodyEvidence.NormalizedValue.IsNone())
        BodyEvidence.NormalizedValue = BodyBuildTag(Profile.GetResolvedBodyBuild());
    FTMOPAppearanceSlot HairEvidence = Profile.Hair;
    if (HairEvidence.NormalizedValue.IsNone() &&
        Profile.HairColorCategory != ETMOPHairColor::Unknown)
        HairEvidence.NormalizedValue = EnumLeafTag(
            UEnum::GetValueAsString(Profile.HairColorCategory));
    FTMOPAppearanceSlot FacialHairEvidence = Profile.BeardOrMustache;
    if (FacialHairEvidence.NormalizedValue.IsNone() &&
        Profile.FacialHairCategory != ETMOPFacialHairType::Unknown)
        FacialHairEvidence.NormalizedValue = EnumLeafTag(
            UEnum::GetValueAsString(Profile.FacialHairCategory));
    FTMOPAppearanceSlot OuterwearEvidence = Profile.JacketOrCoat;
    if (OuterwearEvidence.NormalizedValue.IsNone() &&
        Profile.OuterwearCategory != ETMOPOuterwearType::Unknown)
        OuterwearEvidence.NormalizedValue = EnumLeafTag(
            UEnum::GetValueAsString(Profile.OuterwearCategory));
    FTMOPAppearanceSlot HeadwearEvidence = Profile.Headwear;
    if (HeadwearEvidence.NormalizedValue.IsNone() &&
        Profile.HeadwearCategory != ETMOPHeadwearType::Unknown)
        HeadwearEvidence.NormalizedValue = EnumLeafTag(
            UEnum::GetValueAsString(Profile.HeadwearCategory));
    FTMOPAppearanceSlot GlovesEvidence;
    const FString OtherText = Profile.OtherCharacteristics.OriginalText;
    if (OtherText.Contains(TEXT("handsk"), ESearchCase::IgnoreCase) ||
        OtherText.Contains(TEXT("vante"), ESearchCase::IgnoreCase))
        GlovesEvidence = Profile.OtherCharacteristics;

    // Gender-specific fallback meshes must be configured in the asset catalog.
    // Keep the neutral ID for profiles whose gender is unspecified.
    FName BodyFallbackId(TEXT("UNKNOWN_BODY_STANDARD"));
    if (Profile.Gender == ETMOPPersonGender::Male)
        BodyFallbackId = TEXT("UNKNOWN_BODY_STANDARD_male");
    else if (Profile.Gender == ETMOPPersonGender::Female)
        BodyFallbackId = TEXT("UNKNOWN_BODY_STANDARD_female");
    OutAppearance.Body = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Body, A.Body,
        { BodyEvidence }, BodyFallbackId, false,
        Random, OutAppearance.Diagnostics);
    // Missing build evidence means a normal average body, not an anonymised body.
    OutAppearance.Body.bUsesObscuredFallback = false;
    OutAppearance.Face = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Face, A.Face,
        { Profile.FaceShape, Profile.Nose }, A.UnknownFaceCatalogId, false,
        Random, OutAppearance.Diagnostics);
    OutAppearance.Hair = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Hair, A.Hair, { HairEvidence },
        TEXT("UNKNOWN_HAIR_OBSCURED"), Profile.HairColorCategory == ETMOPHairColor::Bald,
        Random, OutAppearance.Diagnostics);
    const bool bNoFacialHair =
        Profile.FacialHairCategory == ETMOPFacialHairType::None ||
        (Profile.FacialHairCategory == ETMOPFacialHairType::Unknown &&
         !IsAppearanceSlotKnown(FacialHairEvidence));
    OutAppearance.FacialHair = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::FacialHair, A.FacialHair,
        { FacialHairEvidence }, NAME_None, bNoFacialHair,
        Random, OutAppearance.Diagnostics);
    OutAppearance.Outerwear = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Outerwear, A.Outerwear, { OuterwearEvidence },
        A.UnknownOuterwearCatalogId, Profile.OuterwearCategory == ETMOPOuterwearType::None,
        Random, OutAppearance.Diagnostics);
    OutAppearance.UpperBody = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::UpperBody, A.UpperBody, { Profile.ShirtOrSweater },
        A.UnknownUpperBodyCatalogId, false, Random, OutAppearance.Diagnostics);
    OutAppearance.Trousers = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Trousers, A.Trousers, { Profile.Trousers },
        A.UnknownTrousersCatalogId, false, Random, OutAppearance.Diagnostics);
    OutAppearance.Footwear = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Footwear, A.Footwear, { Profile.Shoes },
        A.UnknownFootwearCatalogId, false, Random, OutAppearance.Diagnostics);
    OutAppearance.Gloves = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Gloves, A.Gloves, { GlovesEvidence },
        TEXT("UNKNOWN_GLOVES_OBSCURED"), false, Random, OutAppearance.Diagnostics);
    OutAppearance.Headwear = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Headwear, A.Headwear, { HeadwearEvidence },
        TEXT("UNKNOWN_HEADWEAR_OBSCURED"), Profile.HeadwearCategory == ETMOPHeadwearType::None,
        Random, OutAppearance.Diagnostics);
    OutAppearance.Scarf = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Scarf, A.Scarf, { Profile.Scarf },
        NAME_None, !IsAppearanceSlotKnown(Profile.Scarf),
        Random, OutAppearance.Diagnostics);
    OutAppearance.Glasses = ResolvePart(Profile, AssetCatalog,
        ETMOPAppearancePartType::Glasses, A.Glasses, { Profile.Glasses },
        NAME_None, !IsAppearanceSlotKnown(Profile.Glasses),
        Random, OutAppearance.Diagnostics);
    OutAppearance.FaceMorphs = GenerateFaceMorphs(
        Profile, OutAppearance.ResolvedSeed);
    if (A.GenerationMode == ETMOPAppearanceGenerationMode::MetaHuman &&
        !A.bUseMetaHumanHybridHead)
    {
        OutAppearance.bUsesBespokeMetaHuman = true;
        auto PreserveBespokePart = [](FTMOPResolvedAppearancePart& Part,
            const ETMOPAppearancePartType Type)
        {
            Part = FTMOPResolvedAppearancePart();
            Part.PartType = Type;
            Part.CatalogId = TEXT("METAHUMAN_BESPOKE");
            Part.bIntentionallyEmpty = true;
        };
        PreserveBespokePart(OutAppearance.Body, ETMOPAppearancePartType::Body);
        PreserveBespokePart(OutAppearance.Face, ETMOPAppearancePartType::Face);
        PreserveBespokePart(OutAppearance.Hair, ETMOPAppearancePartType::Hair);
        PreserveBespokePart(OutAppearance.FacialHair,
            ETMOPAppearancePartType::FacialHair);
    }
    return true;
}
