#pragma once

#include "CoreMinimal.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "TMOPNavigationQueryFilters.generated.h"

UCLASS()
class TMOPENGINE_API UTMOPNavFilter_NormalPedestrian
    : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UTMOPNavFilter_NormalPedestrian();
};

UCLASS()
class TMOPENGINE_API UTMOPNavFilter_CautiousPedestrian
    : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UTMOPNavFilter_CautiousPedestrian();
};

UCLASS()
class TMOPENGINE_API UTMOPNavFilter_FastPedestrian
    : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UTMOPNavFilter_FastPedestrian();
};

UCLASS()
class TMOPENGINE_API UTMOPNavFilter_HistoricalOverride
    : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UTMOPNavFilter_HistoricalOverride();
};

UCLASS()
class TMOPENGINE_API UTMOPNavFilter_Emergency
    : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UTMOPNavFilter_Emergency();
};
