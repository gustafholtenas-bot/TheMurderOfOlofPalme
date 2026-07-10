#pragma once

#include "CoreMinimal.h"
#include "TMOPWorldTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPWorldValueType : uint8
{
    None,
    Boolean,
    Integer,
    Float,
    Name,
    String
};

/**
 * A small serializable value used by the causal world-state database.
 * Only the field matching ValueType is considered active.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPWorldStateValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|World")
    ETMOPWorldValueType ValueType = ETMOPWorldValueType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|World")
    bool BooleanValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|World")
    int32 IntegerValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|World")
    float FloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|World")
    FName NameValue = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|World")
    FString StringValue;

    UFUNCTION(BlueprintPure, Category = "TMOP|World")
    static FTMOPWorldStateValue MakeBoolean(bool Value);

    UFUNCTION(BlueprintPure, Category = "TMOP|World")
    static FTMOPWorldStateValue MakeInteger(int32 Value);

    UFUNCTION(BlueprintPure, Category = "TMOP|World")
    static FTMOPWorldStateValue MakeFloat(float Value);

    UFUNCTION(BlueprintPure, Category = "TMOP|World")
    static FTMOPWorldStateValue MakeName(FName Value);

    UFUNCTION(BlueprintPure, Category = "TMOP|World")
    static FTMOPWorldStateValue MakeString(const FString& Value);

    UFUNCTION(BlueprintPure, Category = "TMOP|World")
    FString ToDebugString() const;

    bool operator==(const FTMOPWorldStateValue& Other) const;
    bool operator!=(const FTMOPWorldStateValue& Other) const;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPWorldObjectInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|World")
    FName ObjectId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|World")
    FName ObjectType = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|World")
    TObjectPtr<UObject> Object = nullptr;
};
