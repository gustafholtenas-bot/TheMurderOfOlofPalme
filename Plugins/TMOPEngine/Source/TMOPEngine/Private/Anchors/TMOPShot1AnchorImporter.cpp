#include "Anchors/TMOPShot1AnchorImporter.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    const FName ImportedShot1Tag(TEXT("TMOP_IMPORTED_SHOT1_ANCHOR"));

    bool ReadNumber(
        const TSharedPtr<FJsonObject>& Object,
        const TCHAR* Field,
        double& OutValue)
    {
        return Object.IsValid() && Object->TryGetNumberField(Field, OutValue);
    }

    ETMOPHistoricalConfidence ParseConfidence(const FString& Value)
    {
        if (Value.Equals(TEXT("DOCUMENTED"), ESearchCase::IgnoreCase))
            return ETMOPHistoricalConfidence::Documented;
        if (Value.Equals(TEXT("INFERRED"), ESearchCase::IgnoreCase))
            return ETMOPHistoricalConfidence::Inferred;
        if (Value.Equals(TEXT("SPECULATIVE"), ESearchCase::IgnoreCase))
            return ETMOPHistoricalConfidence::Speculative;
        return ETMOPHistoricalConfidence::Reconstructed;
    }
}

ATMOPShot1AnchorImporter::ATMOPShot1AnchorImporter()
{
    PrimaryActorTick.bCanEverTick = false;
    AnchorClass = ATMOPHistoricalAnchor::StaticClass();
}

FString ATMOPShot1AnchorImporter::GetResolvedJsonPath() const
{
    if (FPaths::IsRelative(JsonFilePath))
        return FPaths::ConvertRelativePathToFull(
            FPaths::Combine(FPaths::ProjectContentDir(), JsonFilePath));
    return FPaths::ConvertRelativePathToFull(JsonFilePath);
}

void ATMOPShot1AnchorImporter::ImportOrUpdateShot1Anchors()
{
    LastCreatedCount = 0;
    LastUpdatedCount = 0;
    LastErrorCount = 0;

    const FString ResolvedPath = GetResolvedJsonPath();
    FString JsonText;
    if (!FFileHelper::LoadFileToString(JsonText, *ResolvedPath))
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Shot1 import: could not read '%s'."), *ResolvedPath);
        return;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Shot1 import: invalid JSON in '%s'."), *ResolvedPath);
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* AnchorValues = nullptr;
    if (!Root->TryGetArrayField(TEXT("anchors"), AnchorValues) ||
        AnchorValues == nullptr)
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Shot1 import: JSON has no 'anchors' array."));
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr || AnchorClass.Get() == nullptr)
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Shot1 import: world or AnchorClass is missing."));
        return;
    }

    TSet<FName> ImportedIds;
    for (const TSharedPtr<FJsonValue>& Value : *AnchorValues)
    {
        const TSharedPtr<FJsonObject> JsonAnchor =
            Value.IsValid() ? Value->AsObject() : nullptr;
        if (!JsonAnchor.IsValid())
        {
            ++LastErrorCount;
            continue;
        }

        FString AnchorIdString;
        if (!JsonAnchor->TryGetStringField(TEXT("anchor_id"), AnchorIdString) ||
            AnchorIdString.IsEmpty())
        {
            ++LastErrorCount;
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Shot1 import: skipped row without anchor_id."));
            continue;
        }

        const FName AnchorId(*AnchorIdString);
        if (ImportedIds.Contains(AnchorId))
        {
            ++LastErrorCount;
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Shot1 import: duplicate ID '%s' in JSON."),
                *AnchorIdString);
            continue;
        }
        ImportedIds.Add(AnchorId);

        const TSharedPtr<FJsonObject>* LocationObject = nullptr;
        if (!JsonAnchor->TryGetObjectField(
                TEXT("unreal_location_cm"), LocationObject) ||
            LocationObject == nullptr || !LocationObject->IsValid())
        {
            ++LastErrorCount;
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Shot1 import: '%s' has no Unreal position."),
                *AnchorIdString);
            continue;
        }

        double X = 0.0;
        double Y = 0.0;
        double Z = 0.0;
        if (!ReadNumber(*LocationObject, TEXT("x"), X) ||
            !ReadNumber(*LocationObject, TEXT("y"), Y) ||
            !ReadNumber(*LocationObject, TEXT("z"), Z))
        {
            ++LastErrorCount;
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Shot1 import: '%s' has an invalid Unreal position."),
                *AnchorIdString);
            continue;
        }

        float BlenderYaw = 0.0f;
        const TSharedPtr<FJsonObject>* RotationObject = nullptr;
        if (JsonAnchor->TryGetObjectField(
                TEXT("blender_rotation_euler_degrees"), RotationObject) &&
            RotationObject != nullptr && RotationObject->IsValid())
        {
            double ZDegrees = 0.0;
            if (ReadNumber(*RotationObject, TEXT("z"), ZDegrees))
                BlenderYaw = static_cast<float>(ZDegrees);
        }

        const FVector Location(X, Y, Z);
        const FRotator Rotation(0.0f, -BlenderYaw, 0.0f);
        ATMOPHistoricalAnchor* Anchor = FindExistingAnchor(AnchorId);
        const bool bUpdating = IsValid(Anchor);
        if (bUpdating && !bUpdateExistingAnchors)
        {
            ++LastErrorCount;
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Shot1 import: '%s' already exists; update disabled."),
                *AnchorIdString);
            continue;
        }

        if (!bUpdating)
        {
            FActorSpawnParameters Params;
            Params.Name = MakeUniqueObjectName(
                World->PersistentLevel, AnchorClass.Get(), AnchorId);
            Params.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Anchor = World->SpawnActor<ATMOPHistoricalAnchor>(
                AnchorClass, Location, Rotation, Params);
            if (!IsValid(Anchor))
            {
                ++LastErrorCount;
                continue;
            }
            ++LastCreatedCount;
        }
        else
        {
            Anchor->Modify();
            Anchor->SetActorLocationAndRotation(
                Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
            ++LastUpdatedCount;
        }

        Anchor->EntityIdentity->EntityId = AnchorId;
        Anchor->EntityIdentity->EntityType = TEXT("Anchor");

        FString DisplayName;
        JsonAnchor->TryGetStringField(TEXT("display_name"), DisplayName);
        Anchor->DisplayName = FText::FromString(
            DisplayName.IsEmpty() ? AnchorIdString : DisplayName);
        Anchor->AnchorCategory = ETMOPAnchorCategory::WitnessPosition;

        FString Confidence;
        JsonAnchor->TryGetStringField(TEXT("confidence"), Confidence);
        Anchor->Confidence = ParseConfidence(Confidence);

        FString SourceObject;
        FString SourceTime;
        FString EventId;
        FString Notes;
        JsonAnchor->TryGetStringField(TEXT("source_object"), SourceObject);
        JsonAnchor->TryGetStringField(TEXT("source_time"), SourceTime);
        JsonAnchor->TryGetStringField(TEXT("event_id"), EventId);
        JsonAnchor->TryGetStringField(TEXT("notes"), Notes);
        Anchor->Notes = FString::Printf(
            TEXT("%s Event=%s; Time=%s; BlenderObject=%s"),
            *Notes, *EventId, *SourceTime, *SourceObject);

        Anchor->Sources.Reset();
        FTMOPSourceReference& Source = Anchor->Sources.AddDefaulted_GetRef();
        Source.SourceId = EventId.IsEmpty() ? TEXT("EVENT_FIRST_SHOT") : EventId;
        Source.SourceType = TEXT("BlenderTimeline");
        Source.Description = SourceObject;
        Source.ExternalReference = SourceTime;

        Anchor->bHardHistoricalAnchor = true;
        Anchor->PlacementRadiusCm = 0.0f;
        Anchor->bProjectPlacementToNavMesh = !bUseExactPositions;
        Anchor->bCanBeUsedForRouting = !bUseExactPositions;
        Anchor->Tags.AddUnique(ImportedShot1Tag);

#if WITH_EDITOR
        Anchor->SetActorLabel(AnchorIdString);
        Anchor->SetFolderPath(TEXT("TMOP/Anchors/Shot1Witnesses"));
#endif
        Anchor->RerunConstructionScripts();
        Anchor->MarkPackageDirty();
    }

    MarkPackageDirty();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP Shot1 import complete: %d created, %d updated, %d errors."),
        LastCreatedCount, LastUpdatedCount, LastErrorCount);
}

ATMOPHistoricalAnchor* ATMOPShot1AnchorImporter::FindExistingAnchor(
    const FName AnchorId) const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
        return nullptr;

    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        ATMOPHistoricalAnchor* Anchor = *It;
        if (IsValid(Anchor) && Anchor->GetAnchorId() == AnchorId)
            return Anchor;
    }
    return nullptr;
}
