#pragma once

#include "CoreMinimal.h"

class SWidget;
class UStaticMesh;
class UMaterialInterface;

TSharedRef<SWidget> MakeTMOPWorldAtlas(UStaticMesh* Mesh, UMaterialInterface* Material,
    const FRotator& MeshAlignment, bool bCoastlineOverlay);
