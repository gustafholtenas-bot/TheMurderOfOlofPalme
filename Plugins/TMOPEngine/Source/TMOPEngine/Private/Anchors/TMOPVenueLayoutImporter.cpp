#include "Anchors/TMOPVenueLayoutImporter.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    const FName ImportedVenueLayoutTag(TEXT("TMOP_IMPORTED_VENUE_LAYOUT_ANCHOR"));

    bool ReadVenueLayoutNumber(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, double& Out)
    {
        return Object.IsValid() && Object->TryGetNumberField(Field, Out);
    }
}

ATMOPVenueLayoutImporter::ATMOPVenueLayoutImporter()
{
    PrimaryActorTick.bCanEverTick = false;
    AnchorClass = ATMOPHistoricalAnchor::StaticClass();
}

FString ATMOPVenueLayoutImporter::GetResolvedJsonPath() const
{
    if (FPaths::IsRelative(JsonFilePath))
        return FPaths::ConvertRelativePathToFull(
            FPaths::Combine(FPaths::ProjectContentDir(), JsonFilePath));
    return FPaths::ConvertRelativePathToFull(JsonFilePath);
}

void ATMOPVenueLayoutImporter::ImportOrUpdateVenueLayoutAnchors()
{
    LastCreatedCount = LastUpdatedCount = LastErrorCount = 0;

    FString JsonText;
    const FString Path = GetResolvedJsonPath();
    if (!FFileHelper::LoadFileToString(JsonText, *Path))
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error, TEXT("TMOP venue import: could not read '%s'."), *Path);
        return;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error, TEXT("TMOP venue import: invalid JSON."));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
    if (!Root->TryGetArrayField(TEXT("anchors"), Values) || Values == nullptr)
    {
        ++LastErrorCount;
        UE_LOG(LogTemp, Error, TEXT("TMOP venue import: missing anchors array."));
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr || AnchorClass.Get() == nullptr)
    {
        ++LastErrorCount;
        return;
    }

    TSet<FName> ImportedIds;
    for (const TSharedPtr<FJsonValue>& Value : *Values)
    {
        const TSharedPtr<FJsonObject> Json = Value.IsValid() ? Value->AsObject() : nullptr;
        FString IdString;
        FString ParentString;
        const TSharedPtr<FJsonObject>* OffsetObject = nullptr;
        if (!Json.IsValid() || !Json->TryGetStringField(TEXT("anchor_id"), IdString) ||
            IdString.IsEmpty() ||
            !Json->TryGetStringField(TEXT("parent_anchor_id"), ParentString) ||
            ParentString.IsEmpty() ||
            !Json->TryGetObjectField(TEXT("relative_offset_cm"), OffsetObject) ||
            OffsetObject == nullptr || !OffsetObject->IsValid())
        {
            ++LastErrorCount;
            continue;
        }

        const FName AnchorId(*IdString);
        const FName ParentId(*ParentString);
        if (ImportedIds.Contains(AnchorId))
        {
            ++LastErrorCount;
            continue;
        }
        ImportedIds.Add(AnchorId);

        ATMOPHistoricalAnchor* Parent = FindExistingAnchor(ParentId);
        if (!IsValid(Parent))
        {
            ++LastErrorCount;
            UE_LOG(LogTemp, Warning, TEXT("TMOP venue import: missing midpoint '%s' for '%s'."),
                *ParentString, *IdString);
            continue;
        }

        double X = 0.0, Y = 0.0, Z = 0.0, RelativeYaw = 0.0;
        if (!ReadVenueLayoutNumber(*OffsetObject, TEXT("x"), X) ||
            !ReadVenueLayoutNumber(*OffsetObject, TEXT("y"), Y) ||
            !ReadVenueLayoutNumber(*OffsetObject, TEXT("z"), Z))
        {
            ++LastErrorCount;
            continue;
        }
        Json->TryGetNumberField(TEXT("relative_yaw_degrees"), RelativeYaw);

        const FTransform ParentTransform = Parent->GetActorTransform();
        const FVector Location = ParentTransform.TransformPosition(
            FVector(static_cast<float>(X), static_cast<float>(Y), static_cast<float>(Z)));
        const FRotator Rotation(0.0f,
            Parent->GetActorRotation().Yaw + static_cast<float>(RelativeYaw), 0.0f);

        ATMOPHistoricalAnchor* Anchor = FindExistingAnchor(AnchorId);
        const bool bUpdating = IsValid(Anchor);
        if (bUpdating && !bUpdateExistingAnchors)
        {
            ++LastErrorCount;
            continue;
        }

        if (!bUpdating)
        {
            FActorSpawnParameters Params;
            Params.Name = MakeUniqueObjectName(World->PersistentLevel, AnchorClass.Get(), AnchorId);
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Anchor = World->SpawnActor<ATMOPHistoricalAnchor>(AnchorClass, Location, Rotation, Params);
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
            Anchor->SetActorLocationAndRotation(Location, Rotation, false, nullptr,
                ETeleportType::TeleportPhysics);
            ++LastUpdatedCount;
        }

        Anchor->EntityIdentity->EntityId = AnchorId;
        Anchor->EntityIdentity->EntityType = TEXT("Anchor");
        Anchor->DisplayName = FText::FromName(AnchorId);
        Anchor->AnchorCategory = ETMOPAnchorCategory::InteriorPoint;
        Anchor->ParentAnchorId = ParentId;
        Anchor->Confidence = ETMOPHistoricalConfidence::Reconstructed;
        Anchor->PlacementRadiusCm = 0.0f;
        Anchor->MinimumSpacingCm = 80.0f;
        Anchor->bProjectPlacementToNavMesh = false;
        Anchor->bCanBeUsedForRouting = true;
        Anchor->bHardHistoricalAnchor = false;
        Json->TryGetStringField(TEXT("notes"), Anchor->Notes);
        Anchor->Tags.AddUnique(ImportedVenueLayoutTag);

#if WITH_EDITOR
        Anchor->SetActorLabel(IdString);
        Anchor->SetFolderPath(*FString::Printf(TEXT("TMOP/Anchors/VenueLayouts/%s"), *ParentString));
#endif
        Anchor->RerunConstructionScripts();
        Anchor->MarkPackageDirty();
    }

    MarkPackageDirty();
    UE_LOG(LogTemp, Display, TEXT("TMOP venue import complete: %d created, %d updated, %d errors."),
        LastCreatedCount, LastUpdatedCount, LastErrorCount);
}

ATMOPHistoricalAnchor* ATMOPVenueLayoutImporter::FindExistingAnchor(const FName AnchorId) const
{
    if (UWorld* World = GetWorld())
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
            if (IsValid(*It) && It->GetAnchorId() == AnchorId)
                return *It;
    return nullptr;
}
