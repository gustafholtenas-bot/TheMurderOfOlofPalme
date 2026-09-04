#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPersonRegistryDirector.generated.h"

class ATMOPHistoricalAgent;
class ATMOPVehicleBase;
class ATMOPGroupDirector;
class ATMOPTimelineValidationDirector;
class UDataTable;
class UCurveFloat;
struct FTMOPGroupProfileRow;

DECLARE_MULTICAST_DELEGATE_FiveParams(
    FTMOPPersonTimelineAppliedNativeSignature,
    FName,
    const FTMOPPersonTimelineEntry&,
    int32,
    bool,
    bool);

/** Configures the central person DataTable when the level starts. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPersonRegistryDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPPersonRegistryDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People")
    TObjectPtr<UDataTable> PersonProfileTable;

    /** Optional modular body/clothing catalog. Row struct: FTMOPAppearanceAssetRow. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Appearance")
    TObjectPtr<UDataTable> AppearanceAssetTable;

    /** Authoritative editable group list. Row struct: FTMOPGroupProfileRow. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Groups")
    TObjectPtr<UDataTable> GroupDefinitionTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    TSubclassOf<ATMOPHistoricalAgent> DefaultAgentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    bool bSpawnPeopleAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    bool bCatchUpToCurrentClockOnBeginPlay = true;

    /** Arrival-timed walkers may temporarily exceed their normal sprint speed
     * by this factor after delays. The deadline correction remains exact. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|People|Simulation|Timeline Precision",
        meta=(ClampMin="1.0", ClampMax="3.0"))
    float TimelineCatchUpSpeedMultiplier = 1.35f;

    /**
     * ObservedUnknown people are evidence markers rather than authoritative
     * traffic participants.  Keeping their collision disabled prevents a
     * speculative person from blocking historical cars or emergency vehicles.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    bool bDisableCollisionForObservedUnknownPeople = true;

    /** Central defaults copied to every person spawned by this director. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade")
    bool bEnablePersonSpawnFade = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade")
    bool bEnablePersonDespawnFade = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade",
        meta=(ClampMin="0.0", ClampMax="10.0", Units="s"))
    float PersonSpawnFadeDurationSeconds = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade",
        meta=(ClampMin="0.0", ClampMax="10.0", Units="s"))
    float PersonDespawnFadeDurationSeconds = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade")
    TObjectPtr<UCurveFloat> PersonVisibilityFadeCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade")
    FName PersonVisibilityFadeMaterialParameter = TEXT("TMOP_VisibilityFade");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade")
    bool bWritePersonFadeToCustomPrimitiveData = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade",
        meta=(EditCondition="bWritePersonFadeToCustomPrimitiveData", ClampMin="0", ClampMax="31"))
    int32 PersonFadeCustomPrimitiveDataIndex = 0;

    /** Enable only for legacy materials that cannot read Custom Primitive Data. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Spawn Fade")
    bool bWritePersonFadeMaterialParameter = false;

    /** Automatically validates every spawned person's real timeline execution. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Validation")
    bool bEnableAutomaticTimelineValidation = true;

    /** Prefer DT_TMOP_Groups when a valid GroupDefinitionTable is assigned. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Groups")
    bool bCreateGroupsFromGroupTable = true;

    /** Legacy fallback used only when no valid group table is assigned. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Groups")
    bool bCreateGroupsFromPeopleTable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    FTMOPTime SimulationEpoch = FTMOPTime(23, 0, 0);

    /** Recovers historical NPCs and the player if they fall below the map. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World|Fall Safety")
    bool bEnableWorldFallSafety = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World|Fall Safety",
        meta=(Units="cm"))
    float FallRecoveryTriggerZ = -3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World|Fall Safety",
        meta=(ClampMin="0.0", Units="cm"))
    float FallRecoveryHeightOffsetCm = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World|Fall Safety",
        meta=(ClampMin="0.05", Units="s"))
    float FallSafetySampleIntervalSeconds = 0.25f;

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    int32 RefreshAllActiveProfiles();

    UFUNCTION(BlueprintCallable, Category="TMOP|People|Simulation")
    int32 InitializePersonSimulation();

    /**
     * Advances timeline indices/spawn state for a World Bake seek without
     * applying catch-up teleports. The baked frame supplies final transforms.
     */
    UFUNCTION(BlueprintCallable, Category="TMOP|People|World Bake")
    int32 InitializePersonSimulationForWorldBake();

    UFUNCTION(BlueprintCallable, Category="TMOP|People|Simulation")
    bool ValidatePeopleTable(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|People|Appearance")
    bool ValidateAppearanceAssetTable(TArray<FString>& OutErrors) const;

    /** Details-panel wrapper for validating the configured appearance catalog. */
    UFUNCTION(CallInEditor, Category="TMOP|People|Appearance",
        meta=(DisplayName="Validate Appearance Asset Table"))
    void ValidateAppearanceAssetTableInEditor();

    UFUNCTION(BlueprintCallable, Category="TMOP|People|Groups")
    bool ValidateGroupTable(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category="TMOP|People|Simulation")
    ATMOPHistoricalAgent* FindSpawnedPerson(FName EntityId) const;

    /** Returns the configured talk response for the selected side of the shot. */
    UFUNCTION(BlueprintPure, Category="TMOP|People|Dialog")
    FText GetPersonDialog(FName EntityId, bool bAfterShot) const;

    /** Telemetry for all consumed entries, including group and vehicle actions. */
    FTMOPPersonTimelineAppliedNativeSignature OnTimelineEntryApplied;

private:
    struct FPersonRuntime
    {
        FName RowName = NAME_None;
        FTMOPPersonProfileRow Profile;
        TWeakObjectPtr<ATMOPHistoricalAgent> Agent;
        int32 NextTimelineIndex = 0;
        int32 NextAutomaticSpeechIndex = 0;
        int32 LastResolvedAutomaticSpeechSecond = INDEX_NONE;
        bool bSpawnedByDirector = false;
        bool bCompleted = false;
        /** Physical execution/departure time used by EvaluatePeople. */
        int32 CachedResolvedSecond = INDEX_NONE;
        /** Logical timestamp shown in the editor (arrival for ARRIVAL moves). */
        int32 CachedTimelineSecond = INDEX_NONE;
        /** Logical timestamp of the preceding consumed array entry. */
        int32 LastResolvedTimelineSecond = INDEX_NONE;
    };

    void EvaluatePeople(int32 CurrentSecond, bool bCatchUp);
    void EvaluateAutomaticSpeech(int32 CurrentSecond, int32 PreviousSecond);
    bool ResolveAutomaticSpeechSecond(
        const FPersonRuntime& Runtime,
        const FTMOPTimedSpeechLine& Line,
        int32& OutSecond) const;
    bool SpawnPerson(FPersonRuntime& Runtime, const FTMOPPersonTimelineEntry& InitialEntry);
    bool ApplyTimelineEntry(FPersonRuntime& Runtime,
        const FTMOPPersonTimelineEntry& Entry, bool bCatchUp);
    bool ApplyPlacement(ATMOPHistoricalAgent* Agent,
        const FTMOPPersonTimelineEntry& Entry, bool bCatchUp);
    void ApplyConversationFocus(
        ATMOPHistoricalAgent* Speaker,
        const FTMOPPersonTimelineEntry& Entry);
    void ClearConversationFocus(ATMOPHistoricalAgent* Speaker);
    bool IsAgentBusy(const ATMOPHistoricalAgent* Agent) const;
    bool ResolveEntrySecond(FPersonRuntime& Runtime,
        const FTMOPPersonTimelineEntry& Entry, int32& OutSecond) const;
    int32 EstimateTravelSeconds(const FPersonRuntime& Runtime,
        const FTMOPPersonTimelineEntry& Entry) const;
    ATMOPGroupDirector* FindGroupDirector() const;
    bool HasValidGroupTable() const;
    const FTMOPGroupProfileRow* FindGroupRow(FName GroupId) const;
    bool IsGroupLeader(const FTMOPPersonProfileRow& Profile,
        const ATMOPHistoricalAgent* Agent) const;
    bool ShouldFollowGroupLeader(const FTMOPPersonProfileRow& Profile,
        const ATMOPHistoricalAgent* Agent) const;
    void ApplyGroupTableMemberships();
    void RebuildGroupsFromGroupTable();
    void RebuildGroupsFromPeople();
    ATMOPVehicleBase* FindVehicle(FName VehicleId) const;
    void UpdateWorldFallSafety(float DeltaSeconds);

    bool bRestoringWorldBake = false;

    TMap<FName, FPersonRuntime> RuntimePeople;
    TMap<TWeakObjectPtr<class APawn>, FTransform> LastSafePawnTransforms;
    float FallSafetyAccumulator = 0.0f;
    int32 LastEvaluatedSecond = INDEX_NONE;
};
