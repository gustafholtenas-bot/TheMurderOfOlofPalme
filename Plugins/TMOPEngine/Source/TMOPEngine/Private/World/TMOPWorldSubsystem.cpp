#include "World/TMOPWorldSubsystem.h"

#include "Engine/GameInstance.h"
#include "Time/TMOPClockSubsystem.h"

void UTMOPWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RegisteredObjects.Reset();
    RuntimeWorldState.Reset();
    BaselineWorldState.Reset();

    Collection.InitializeDependency<UTMOPClockSubsystem>();

    if (UTMOPClockSubsystem* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        Clock->OnLoopRestarted.AddDynamic(this, &UTMOPWorldSubsystem::HandleLoopRestarted);
    }
}

void UTMOPWorldSubsystem::Deinitialize()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock = GameInstance->GetSubsystem<UTMOPClockSubsystem>())
        {
            Clock->OnLoopRestarted.RemoveDynamic(this, &UTMOPWorldSubsystem::HandleLoopRestarted);
        }
    }

    RegisteredObjects.Reset();
    RuntimeWorldState.Reset();
    BaselineWorldState.Reset();

    Super::Deinitialize();
}

bool UTMOPWorldSubsystem::RegisterWorldObject(
    const FName ObjectId,
    const FName ObjectType,
    UObject* Object)
{
    if (ObjectId.IsNone() || !IsValid(Object))
    {
        return false;
    }

    if (const FRegisteredObject* Existing = RegisteredObjects.Find(ObjectId))
    {
        if (Existing->Object.IsValid() && Existing->Object.Get() != Object)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP world rejected duplicate object ID '%s'."),
                *ObjectId.ToString());
            return false;
        }
    }

    FRegisteredObject& Record = RegisteredObjects.FindOrAdd(ObjectId);
    Record.ObjectType = ObjectType;
    Record.Object = Object;

    OnWorldObjectRegistered.Broadcast(ObjectId, ObjectType, Object);
    return true;
}

bool UTMOPWorldSubsystem::UnregisterWorldObject(
    const FName ObjectId,
    UObject* ExpectedObject)
{
    FRegisteredObject* Existing = RegisteredObjects.Find(ObjectId);
    if (Existing == nullptr)
    {
        return false;
    }

    UObject* ExistingObject = Existing->Object.Get();
    if (ExpectedObject != nullptr && ExistingObject != ExpectedObject)
    {
        return false;
    }

    const FName ObjectType = Existing->ObjectType;
    RegisteredObjects.Remove(ObjectId);
    OnWorldObjectUnregistered.Broadcast(ObjectId, ObjectType, ExistingObject);
    return true;
}

UObject* UTMOPWorldSubsystem::FindWorldObject(const FName ObjectId) const
{
    if (const FRegisteredObject* Existing = RegisteredObjects.Find(ObjectId))
    {
        return Existing->Object.Get();
    }

    return nullptr;
}

bool UTMOPWorldSubsystem::IsWorldObjectRegistered(const FName ObjectId) const
{
    const FRegisteredObject* Existing = RegisteredObjects.Find(ObjectId);
    return Existing != nullptr && Existing->Object.IsValid();
}

TArray<FTMOPWorldObjectInfo> UTMOPWorldSubsystem::GetWorldObjectsByType(
    const FName ObjectType) const
{
    TArray<FTMOPWorldObjectInfo> Results;

    for (const TPair<FName, FRegisteredObject>& Pair : RegisteredObjects)
    {
        if (Pair.Value.ObjectType != ObjectType || !Pair.Value.Object.IsValid())
        {
            continue;
        }

        FTMOPWorldObjectInfo Info;
        Info.ObjectId = Pair.Key;
        Info.ObjectType = Pair.Value.ObjectType;
        Info.Object = Pair.Value.Object.Get();
        Results.Add(MoveTemp(Info));
    }

    Results.Sort([](
        const FTMOPWorldObjectInfo& Left,
        const FTMOPWorldObjectInfo& Right)
    {
        return Left.ObjectId.LexicalLess(Right.ObjectId);
    });

    return Results;
}

TArray<FName> UTMOPWorldSubsystem::GetRegisteredObjectIds() const
{
    TArray<FName> Results;
    Results.Reserve(RegisteredObjects.Num());

    for (const TPair<FName, FRegisteredObject>& Pair : RegisteredObjects)
    {
        if (Pair.Value.Object.IsValid())
        {
            Results.Add(Pair.Key);
        }
    }

    Results.Sort(FNameLexicalLess());
    return Results;
}

bool UTMOPWorldSubsystem::SetWorldState(
    const FName StateKey,
    const FTMOPWorldStateValue NewValue)
{
    if (StateKey.IsNone())
    {
        return false;
    }

    FTMOPWorldStateValue OldValue;
    if (const FTMOPWorldStateValue* Existing = RuntimeWorldState.Find(StateKey))
    {
        OldValue = *Existing;
        if (OldValue == NewValue)
        {
            return false;
        }
    }

    RuntimeWorldState.Add(StateKey, NewValue);
    OnWorldStateChanged.Broadcast(StateKey, OldValue, NewValue);
    return true;
}

bool UTMOPWorldSubsystem::TryGetWorldState(
    const FName StateKey,
    FTMOPWorldStateValue& OutValue) const
{
    if (const FTMOPWorldStateValue* Existing = RuntimeWorldState.Find(StateKey))
    {
        OutValue = *Existing;
        return true;
    }

    OutValue = FTMOPWorldStateValue();
    return false;
}

bool UTMOPWorldSubsystem::HasWorldState(const FName StateKey) const
{
    return RuntimeWorldState.Contains(StateKey);
}

bool UTMOPWorldSubsystem::RemoveWorldState(const FName StateKey)
{
    const FTMOPWorldStateValue* Existing = RuntimeWorldState.Find(StateKey);
    if (Existing == nullptr)
    {
        return false;
    }

    const FTMOPWorldStateValue OldValue = *Existing;
    RuntimeWorldState.Remove(StateKey);
    OnWorldStateChanged.Broadcast(
        StateKey,
        OldValue,
        FTMOPWorldStateValue());
    return true;
}

void UTMOPWorldSubsystem::SetBaselineWorldState(
    const FName StateKey,
    const FTMOPWorldStateValue Value)
{
    if (StateKey.IsNone())
    {
        return;
    }

    BaselineWorldState.Add(StateKey, Value);

    if (!RuntimeWorldState.Contains(StateKey))
    {
        RuntimeWorldState.Add(StateKey, Value);
    }
}

bool UTMOPWorldSubsystem::TryGetBaselineWorldState(
    const FName StateKey,
    FTMOPWorldStateValue& OutValue) const
{
    if (const FTMOPWorldStateValue* Existing = BaselineWorldState.Find(StateKey))
    {
        OutValue = *Existing;
        return true;
    }

    OutValue = FTMOPWorldStateValue();
    return false;
}

void UTMOPWorldSubsystem::ResetRuntimeWorldState(const int32 LoopNumber)
{
    RuntimeWorldState = BaselineWorldState;
    RemoveInvalidWorldObjects();
    OnRuntimeWorldReset.Broadcast(LoopNumber);
}

FString UTMOPWorldSubsystem::BuildDebugReport() const
{
    FString Report = FString::Printf(
        TEXT("TMOP World\nRegistered objects: %d\nRuntime states: %d\nBaseline states: %d"),
        RegisteredObjects.Num(),
        RuntimeWorldState.Num(),
        BaselineWorldState.Num());

    TArray<FName> StateKeys;
    RuntimeWorldState.GetKeys(StateKeys);
    StateKeys.Sort(FNameLexicalLess());

    for (const FName Key : StateKeys)
    {
        const FTMOPWorldStateValue& Value = RuntimeWorldState[Key];
        Report += FString::Printf(
            TEXT("\n%s = %s"),
            *Key.ToString(),
            *Value.ToDebugString());
    }

    return Report;
}

int32 UTMOPWorldSubsystem::RemoveInvalidWorldObjects()
{
    int32 RemovedCount = 0;

    for (auto Iterator = RegisteredObjects.CreateIterator(); Iterator; ++Iterator)
    {
        if (!Iterator.Value().Object.IsValid())
        {
            Iterator.RemoveCurrent();
            ++RemovedCount;
        }
    }

    return RemovedCount;
}

void UTMOPWorldSubsystem::HandleLoopRestarted(
    const int32 NewLoopNumber,
    const FTMOPTime RestartTime)
{
    ResetRuntimeWorldState(NewLoopNumber);
}
