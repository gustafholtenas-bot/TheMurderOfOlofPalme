#pragma once

#include "CoreMinimal.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "TMOPTimedPropDirector.generated.h"

class UStaticMesh;
class UTMOPItemDefinition;
class ATMOPWorldItem;
class ATMOPFindingActor;

UENUM(BlueprintType)
enum class ETMOPTimedPropAction : uint8
{
    Spawn,
    Despawn
};

UENUM(BlueprintType)
enum class ETMOPTimedPropKind : uint8
{
    StaticMesh UMETA(DisplayName="Static Mesh (Not Pickup)"),
    ActorClass UMETA(DisplayName="Actor Class"),
    PickupItem UMETA(DisplayName="Pickup Item"),
    Finding UMETA(DisplayName="Historical Finding")
};

UENUM(BlueprintType)
enum class ETMOPTimedPropPlacement : uint8
{
    Anchor,
    WorldTransform UMETA(DisplayName="World Transform")
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTimedPropEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop")
    FName EntryId = NAME_None;

    /** Spawn and Despawn entries use the same Instance ID. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop")
    FName InstanceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop")
    ETMOPTimedPropAction Action = ETMOPTimedPropAction::Spawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Time")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Time")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative",
            DisplayName="Shared Event ID"))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative || TimingMode==ETMOPEventTimingMode::RelativeToPreviousEntry",
            DisplayName="Offset Seconds"))
    int32 OffsetSeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Spawn",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn"))
    ETMOPTimedPropKind PropKind = ETMOPTimedPropKind::StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Spawn",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && (PropKind==ETMOPTimedPropKind::StaticMesh || PropKind==ETMOPTimedPropKind::Finding)"))
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    FText FindingDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    FString EvidenceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    FString SourceTimeLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    double SourceLatitude = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    double SourceLongitude = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    bool bLocationApproximate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    FVector FindingScale = FVector(0.25f, 0.25f, 0.12f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Finding",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::Finding"))
    FLinearColor FindingColor = FLinearColor(0.8f, 0.65f, 0.15f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Spawn",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::ActorClass"))
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Spawn",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::PickupItem"))
    TObjectPtr<UTMOPItemDefinition> ItemDefinition;

    /** Empty uses ATMOPWorldItem. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Spawn",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::PickupItem"))
    TSubclassOf<ATMOPWorldItem> WorldItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Placement",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn"))
    ETMOPTimedPropPlacement Placement = ETMOPTimedPropPlacement::Anchor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Placement",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && Placement==ETMOPTimedPropPlacement::Anchor",
            DisplayName="Anchor ID"))
    FName AnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Placement",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && Placement==ETMOPTimedPropPlacement::WorldTransform"))
    FTransform WorldTransform = FTransform::Identity;

    /** Applied in anchor-local space, or after World Transform. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Placement",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn"))
    FTransform LocalOffset = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Placement",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn"))
    bool bSnapToGround = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Placement",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && bSnapToGround"))
    float GroundOffsetCm = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Static Mesh",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::StaticMesh"))
    bool bEnableCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Static Mesh",
        meta=(EditCondition="Action==ETMOPTimedPropAction::Spawn && PropKind==ETMOPTimedPropKind::StaticMesh"))
    bool bCastShadow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Prop|Source")
    FString Notes;
};

/** Spawns and removes scene meshes, custom actors and pickup items over time. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTimedPropDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTimedPropDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Timed Props",
        meta=(TitleProperty="EntryId"))
    TArray<FTMOPTimedPropEntry> ScheduledEntries;

    UFUNCTION(BlueprintCallable, Category="TMOP|Timed Props")
    void RestartScheduleAtCurrentTime();

    UFUNCTION(BlueprintPure, Category="TMOP|Timed Props")
    AActor* FindSpawnedInstance(FName InstanceId) const;

private:
    void EvaluateSchedule(int32 CurrentSecond, bool bCatchUp);
    bool ResolveEntrySecond(
        const FTMOPTimedPropEntry& Entry, int32& OutSecond) const;
    bool ApplyEntry(const FTMOPTimedPropEntry& Entry);
    bool ResolveSpawnTransform(
        const FTMOPTimedPropEntry& Entry, FTransform& OutTransform) const;
    void DestroyAllSpawnedInstances();

    int32 NextEntryIndex = 0;
    int32 LastResolvedEntrySecond = INDEX_NONE;
    int32 LastEvaluatedSecond = INDEX_NONE;
    TMap<FName, TWeakObjectPtr<AActor>> SpawnedInstances;
};

