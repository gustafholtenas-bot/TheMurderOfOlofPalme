#pragma once

#include "CoreMinimal.h"
#include "NavModifierVolume.h"
#include "TMOPNavigationModifierVolumes.generated.h"

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPSidewalkNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPSidewalkNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPCrosswalkNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPCrosswalkNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPRoadNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPRoadNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPStairsNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPStairsNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPBridgeNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPBridgeNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPInteriorNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPInteriorNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPRestrictedNavVolume : public ANavModifierVolume
{
    GENERATED_BODY()

public:
    ATMOPRestrictedNavVolume(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
