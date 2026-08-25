#include "UI/TMOPMapComponent.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "EngineUtils.h"

namespace
{
bool ContainsAny(const FString& Value, const TArray<FString>& Tokens)
{
    for (const FString& Token : Tokens)
        if (Value.Contains(Token, ESearchCase::IgnoreCase)) return true;
    return false;
}

bool ClassifyVenue(const FString& AnchorId, ETMOPMapMarkerCategory& OutCategory)
{
    if (AnchorId.StartsWith(TEXT("Metro"), ESearchCase::IgnoreCase))
        OutCategory = ETMOPMapMarkerCategory::Metro;
    else if (ContainsAny(AnchorId, {TEXT("Biograf"), TEXT("Cinema"),
        TEXT("Rigoletto"), TEXT("Saga"), TEXT("Zita")}))
        OutCategory = ETMOPMapMarkerCategory::Cinema;
    else if (ContainsAny(AnchorId, {TEXT("Nattklubb"), TEXT("Klubb"),
        TEXT("Alexandra"), TEXT("Nalen"), TEXT("Katlinka"),
        TEXT("LaCarterie"), TEXT("MarokanskKlubb")}))
        OutCategory = ETMOPMapMarkerCategory::Club;
    else if (ContainsAny(AnchorId, {TEXT("Pub"), TEXT("Bar"), TEXT("Cafe"),
        TEXT("MonCheri"), TEXT("TreBackar"), TEXT("Sandrews")}))
        OutCategory = ETMOPMapMarkerCategory::Pub;
    else if (ContainsAny(AnchorId, {TEXT("Restaurang"), TEXT("Resturang"),
        TEXT("Bohemia"), TEXT("StClara"), TEXT("CoqBlanc"),
        TEXT("LaCocarade"), TEXT("Gourmet"), TEXT("Karelia"),
        TEXT("Peking"), TEXT("Goldendays")}))
        OutCategory = ETMOPMapMarkerCategory::Restaurant;
    else return false;
    return true;
}

FString VenueKey(FString AnchorId)
{
    AnchorId.ReplaceInline(TEXT("_baksida_entrance"), TEXT(""),
        ESearchCase::IgnoreCase);
    AnchorId.ReplaceInline(TEXT("_entrance"), TEXT(""),
        ESearchCase::IgnoreCase);
    AnchorId.ReplaceInline(TEXT("_inside"), TEXT(""),
        ESearchCase::IgnoreCase);
    return AnchorId;
}

FText VenueDisplayName(const FString& Key)
{
    FString Name = Key;
    if (Name.StartsWith(TEXT("MetroHotorget"), ESearchCase::IgnoreCase))
    {
        const FString Number = Name.Mid(FString(TEXT("MetroHotorget")).Len());
        return FText::FromString(Number.IsEmpty() ? TEXT("Hötorget T-bana")
            : FString::Printf(TEXT("Hötorget T-bana – entré %s"), *Number));
    }
    if (Name.StartsWith(TEXT("MetroRadmansgatan"), ESearchCase::IgnoreCase))
    {
        const FString Number = Name.Mid(FString(TEXT("MetroRadmansgatan")).Len());
        return FText::FromString(Number.IsEmpty() ? TEXT("Rådmansgatan T-bana")
            : FString::Printf(TEXT("Rådmansgatan T-bana – entré %s"), *Number));
    }
    const TArray<FString> Prefixes = {TEXT("Restaurang"), TEXT("Resturang"),
        TEXT("Biograf"), TEXT("Nattklubb")};
    for (const FString& Prefix : Prefixes)
        if (Name.StartsWith(Prefix, ESearchCase::IgnoreCase))
        {
            Name.RightChopInline(Prefix.Len());
            break;
        }
    Name.ReplaceInline(TEXT("_"), TEXT(" "));
    const TMap<FString, FString> PrettyNames = {
        {TEXT("StClara"), TEXT("St. Clara")},
        {TEXT("LaCocarade"), TEXT("La Cocarde")},
        {TEXT("MonCheri"), TEXT("Mon Chéri")},
        {TEXT("CoqBlanc"), TEXT("Coq Blanc")},
        {TEXT("TreBackar"), TEXT("Tre Backar")},
        {TEXT("goldendays"), TEXT("Golden Days")},
        {TEXT("sandrews"), TEXT("S:t Andrews")}
    };
    if (const FString* Pretty = PrettyNames.Find(Name)) Name = *Pretty;
    return FText::FromString(Name);
}
}

UTMOPMapComponent::UTMOPMapComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPMapComponent::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoDiscoverVenueMarkers) DiscoverVenueMarkers();
}

FVector2D UTMOPMapComponent::WorldToMapUV(const FVector WorldLocation) const
{
    const FVector2D Size = WorldMaximum - WorldMinimum;
    const FVector2D NormalizedWorld(
        FMath::IsNearlyZero(Size.X) ? 0.5f : (WorldLocation.X - WorldMinimum.X) / Size.X,
        FMath::IsNearlyZero(Size.Y) ? 0.5f : (WorldLocation.Y - WorldMinimum.Y) / Size.Y);
    FVector2D UV = bSwapWorldAxes
        ? FVector2D(NormalizedWorld.Y, NormalizedWorld.X)
        : NormalizedWorld;
    if (bInvertImageX) UV.X = 1.0f - UV.X;
    if (bInvertImageY) UV.Y = 1.0f - UV.Y;
    return UV;
}

FVector UTMOPMapComponent::MapUVToWorld(FVector2D MapUV, const float WorldZ) const
{
    if (bInvertImageY) MapUV.Y = 1.0f - MapUV.Y;
    if (bInvertImageX) MapUV.X = 1.0f - MapUV.X;
    const FVector2D NormalizedWorld = bSwapWorldAxes
        ? FVector2D(MapUV.Y, MapUV.X)
        : MapUV;
    const FVector2D XY = WorldMinimum +
        NormalizedWorld * (WorldMaximum - WorldMinimum);
    return FVector(XY.X, XY.Y, WorldZ);
}

void UTMOPMapComponent::AddOrUpdateMarker(const FTMOPMapMarker& Marker)
{
    if (Marker.MarkerId.IsNone()) return;
    const int32 Index = Markers.IndexOfByPredicate([&Marker](const FTMOPMapMarker& Existing)
        { return Existing.MarkerId == Marker.MarkerId; });
    if (Index == INDEX_NONE) Markers.Add(Marker);
    else Markers[Index] = Marker;
}

bool UTMOPMapComponent::SetMarkerDiscovered(const FName MarkerId, const bool bDiscovered)
{
    FTMOPMapMarker* Marker = Markers.FindByPredicate([MarkerId](const FTMOPMapMarker& Existing)
        { return Existing.MarkerId == MarkerId; });
    if (Marker == nullptr) return false;
    Marker->bDiscovered = bDiscovered;
    return true;
}

int32 UTMOPMapComponent::DiscoverVenueMarkers()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    struct FCandidate
    {
        ATMOPHistoricalAnchor* Anchor = nullptr;
        ETMOPMapMarkerCategory Category = ETMOPMapMarkerCategory::Custom;
        bool bEntrance = false;
    };
    TMap<FString, FCandidate> Venues;
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        ATMOPHistoricalAnchor* Anchor = *It;
        if (!IsValid(Anchor)) continue;
        const FString Id = Anchor->GetAnchorId().ToString();
        ETMOPMapMarkerCategory Category;
        if (!ClassifyVenue(Id, Category)) continue;
        const bool bEntrance = Id.Contains(TEXT("entrance"), ESearchCase::IgnoreCase);
        const bool bInside = Id.Contains(TEXT("inside"), ESearchCase::IgnoreCase);
        if (!bEntrance && bInside && Venues.Contains(VenueKey(Id))) continue;
        const FString Key = VenueKey(Id);
        FCandidate* Existing = Venues.Find(Key);
        if (Existing == nullptr || (bEntrance && !Existing->bEntrance))
        {
            FCandidate Candidate;
            Candidate.Anchor = Anchor;
            Candidate.Category = Category;
            Candidate.bEntrance = bEntrance;
            Venues.Add(Key, Candidate);
        }
    }

    int32 Added = 0;
    for (const TPair<FString, FCandidate>& Pair : Venues)
    {
        if (!IsValid(Pair.Value.Anchor)) continue;
        FTMOPMapMarker Marker;
        Marker.MarkerId = FName(*FString::Printf(TEXT("VENUE_%s"), *Pair.Key));
        Marker.DisplayName = VenueDisplayName(Pair.Key);
        Marker.WorldLocation = Pair.Value.Anchor->GetAnchorLocation();
        Marker.Category = Pair.Value.Category;
        Marker.Icon = GetCategoryIcon(Marker.Category);
        switch (Marker.Category)
        {
        case ETMOPMapMarkerCategory::Metro: Marker.Color = FLinearColor(0.15f, 0.65f, 1.0f); break;
        case ETMOPMapMarkerCategory::Cinema: Marker.Color = FLinearColor(0.95f, 0.3f, 0.25f); break;
        case ETMOPMapMarkerCategory::Restaurant: Marker.Color = FLinearColor(1.0f, 0.65f, 0.12f); break;
        case ETMOPMapMarkerCategory::Club: Marker.Color = FLinearColor(0.75f, 0.25f, 1.0f); break;
        case ETMOPMapMarkerCategory::Pub: Marker.Color = FLinearColor(0.25f, 0.85f, 0.45f); break;
        default: break;
        }
        AddOrUpdateMarker(Marker);
        ++Added;
    }
    UE_LOG(LogTemp, Display, TEXT("TMOP map: discovered %d venue marker(s)."), Added);
    return Added;
}

UTexture2D* UTMOPMapComponent::GetCategoryIcon(const ETMOPMapMarkerCategory Category) const
{
    switch (Category)
    {
    case ETMOPMapMarkerCategory::Restaurant: return RestaurantIcon;
    case ETMOPMapMarkerCategory::Cinema: return CinemaIcon;
    case ETMOPMapMarkerCategory::Metro: return MetroIcon;
    case ETMOPMapMarkerCategory::Club: return ClubIcon;
    case ETMOPMapMarkerCategory::Pub: return PubIcon;
    default: return nullptr;
    }
}

FVector UTMOPMapComponent::GetTrackedWorldLocation() const
{
    return GetOwner() != nullptr ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

FVector2D UTMOPMapComponent::GetTrackedMapDirection() const
{
    if (GetOwner() == nullptr) return FVector2D(0.0f, -1.0f);
    const FVector Location = GetOwner()->GetActorLocation();
    FVector Direction = GetOwner()->GetActorForwardVector();
    Direction = Direction.RotateAngleAxis(-MapNorthYawDegrees, FVector::UpVector);
    const FVector2D MapDelta = WorldToMapUV(Location + Direction * 1000.0f) -
        WorldToMapUV(Location);
    return MapDelta.IsNearlyZero() ? FVector2D(0.0f, -1.0f)
        : MapDelta.GetSafeNormal();
}
