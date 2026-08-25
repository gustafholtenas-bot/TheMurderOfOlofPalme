#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/TMOPItemMeshTypes.h"
#include "TMOPItemMeshSubsystem.generated.h"

class UDataTable;
class UStaticMesh;

/** Central cached resolver for /Game/TMOP/Data/DT_TMOP_ItemMeshes. */
UCLASS()
class TMOPENGINE_API UTMOPItemMeshSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UTMOPItemMeshSubsystem();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Item Meshes")
    TObjectPtr<UDataTable> ItemMeshTable;

    UFUNCTION(BlueprintPure, Category="TMOP|Item Meshes")
    bool FindItemMeshDefinition(FName ItemId, FTMOPItemMeshRow& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Item Meshes")
    UStaticMesh* ResolveStaticMesh(FName ItemId) const;
};
