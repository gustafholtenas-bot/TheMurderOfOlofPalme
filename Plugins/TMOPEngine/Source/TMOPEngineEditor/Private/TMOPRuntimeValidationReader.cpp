#include "TMOPRuntimeValidationReader.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    struct FArrivalResult
    {
        int32 PlannedSecond = INDEX_NONE;
        int32 ActualSecond = INDEX_NONE;
        float DeviationSeconds = 0.0f;
        FString Event;
        FString Severity;
        FString Message;
        FString ReportPath;
    };

    FString MakeKey(const FName EntityId, const FName EntryId)
    {
        return EntityId.ToString() + TEXT("\x1f") + EntryId.ToString();
    }

    FString FormatClock(const int32 Second)
    {
        const int32 Normalized = FMath::Max(0, Second) % (24 * 3600);
        return FString::Printf(TEXT("%02d:%02d:%02d"),
            Normalized / 3600,
            (Normalized / 60) % 60,
            Normalized % 60);
    }

    FString FindLatestReportPath()
    {
        const FString Directory = FPaths::Combine(
            FPaths::ProjectSavedDir(), TEXT("TMOP"), TEXT("Validation"));
        TArray<FString> Files;
        IFileManager::Get().FindFiles(
            Files,
            *FPaths::Combine(Directory, TEXT("TimelineValidation_*.json")),
            true,
            false);
        Files.Sort();
        return Files.IsEmpty()
            ? FString()
            : FPaths::Combine(Directory, Files.Last());
    }

    class FValidationCache
    {
    public:
        static FValidationCache& Get()
        {
            static FValidationCache Cache;
            return Cache;
        }

        uint64 GetRevision()
        {
            Refresh();
            return Revision;
        }

        const FArrivalResult* Find(
            const FName EntityId, const FName EntryId)
        {
            Refresh();
            return Results.Find(MakeKey(EntityId, EntryId));
        }

    private:
        void Refresh()
        {
            const FString LatestPath = FindLatestReportPath();
            const FDateTime LatestTimestamp = LatestPath.IsEmpty()
                ? FDateTime::MinValue()
                : IFileManager::Get().GetTimeStamp(*LatestPath);
            if (LatestPath == LoadedPath && LatestTimestamp == LoadedTimestamp)
                return;

            LoadedPath = LatestPath;
            LoadedTimestamp = LatestTimestamp;
            Revision = LatestPath.IsEmpty()
                ? 0
                : static_cast<uint64>(LatestTimestamp.GetTicks());
            Results.Reset();
            if (LatestPath.IsEmpty()) return;

            FString Json;
            if (!FFileHelper::LoadFileToString(Json, *LatestPath)) return;
            TSharedPtr<FJsonObject> Root;
            const TSharedRef<TJsonReader<>> Reader =
                TJsonReaderFactory<>::Create(Json);
            if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
                return;

            const TArray<TSharedPtr<FJsonValue>>* Records = nullptr;
            if (!Root->TryGetArrayField(TEXT("records"), Records) ||
                Records == nullptr)
                return;

            for (const TSharedPtr<FJsonValue>& Value : *Records)
            {
                const TSharedPtr<FJsonObject> Object =
                    Value.IsValid() ? Value->AsObject() : nullptr;
                if (!Object.IsValid()) continue;
                FString Event;
                Object->TryGetStringField(TEXT("event"), Event);
                if (Event != TEXT("Completed") &&
                    Event != TEXT("Failed") &&
                    Event != TEXT("VehicleArrived") &&
                    Event != TEXT("VehicleArrivalFailed"))
                    continue;

                bool bScheduledAsArrival = false;
                Object->TryGetBoolField(
                    TEXT("scheduledAsArrival"), bScheduledAsArrival);
                if ((Event == TEXT("Completed") || Event == TEXT("Failed")) &&
                    !bScheduledAsArrival)
                    continue;

                FString Entity;
                FString Entry;
                double Planned = INDEX_NONE;
                double Actual = INDEX_NONE;
                double Deviation = 0.0;
                if (!Object->TryGetStringField(TEXT("entityId"), Entity) ||
                    !Object->TryGetStringField(TEXT("entryId"), Entry) ||
                    Entity.IsEmpty() || Entry.IsEmpty() ||
                    !Object->TryGetNumberField(TEXT("plannedSecond"), Planned) ||
                    !Object->TryGetNumberField(TEXT("actualSecond"), Actual) ||
                    Planned < 0.0 || Actual < 0.0)
                    continue;
                Object->TryGetNumberField(
                    TEXT("timeDeviationSeconds"), Deviation);

                FArrivalResult Result;
                Result.PlannedSecond = FMath::RoundToInt(Planned);
                Result.ActualSecond = FMath::RoundToInt(Actual);
                Result.DeviationSeconds = static_cast<float>(Deviation);
                Result.Event = Event;
                Object->TryGetStringField(TEXT("severity"), Result.Severity);
                Object->TryGetStringField(TEXT("message"), Result.Message);
                Result.ReportPath = LatestPath;
                Results.Add(MakeKey(FName(*Entity), FName(*Entry)),
                    MoveTemp(Result));
            }
        }

        FString LoadedPath;
        FDateTime LoadedTimestamp = FDateTime::MinValue();
        uint64 Revision = 0;
        TMap<FString, FArrivalResult> Results;
    };
}

uint64 TMOPRuntimeValidation::GetLatestReportRevision()
{
    return FValidationCache::Get().GetRevision();
}

bool TMOPRuntimeValidation::BuildArrivalBadge(
    const FName EntityId,
    const FName EntryId,
    FText& OutText,
    FText& OutToolTip,
    FLinearColor& OutColor)
{
    if (EntityId.IsNone() || EntryId.IsNone()) return false;
    const FArrivalResult* Result =
        FValidationCache::Get().Find(EntityId, EntryId);
    if (Result == nullptr) return false;

    if (Result->Event == TEXT("VehicleArrivalFailed") ||
        Result->Event == TEXT("Failed"))
    {
        OutText = FText::FromString(TEXT("ANKOMST MISSLYCKADES"));
        OutColor = FLinearColor(0.75f, 0.05f, 0.03f);
    }
    else
    {
        const int32 RoundedDeviation =
            FMath::RoundToInt(Result->DeviationSeconds);
        if (RoundedDeviation == 0)
        {
            OutText = FText::FromString(TEXT("ANKOM I TID"));
            OutColor = FLinearColor(0.05f, 0.42f, 0.12f);
        }
        else if (RoundedDeviation > 0)
        {
            OutText = FText::FromString(FString::Printf(
                TEXT("ANKOM +%d s SEN"), RoundedDeviation));
            OutColor = Result->Severity == TEXT("Error")
                ? FLinearColor(0.75f, 0.05f, 0.03f)
                : FLinearColor(0.78f, 0.38f, 0.03f);
        }
        else
        {
            OutText = FText::FromString(FString::Printf(
                TEXT("ANKOM %d s TIDIGT"), FMath::Abs(RoundedDeviation)));
            OutColor = Result->Severity == TEXT("Error")
                ? FLinearColor(0.75f, 0.05f, 0.03f)
                : FLinearColor(0.78f, 0.38f, 0.03f);
        }
    }

    FString ToolTip = FString::Printf(
        TEXT("Planerad ankomst: %s\nFaktisk ankomst: %s\nAvvikelse: %+.0f sekunder"),
        *FormatClock(Result->PlannedSecond),
        *FormatClock(Result->ActualSecond),
        Result->DeviationSeconds);
    if (!Result->Message.IsEmpty()) ToolTip += TEXT("\n") + Result->Message;
    ToolTip += TEXT("\nRapport: ") + Result->ReportPath;
    OutToolTip = FText::FromString(ToolTip);
    return true;
}
