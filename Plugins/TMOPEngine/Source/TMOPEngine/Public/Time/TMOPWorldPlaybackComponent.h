#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "Observations/TMOPObservationTypes.h"
#include "Traffic/TMOPTrafficSignalTypes.h"
#include "World/TMOPWorldTypes.h"
#include "TMOPWorldPlaybackComponent.generated.h"

class ATMOPSimulationDebugDirector;
class USceneComponent;

USTRUCT()
struct FTMOPPlaybackAuxiliaryState
{
    GENERATED_BODY()
    UPROPERTY() TArray<FTMOPGroupSnapshot> Groups;
    UPROPERTY() TArray<FTMOPObservationRuntime> Observations;
    UPROPERTY() TArray<FTMOPSignalGroupState> Signals;
};

USTRUCT()
struct FTMOPPlaybackWorldState
{
    GENERATED_BODY()
    UPROPERTY() TMap<FName, FTMOPWorldStateValue> Values;
};

USTRUCT()
struct FTMOPPlaybackCollisionState
{
    GENERATED_BODY()
    UPROPERTY() int32 Mode = 0;
    UPROPERTY() int32 ObjectType = 0;
    UPROPERTY() TArray<uint8> Responses;
};

USTRUCT()
struct FTMOPPlaybackMaterialState
{
    GENERATED_BODY()
    UPROPERTY() FString Parent;
    UPROPERTY() bool bDynamic = false;
    UPROPERTY() TMap<FString, float> Scalars;
    UPROPERTY() TMap<FString, FLinearColor> Vectors;
};

USTRUCT()
struct FTMOPPlaybackMorphState
{
    GENERATED_BODY()
    UPROPERTY() TMap<FString, float> Weights;
};

USTRUCT()
struct FTMOPPlaybackValueKey
{
    GENERATED_BODY()
    UPROPERTY() double Time = 0;
    UPROPERTY() FString Value;
};

USTRUCT()
struct FTMOPPlaybackProperty
{
    GENERATED_BODY()
    UPROPERTY() FName Name;
    UPROPERTY() TArray<FTMOPPlaybackValueKey> Keys;
};

USTRUCT()
struct FTMOPPlaybackPoseKey
{
    GENERATED_BODY()
    UPROPERTY() double Time = 0;
    UPROPERTY() FTransform Transform;
    UPROPERTY() FVector Velocity = FVector::ZeroVector;
    UPROPERTY() bool bPresent = true;
    // No blending through spawn, attachment changes, seats or teleports.
    UPROPERTY() bool bCut = false;
};

USTRUCT()
struct FTMOPPlaybackObjectTrack
{
    GENERATED_BODY()
    UPROPERTY() FString Id;
    UPROPERTY() FString ActorId;
    UPROPERTY() FString ClassPath;
    UPROPERTY() FName ComponentName;
    UPROPERTY() FName EntityId;
    UPROPERTY() bool bPlacedActor = false;
    UPROPERTY() TArray<FTMOPPlaybackPoseKey> Poses;
    UPROPERTY() TArray<FTMOPPlaybackProperty> Properties;
};

USTRUCT()
struct FTMOPPlaybackTape
{
    GENERATED_BODY()
    UPROPERTY() int32 Version = 3;
    UPROPERTY() FString Signature;
    UPROPERTY() int32 StartSecond = 82800;
    UPROPERTY() int32 EndSecond = 85500;
    UPROPERTY() double RecordedThrough = -1;
    UPROPERTY() TArray<FTMOPHistoricalEventRuntime> Events;
    UPROPERTY() TArray<FTMOPPlaybackObjectTrack> Tracks;
    UPROPERTY() TArray<FTMOPPlaybackValueKey> WorldStateKeys;
    UPROPERTY() TSet<FName> WorldStateNames;
};

/** One authority for historical playback, independent of how the clock reached T.
 * Records observable state, so an action is never restarted or replanned on seek.
 * Existing live simulation is used only to AUTHOR a new tape.
 */
UCLASS(ClassGroup=(TMOP))
class TMOPENGINE_API UTMOPWorldPlaybackComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTMOPWorldPlaybackComponent();
    virtual void TickComponent(float Delta, ELevelTick Type, FActorComponentTickFunction* Tick) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    bool StartRecording(const FString& Signature);
    bool FinishRecording(bool bSave);
    bool LoadAndPrepare(const FString& Signature);
    bool ValidateFile(const FString& Signature);
    bool VerifyRepeatability();
    bool BeginScrub(UObject* Owner);
    void CancelScrub(UObject* Owner);
    bool RequestSeek(double RequestedSecond, UObject* Owner = nullptr);
    bool IsReady() const { return bReady; }
    bool IsRecording() const { return bRecording; }
    bool IsBusy() const { return PendingSecond != INDEX_NONE; }
    bool IsScrubbing() const { return ScrubOwner.IsValid(); }
    int32 GetPendingSecond() const { return PendingSecond; }
    FString GetStatus() const { return Status; }
    FString GetTapePath() const;
    static UTMOPWorldPlaybackComponent* Find(const UObject* Context);

private:
    void Capture(double Time);
    void CaptureObject(UObject* Object, const FString& Id, const FString& ActorId,
        FName EntityId, bool bPlaced, double Time, TSet<FString>& Seen);
    void RecordValue(FTMOPPlaybackObjectTrack& Track, FName Name, const FString& Value, double Time);
    bool Validate(FString& Error) const;
    bool PrepareObjects();
    bool Evaluate(double Time, bool bSeeking);
    void Quiesce(AActor* Actor);
    void StopLiveDirectors();
    bool ApplyValue(UObject* Object, FName Name, const FString& Value, double Time, bool bSeeking);
    FString ActorId(AActor* Actor) const;
    FString HashCurrentWorld() const;
    ATMOPSimulationDebugDirector* Director() const;

    UPROPERTY(Transient) FTMOPPlaybackTape Tape;
    UPROPERTY(Transient) TMap<FString, TObjectPtr<UObject>> Objects;
    UPROPERTY(Transient) TMap<FString, TObjectPtr<UObject>> Assets;
    TMap<FString, int32> TrackIndices;
    TArray<TArray<int32>> AppliedPropertyKeys;
    int32 AppliedWorldStateKey = INDEX_NONE;
    TWeakObjectPtr<UObject> ScrubOwner;
    TMap<TWeakObjectPtr<AActor>, bool> AuthoringPlayerCollisions;
    bool bReady = false;
    bool bVerifying = false;
    bool bPreparationAttempted = false;
    uint32 TapeChecksum = 0;
    bool bOwnsScrubPause = false;
    bool bRecording = false;
    bool bOldFixedStep = false;
    double OldFixedDelta = 0;
    double LastEvaluatedTime = -1;
    int32 PendingSecond = INDEX_NONE;
    int32 RestoreDelayFrames = 0;
    FString Status;
};
