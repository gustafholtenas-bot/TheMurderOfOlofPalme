#include "Events/TMOPHistoricalEventDirector.h"

#include "Engine/DataTable.h"
#include "Events/TMOPHistoricalEventSubsystem.h"

ATMOPHistoricalEventDirector::ATMOPHistoricalEventDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPHistoricalEventDirector::BeginPlay()
{
    Super::BeginPlay();
    RegisterAllEvents();
}

void ATMOPHistoricalEventDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (bUnregisterOnEndPlay)
    {
        UnregisterAllEvents();
    }

    Super::EndPlay(EndPlayReason);
}

int32 ATMOPHistoricalEventDirector::RegisterAllEvents()
{
    UnregisterAllEvents();

    TArray<FString> Errors;
    if (!ValidateEvents(Errors))
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP historical event director: %s"), *Error);
        }
    }

    int32 RegisteredCount = 0;

    if (EventTable != nullptr)
    {
        static const FString Context(TEXT("TMOPHistoricalEventDirector"));
        TArray<FTMOPHistoricalEventDefinition*> Rows;
        EventTable->GetAllRows(Context, Rows);

        for (const FTMOPHistoricalEventDefinition* Row : Rows)
        {
            if (Row != nullptr && RegisterDefinition(*Row))
            {
                ++RegisteredCount;
            }
        }
    }

    // Inline definitions intentionally come last, so they can override table rows.
    for (const FTMOPHistoricalEventDefinition& Definition : EventDefinitions)
    {
        if (RegisterDefinition(Definition))
        {
            ++RegisteredCount;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("TMOP historical event director registered %d event definition(s)."),
        RegisteredCount);

    return RegisteredCount;
}

void ATMOPHistoricalEventDirector::UnregisterAllEvents()
{
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPHistoricalEventSubsystem* Events = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPHistoricalEventSubsystem>()
        : nullptr;

    if (Events != nullptr)
    {
        for (const FName EventId : RegisteredEventIds)
        {
            Events->RemoveEventDefinition(EventId);
        }
    }

    RegisteredEventIds.Reset();
}

bool ATMOPHistoricalEventDirector::ValidateEvents(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TSet<FName> SeenIds;

    auto ValidateDefinition = [&OutErrors, &SeenIds](
        const FTMOPHistoricalEventDefinition& Definition,
        const FString& Source)
    {
        if (Definition.EventId.IsNone())
        {
            OutErrors.Add(FString::Printf(TEXT("%s has an empty EventId."), *Source));
            return;
        }

        if (SeenIds.Contains(Definition.EventId))
        {
            OutErrors.Add(FString::Printf(
                TEXT("Duplicate EventId '%s' (%s)."),
                *Definition.EventId.ToString(), *Source));
        }
        SeenIds.Add(Definition.EventId);

        if (Definition.TimingMode == ETMOPEventTimingMode::Window &&
            Definition.LatestTime.ToSecondsFromMidnight() <
                Definition.EarliestTime.ToSecondsFromMidnight())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Event '%s' has LatestTime before EarliestTime."),
                *Definition.EventId.ToString()));
        }

        if (Definition.TimingMode == ETMOPEventTimingMode::Relative &&
            Definition.TriggerEventId.IsNone())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Relative event '%s' has no TriggerEventId."),
                *Definition.EventId.ToString()));
        }
    };

    if (EventTable != nullptr)
    {
        if (EventTable->GetRowStruct() !=
            FTMOPHistoricalEventDefinition::StaticStruct())
        {
            OutErrors.Add(TEXT(
                "EventTable must use TMOP Historical Event Definition as its row type."));
        }
        else
        {
            static const FString Context(TEXT("TMOPHistoricalEventValidation"));
            TArray<FTMOPHistoricalEventDefinition*> Rows;
            EventTable->GetAllRows(Context, Rows);
            for (const FTMOPHistoricalEventDefinition* Row : Rows)
            {
                if (Row != nullptr)
                {
                    ValidateDefinition(*Row, TEXT("EventTable"));
                }
            }
        }
    }

    for (int32 Index = 0; Index < EventDefinitions.Num(); ++Index)
    {
        ValidateDefinition(
            EventDefinitions[Index],
            FString::Printf(TEXT("EventDefinitions[%d]"), Index));
    }

    return OutErrors.IsEmpty();
}

bool ATMOPHistoricalEventDirector::RegisterDefinition(
    const FTMOPHistoricalEventDefinition& Definition)
{
    if (Definition.EventId.IsNone())
    {
        return false;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UTMOPHistoricalEventSubsystem* Events = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPHistoricalEventSubsystem>()
        : nullptr;

    if (Events == nullptr)
    {
        return false;
    }

    if (Events->HasEventDefinition(Definition.EventId) &&
        !bReplaceExistingDefinitions)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP historical event '%s' already exists; keeping existing definition."),
            *Definition.EventId.ToString());
        return false;
    }

    if (!Events->RegisterEventDefinition(Definition))
    {
        return false;
    }

    RegisteredEventIds.AddUnique(Definition.EventId);
    return true;
}
