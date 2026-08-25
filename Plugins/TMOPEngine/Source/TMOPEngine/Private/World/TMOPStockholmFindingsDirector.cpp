#include "World/TMOPStockholmFindingsDirector.h"

namespace
{
FName FindingMeshCatalogId(const FName EvidenceId)
{
    if (EvidenceId == TEXT("FYND_BLA_TACKJACKA_ANORALP")) return TEXT("FINDING_BLUE_PADDED_JACKET");
    if (EvidenceId == TEXT("FYND_STALBAGADE_GLASOGON_OMRADE") ||
        EvidenceId == TEXT("FYND_STALBAGADE_GLASOGON_SNICKARBACKEN5"))
        return TEXT("FINDING_STEEL_FRAME_GLASSES");
    if (EvidenceId == TEXT("FYND_LJUSBRUNA_LANGBYXOR")) return TEXT("FINDING_LIGHT_BROWN_TROUSERS");
    if (EvidenceId == TEXT("FYND_BLA_TYGMOSSA")) return TEXT("FINDING_BLUE_FABRIC_CAP");
    if (EvidenceId == TEXT("FYND_ROD_DAMHOGERSKO")) return TEXT("FINDING_RED_WOMENS_RIGHT_SHOE");
    if (EvidenceId == TEXT("FYND_TELEFONBLOCKSLAPP")) return TEXT("FINDING_HANDWRITTEN_PHONE_NOTE");
    if (EvidenceId == TEXT("FYND_AVBRUTEN_GREN")) return TEXT("FINDING_BROKEN_BRANCH");
    if (EvidenceId == TEXT("FYND_HALSDUK_MORDPLATS")) return TEXT("FINDING_SCARF");
    if (EvidenceId == TEXT("FYND_KLADER_HANDELSHOGSKOLAN")) return TEXT("FINDING_CLOTHES_PILE");
    if (EvidenceId == TEXT("FYND_HANDSKE_KYRKAN") ||
        EvidenceId == TEXT("FYND_HANDSKE_MORDPLATS"))
        return TEXT("FINDING_GREY_GREEN_GLOVE");
    if (EvidenceId == TEXT("FYND_KULA_1") || EvidenceId == TEXT("FYND_KULA_2"))
        return TEXT("FINDING_BULLET_357");
    return EvidenceId;
}
}

ATMOPStockholmFindingsDirector::ATMOPStockholmFindingsDirector()
{
    ScheduledEntries.Reset();

    auto AddFinding = [this](
        const FName Id,
        const TCHAR* DisplayName,
        const FTMOPTime& SpawnTime,
        const ETMOPEventTimingMode TimingMode,
        const FName SharedEventId,
        const int32 OffsetSeconds,
        const FVector& WorldLocation,
        const FVector& Scale,
        const FLinearColor& Color,
        const double Latitude,
        const double Longitude,
        const TCHAR* SourceTime,
        const TCHAR* Source)
    {
        FTMOPTimedPropEntry& Entry = ScheduledEntries.AddDefaulted_GetRef();
        Entry.EntryId = Id;
        Entry.InstanceId = Id;
        Entry.Action = ETMOPTimedPropAction::Spawn;
        Entry.PropKind = ETMOPTimedPropKind::Finding;
        Entry.ItemMeshId = FindingMeshCatalogId(Id);
        Entry.Placement = ETMOPTimedPropPlacement::WorldTransform;
        Entry.WorldTransform = FTransform(
            FRotator::ZeroRotator, WorldLocation, FVector::OneVector);
        Entry.Time = SpawnTime;
        Entry.TimingMode = TimingMode;
        Entry.SharedEventId = SharedEventId;
        Entry.OffsetSeconds = OffsetSeconds;
        Entry.FindingDisplayName = FText::FromString(FString(DisplayName));
        Entry.EvidenceId = Id.ToString();
        Entry.SourceTimeLabel = SourceTime;
        Entry.SourceReference = Source;
        Entry.SourceLatitude = Latitude;
        Entry.SourceLongitude = Longitude;
        Entry.bLocationApproximate = true;
        Entry.FindingScale = Scale;
        Entry.FindingColor = Color;
        Entry.bSnapToGround = true;
        Entry.GroundOffsetCm = 3.0f;
    };

    AddFinding(
        TEXT("FYND_BLA_TACKJACKA_ANORALP"),
        TEXT("Blå täckjacka ANORALP"),
        FTMOPTime(22, 30, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-1483.87f, -25455.968f, 0.0f),
        FVector(0.55f, 0.18f, 0.7f),
        FLinearColor(0.08f, 0.22f, 0.65f, 1.0f),
        59.337934593648,
        18.059338141374,
        TEXT("KMZ observation window; item location/finding date 1986-03-03"),
        TEXT("F-9768; Adolf Fredriks kyrkogård"));
    AddFinding(
        TEXT("FYND_STALBAGADE_GLASOGON_OMRADE"),
        TEXT("Stålbågade glasögon – områdesmarkör"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-48086.055f, 156913.666f, 0.0f),
        FVector(0.16f, 0.05f, 0.05f),
        FLinearColor(0.55f, 0.55f, 0.58f, 1.0f),
        59.333638900000,
        18.091169400000,
        TEXT("placement time unknown; present from scenario start"),
        TEXT("F-1203-A; KMZ coordinate marked as street/area"));
    AddFinding(
        TEXT("FYND_LJUSBRUNA_LANGBYXOR"),
        TEXT("Ljusbruna långbyxor"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-5512.61f, 3878.508f, 0.0f),
        FVector(0.55f, 0.18f, 0.05f),
        FLinearColor(0.48f, 0.31f, 0.16f, 1.0f),
        59.336973479131,
        18.064163016844,
        TEXT("placement time unknown; present from scenario start"),
        TEXT("Fyndlista 7; Tunnelgatan below stairs"));
    AddFinding(
        TEXT("FYND_BLA_TYGMOSSA"),
        TEXT("Blå tygmössa med skärm"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(8038.38f, 8707.831f, 0.0f),
        FVector(0.18f, 0.18f, 0.12f),
        FLinearColor(0.06f, 0.16f, 0.55f, 1.0f),
        59.335707791151,
        18.063745889469,
        TEXT("placement time unknown; present from scenario start"),
        TEXT("Fyndlista 7; Sveavägen–Kungsgatan"));
    AddFinding(
        TEXT("FYND_ROD_DAMHOGERSKO"),
        TEXT("Röd damhögersko"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-10673.677f, 9274.776f, 0.0f),
        FVector(0.28f, 0.1f, 0.1f),
        FLinearColor(0.65f, 0.03f, 0.04f, 1.0f),
        59.337141043415,
        18.065427275326,
        TEXT("placement time unknown; present from scenario start"),
        TEXT("Fyndlista 7; outside Brunkeberg tunnel"));
    AddFinding(
        TEXT("FYND_TELEFONBLOCKSLAPP"),
        TEXT("Handskriven telefonblockslapp"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(5378.224f, 3721.287f, 0.0f),
        FVector(0.16f, 0.11f, 0.01f),
        FLinearColor(0.9f, 0.84f, 0.62f, 1.0f),
        59.336131785817,
        18.063210813536,
        TEXT("placement time unknown; present from scenario start"),
        TEXT("Fyndlista 7; east pavement north of murder site"));
    AddFinding(
        TEXT("FYND_AVBRUTEN_GREN"),
        TEXT("Avbruten gren"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(9599.625f, 5481.564f, 0.0f),
        FVector(0.5f, 0.035f, 0.035f),
        FLinearColor(0.25f, 0.12f, 0.04f, 1.0f),
        59.335726378692,
        18.063119961076,
        TEXT("placement time unknown; present from scenario start"),
        TEXT("Fyndlista 7; pavement by metro entrance"));
    AddFinding(
        TEXT("FYND_HALSDUK_MORDPLATS"),
        TEXT("Halsduk vid mordplatsen"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(2117.897f, -487.383f, 0.0f),
        FVector(0.55f, 0.12f, 0.025f),
        FLinearColor(0.25f, 0.28f, 0.32f, 1.0f),
        59.336568731206,
        18.062845716900,
        TEXT("reported found/observed 09:00 next day; present from scenario start"),
        TEXT("KMZ masked witness marker; source description conflicts with marker name"));
    AddFinding(
        TEXT("FYND_KLADER_HANDELSHOGSKOLAN"),
        TEXT("Kläder i blå sandlåda vid Handelshögskolan"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-28529.932f, -62417.338f, 0.0f),
        FVector(0.6f, 0.35f, 0.18f),
        FLinearColor(0.18f, 0.24f, 0.38f, 1.0f),
        59.341648306192,
        18.055996095436,
        TEXT("found daytime 1986-03-01; placement time unknown"),
        TEXT("Ewa Felicetti account; approximate location"));
    AddFinding(
        TEXT("FYND_STALBAGADE_GLASOGON_SNICKARBACKEN5"),
        TEXT("Stålbågade glasögon vid Snickarbacken 5"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-26273.572f, 17644.899f, 0.0f),
        FVector(0.16f, 0.05f, 0.05f),
        FLinearColor(0.55f, 0.55f, 0.58f, 1.0f),
        59.337992661449,
        18.068035576687,
        TEXT("found 1986-02-27, exact time unknown; present from scenario start"),
        TEXT("F-1203-A / E779; submitted 1986-03-06 14:50"));
    AddFinding(
        TEXT("FYND_HANDSKE_KYRKAN"),
        TEXT("Grå/grön handske vid kyrkan"),
        FTMOPTime(23, 21, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(-20644.71f, -3454.788f, 0.0f),
        FVector(0.24f, 0.12f, 0.035f),
        FLinearColor(0.3f, 0.38f, 0.25f, 1.0f),
        59.338471188056,
        18.064332375584,
        TEXT("KMZ/description states approximately 23:21"),
        TEXT("EAF-9717; masked witness"));
    AddFinding(
        TEXT("FYND_HANDSKE_MORDPLATS"),
        TEXT("Grå/grön handske intill blodet"),
        FTMOPTime(23, 21, 0),
        ETMOPEventTimingMode::Absolute,
        NAME_None,
        0,
        FVector(1750.501f, -836.729f, 0.0f),
        FVector(0.24f, 0.12f, 0.035f),
        FLinearColor(0.3f, 0.38f, 0.25f, 1.0f),
        59.336612539897,
        18.062823658492,
        TEXT("KMZ/description states approximately 23:21"),
        TEXT("EAF-9717; masked witness"));
    AddFinding(
        TEXT("FYND_KULA_1"),
        TEXT("Kula – hittad 1986-03-01 06:30"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Relative,
        TEXT("PALME_SHOT_1"),
        0,
        FVector(5405.533f, 4586.046f, 0.0f),
        FVector(0.022f, 0.022f, 0.022f),
        FLinearColor(0.72f, 0.55f, 0.18f, 1.0f),
        59.336092071939,
        18.063340594814,
        TEXT("spawn tied to shooting event; discovery time retained in label"),
        TEXT("Fyndlista 7; exact position should follow original sketch"));
    AddFinding(
        TEXT("FYND_KULA_2"),
        TEXT("Kula – hittad 1986-03-02 12:20"),
        FTMOPTime(23, 0, 0),
        ETMOPEventTimingMode::Relative,
        TEXT("PALME_SHOT_1"),
        1,
        FVector(5992.826f, 4744.726f, 0.0f),
        FVector(0.022f, 0.022f, 0.022f),
        FLinearColor(0.72f, 0.55f, 0.18f, 1.0f),
        59.336039417872,
        18.063314784097,
        TEXT("spawn tied to shooting event +1 second; discovery time retained in label"),
        TEXT("Fyndlista 7; exact position should follow original sketch"));
}
