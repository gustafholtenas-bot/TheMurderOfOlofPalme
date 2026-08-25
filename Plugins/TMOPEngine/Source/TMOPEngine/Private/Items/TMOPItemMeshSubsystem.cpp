#include "Items/TMOPItemMeshSubsystem.h"

#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
FName NormalizeCatalogId(const FName ItemId)
{
    if (ItemId == TEXT("FYND_BLA_TACKJACKA_ANORALP")) return TEXT("FINDING_BLUE_PADDED_JACKET");
    if (ItemId == TEXT("FYND_STALBAGADE_GLASOGON_OMRADE") ||
        ItemId == TEXT("FYND_STALBAGADE_GLASOGON_SNICKARBACKEN5"))
        return TEXT("FINDING_STEEL_FRAME_GLASSES");
    if (ItemId == TEXT("FYND_LJUSBRUNA_LANGBYXOR")) return TEXT("FINDING_LIGHT_BROWN_TROUSERS");
    if (ItemId == TEXT("FYND_BLA_TYGMOSSA")) return TEXT("FINDING_BLUE_FABRIC_CAP");
    if (ItemId == TEXT("FYND_ROD_DAMHOGERSKO")) return TEXT("FINDING_RED_WOMENS_RIGHT_SHOE");
    if (ItemId == TEXT("FYND_TELEFONBLOCKSLAPP")) return TEXT("FINDING_HANDWRITTEN_PHONE_NOTE");
    if (ItemId == TEXT("FYND_AVBRUTEN_GREN")) return TEXT("FINDING_BROKEN_BRANCH");
    if (ItemId == TEXT("FYND_HALSDUK_MORDPLATS")) return TEXT("FINDING_SCARF");
    if (ItemId == TEXT("FYND_KLADER_HANDELSHOGSKOLAN")) return TEXT("FINDING_CLOTHES_PILE");
    if (ItemId == TEXT("FYND_HANDSKE_KYRKAN") || ItemId == TEXT("FYND_HANDSKE_MORDPLATS"))
        return TEXT("FINDING_GREY_GREEN_GLOVE");
    if (ItemId == TEXT("FYND_KULA_1") || ItemId == TEXT("FYND_KULA_2"))
        return TEXT("FINDING_BULLET_357");
    return ItemId;
}
}

UTMOPItemMeshSubsystem::UTMOPItemMeshSubsystem()
{
    static ConstructorHelpers::FObjectFinder<UDataTable> CatalogFinder(
        TEXT("/Game/TMOP/Data/DT_TMOP_ItemMeshes.DT_TMOP_ItemMeshes"));
    if (CatalogFinder.Succeeded()) ItemMeshTable = CatalogFinder.Object;
}

bool UTMOPItemMeshSubsystem::FindItemMeshDefinition(
    const FName ItemId, FTMOPItemMeshRow& OutDefinition) const
{
    if (!IsValid(ItemMeshTable) || ItemId.IsNone()) return false;
    const FName ResolvedItemId = NormalizeCatalogId(ItemId);
    const FTMOPItemMeshRow* Row = ItemMeshTable->FindRow<FTMOPItemMeshRow>(
        ResolvedItemId, TEXT("TMOP item mesh lookup"), false);
    if (Row == nullptr) return false;
    OutDefinition = *Row;
    return true;
}

UStaticMesh* UTMOPItemMeshSubsystem::ResolveStaticMesh(const FName ItemId) const
{
    FTMOPItemMeshRow Definition;
    return FindItemMeshDefinition(ItemId, Definition)
        ? Definition.Mesh.LoadSynchronous() : nullptr;
}
