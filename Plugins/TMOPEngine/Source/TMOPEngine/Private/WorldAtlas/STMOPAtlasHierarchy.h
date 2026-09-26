#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"

struct FTMOPAtlasEntry;
// Entry belongs to the atlas data, which outlives its detail widgets.
TSharedRef<SWidget> MakeTMOPAtlasHierarchy(const FTMOPAtlasEntry& Entry);
