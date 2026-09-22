#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPlayerAppearanceSave.generated.h"

/** Local cosmetic settings; never writes back to historical person tables. */
UCLASS()
class TMOPENGINE_API UTMOPPlayerAppearanceSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) TMap<int32, FTMOPPersonProfileRow> Players;
};
