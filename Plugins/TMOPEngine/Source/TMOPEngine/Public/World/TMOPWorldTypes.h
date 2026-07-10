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

    static FTMOPWorldStateValue MakeBoolean(bool Value);
    static FTMOPWorldStateValue MakeInteger(int32 Value);
    static FTMOPWorldStateValue MakeFloat(float Value);
    static FTMOPWorldStateValue MakeName(FName Value);
    static FTMOPWorldStateValue MakeString(const FString& Value);

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
