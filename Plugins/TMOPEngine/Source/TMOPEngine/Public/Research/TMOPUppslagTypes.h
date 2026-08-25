#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TMOPUppslagTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPUppslagAvailability : uint8
{
    Unknown UMETA(DisplayName="Okänt"),
    Available UMETA(DisplayName="Utlämnat / tillgängligt"),
    NotReleased UMETA(DisplayName="Inte utlämnat / endast hos polisen"),
    PartiallyAvailable UMETA(DisplayName="Delvis tillgängligt"),
    Partial UMETA(DisplayName="Delvis dokument"),
    MissingPage UMETA(DisplayName="Sida saknas"),
    AvailableMasked UMETA(DisplayName="Tillgängligt men maskerat"),
    UnavailableNoDocument UMETA(DisplayName="Inget dokument tillgängligt")
};

UENUM(BlueprintType)
enum class ETMOPSourceCategory : uint8
{
    PoliceUppslag UMETA(DisplayName="1. Polisuppslag"),
    Book UMETA(DisplayName="2. Böcker"),
    Article UMETA(DisplayName="3. Artiklar"),
    Other UMETA(DisplayName="4. Andra uppgifter")
};

UENUM(BlueprintType)
enum class ETMOPSourceReliability : uint8
{
    Unverified UMETA(DisplayName="Obekräftad"),
    PrimarySource UMETA(DisplayName="Primärkälla"),
    SecondarySource UMETA(DisplayName="Sekundärkälla"),
    Corroborated UMETA(DisplayName="Bekräftad av flera källor"),
    Disputed UMETA(DisplayName="Motsagd / omtvistad")
};

/**
 * One WPU investigation file/reference and its implementation state.
 *
 * The row name should normally be the same as UppslagId, for example
 * L849-00 or A14205-02-A.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPUppslagRow : public FTableRowBase
{
    GENERATED_BODY()

    /** The four top-level groups shown in Källor/Uppslag. Old rows default to police records. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="Källkategori"))
    ETMOPSourceCategory SourceCategory = ETMOPSourceCategory::PoliceUppslag;

    /** Canonical WPU reference without the "Uppslag:" prefix. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Identity",
        meta=(DisplayName="Uppslag ID"))
    FName UppslagId = NAME_None;

    /** Short human-readable description of the document. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Identity")
    FText Title;

    /** Register series, for example L or LA. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Identity",
        meta=(DisplayName="Serie"))
    FName SeriesId = NAME_None;

    /** Metadata row for a WPU letter section; excluded from uppslag totals. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Section",
        meta=(DisplayName="Avsnittsdefinition"))
    bool bIsSectionDefinition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Section",
        meta=(MultiLine="true", DisplayName="Avsnittsbeskrivning"))
    FString SectionDescription;

    /** Parent register entry, for example L261 for L261-00-A. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Identity",
        meta=(DisplayName="Huvuduppslag"))
    FName ParentUppslagId = NAME_None;

    /** Whether the actual document has been released and is available. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Checklist",
        meta=(DisplayName="Tillgänglighet"))
    ETMOPUppslagAvailability Availability =
        ETMOPUppslagAvailability::Unknown;

    /** The source document has been obtained and is available for review. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Checklist",
        meta=(DisplayName="Uthämtat av oss"))
    bool bRetrieved = false;

    /**
     * Main checklist field. Check this only when every relevant fact from the
     * document has been transferred into the project.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Checklist",
        meta=(DisplayName="Inlagt i projektet"))
    bool bAddedToProject = false;

    /** Some information is present, but work from this document remains. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Checklist",
        meta=(DisplayName="Delvis inlagt"))
    bool bPartiallyAdded = false;

    /** The current interpretation or implementation should be checked again. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Checklist",
        meta=(DisplayName="Behöver granskas"))
    bool bNeedsReview = false;

    /** Uncheck for documents that do not affect the playable area or scenario. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Checklist",
        meta=(DisplayName="Relevant för spelet"))
    bool bRelevantToGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="WPU URL"))
    FString SourceUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="Dokumentdatum"))
    FString DocumentDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="Författare / uppgiftslämnare"))
    FString AuthorOrCreator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="Publikation / plattform"))
    FString PublicationOrPlatform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="ISBN / arkiv-ID"))
    FString ISBNOrArchiveId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="Sida / plats i källan"))
    FString PageOrLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(DisplayName="Källbedömning"))
    ETMOPSourceReliability Reliability = ETMOPSourceReliability::Unverified;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Source",
        meta=(MultiLine="true", DisplayName="Källhänvisning / citatbeskrivning"))
    FString CitationText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Links",
        meta=(DisplayName="Person-ID"))
    TArray<FName> PersonEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Links",
        meta=(DisplayName="Fordons-ID"))
    TArray<FName> VehicleEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Links",
        meta=(DisplayName="Grupp-ID"))
    TArray<FName> GroupIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Links",
        meta=(DisplayName="Shared Event-ID"))
    TArray<FName> SharedEventIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Links",
        meta=(DisplayName="Observation-ID"))
    TArray<FName> ObservationIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Links",
        meta=(DisplayName="Anchor-ID"))
    TArray<FName> AnchorIds;

    /** What from the document has already been represented in the project. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Work",
        meta=(MultiLine="true", DisplayName="Inlagt innehåll"))
    FString ImplementedSummary;

    /** Concrete remaining work before Inlagt i projektet may be checked. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Work",
        meta=(MultiLine="true", DisplayName="Återstår"))
    FString RemainingWork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Uppslag|Work",
        meta=(MultiLine="true"))
    FString Notes;
};
