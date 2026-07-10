#include "World/TMOPWorldTypes.h"

FTMOPWorldStateValue FTMOPWorldStateValue::MakeBoolean(const bool Value)
{
    FTMOPWorldStateValue Result;
    Result.ValueType = ETMOPWorldValueType::Boolean;
    Result.BooleanValue = Value;
    return Result;
}

FTMOPWorldStateValue FTMOPWorldStateValue::MakeInteger(const int32 Value)
{
    FTMOPWorldStateValue Result;
    Result.ValueType = ETMOPWorldValueType::Integer;
    Result.IntegerValue = Value;
    return Result;
}

FTMOPWorldStateValue FTMOPWorldStateValue::MakeFloat(const float Value)
{
    FTMOPWorldStateValue Result;
    Result.ValueType = ETMOPWorldValueType::Float;
    Result.FloatValue = Value;
    return Result;
}

FTMOPWorldStateValue FTMOPWorldStateValue::MakeName(const FName Value)
{
    FTMOPWorldStateValue Result;
    Result.ValueType = ETMOPWorldValueType::Name;
    Result.NameValue = Value;
    return Result;
}

FTMOPWorldStateValue FTMOPWorldStateValue::MakeString(const FString& Value)
{
    FTMOPWorldStateValue Result;
    Result.ValueType = ETMOPWorldValueType::String;
    Result.StringValue = Value;
    return Result;
}

FString FTMOPWorldStateValue::ToDebugString() const
{
    switch (ValueType)
    {
    case ETMOPWorldValueType::Boolean:
        return BooleanValue ? TEXT("true") : TEXT("false");
    case ETMOPWorldValueType::Integer:
        return FString::FromInt(IntegerValue);
    case ETMOPWorldValueType::Float:
        return FString::SanitizeFloat(FloatValue);
    case ETMOPWorldValueType::Name:
        return NameValue.ToString();
    case ETMOPWorldValueType::String:
        return StringValue;
    default:
        return TEXT("<none>");
    }
}

bool FTMOPWorldStateValue::operator==(const FTMOPWorldStateValue& Other) const
{
    if (ValueType != Other.ValueType)
    {
        return false;
    }

    switch (ValueType)
    {
    case ETMOPWorldValueType::Boolean:
        return BooleanValue == Other.BooleanValue;
    case ETMOPWorldValueType::Integer:
        return IntegerValue == Other.IntegerValue;
    case ETMOPWorldValueType::Float:
        return FMath::IsNearlyEqual(FloatValue, Other.FloatValue);
    case ETMOPWorldValueType::Name:
        return NameValue == Other.NameValue;
    case ETMOPWorldValueType::String:
        return StringValue == Other.StringValue;
    default:
        return true;
    }
}

bool FTMOPWorldStateValue::operator!=(const FTMOPWorldStateValue& Other) const
{
    return !(*this == Other);
}
