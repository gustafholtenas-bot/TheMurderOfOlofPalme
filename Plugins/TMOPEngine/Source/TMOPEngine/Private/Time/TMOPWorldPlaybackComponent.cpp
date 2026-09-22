#include "Time/TMOPWorldPlaybackComponent.h"
#include "Engine/GameInstance.h"
#include "Serialization/JsonWriter.h"
#include "HAL/PlatformTime.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/MorphTarget.h"
#include "World/TMOPWorldSubsystem.h"
#include "World/TMOPVerticalTransport.h"
#include "Traffic/TMOPTrafficSignalController.h"
#include "Components/TextRenderComponent.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/Crc.h"
#include "Engine/LatentActionManager.h"
#include "Testing/TMOPTimelineValidationDirector.h"
#include "Misc/SecureHash.h"
#include "Components/AudioComponent.h"
#include "JsonObjectConverter.h"
#include "Observations/TMOPObservationDirector.h"
#include "Schedules/TMOPScheduleSubsystem.h"
#include "Venues/TMOPGrandSimulationDirector.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "RecordedCalls/TMOPRecordedCallDirector.h"
#include "Time/TMOPTimeTravelPolicy.h"
#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationDebugDirector.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Groups/TMOPGroupDirector.h"
#include "World/TMOPTimedPropDirector.h"
#include "World/TMOPAerialVehicleDirector.h"
#include "World/TMOPLightScheduleDirector.h"
#include "Events/TMOPPalmeShotDirector.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Transit/TMOPBusScheduleDirector.h"
#include "Radio/TMOPPlayerRadioComponent.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimationAsset.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

namespace
{
const FName SeekPause(TEXT("HistoricalSeek"));
const FName MissingTapePause(TEXT("HistoricalBakeRequired"));

template<class T> int32 Floor(const TArray<T>& Keys, double Time)
{
    return TMOPTimeTravel::FloorKeyIndex(Keys.Num(), [&Keys](int I) { return Keys[I].Time; }, Time);
}

// Runtime scalar intent is data. UObject pointers, delegates and private engine
// state are deliberately excluded; scene/asset/attachment channels have adapters.
bool RecordProperty(const FProperty* P)
{
    const UClass* Owner = P->GetOwnerClass();
    if (!Owner || !Owner->GetName().StartsWith(TEXT("TMOP"))) return false;
    if (P->GetName().StartsWith(TEXT("NameLabel")) || P->GetFName() == TEXT("bShowNameLabel")) return false;
    if (!P->HasAnyPropertyFlags(CPF_BlueprintVisible | CPF_SaveGame)) return false;
    return CastField<FNumericProperty>(P) || CastField<FBoolProperty>(P) ||
        CastField<FEnumProperty>(P) || CastField<FNameProperty>(P) ||
        CastField<FStrProperty>(P) || CastField<FTextProperty>(P);
}

bool IsDirector(AActor* A)
{
    return A->IsA<ATMOPPersonRegistryDirector>() || A->IsA<ATMOPHistoricalVehicleDirector>() ||
        A->IsA<ATMOPGroupDirector>() || A->IsA<ATMOPBusScheduleDirector>() ||
        A->IsA<ATMOPTimedPropDirector>() || A->IsA<ATMOPAerialVehicleDirector>() ||
        A->IsA<ATMOPLightScheduleDirector>() || A->IsA<ATMOPPalmeShotDirector>() ||
        A->IsA<ATMOPObservationDirector>() || A->IsA<ATMOPGrandSimulationDirector>() ||
        A->IsA<ATMOPTimelineValidationDirector>();
}

bool HasHistoricalOwner(AActor* A)
{
    return A->GetOwner() && (A->GetOwner()->IsA<ATMOPTimedPropDirector>() ||
        A->GetOwner()->IsA<ATMOPAerialVehicleDirector>());
}
}

UTMOPWorldPlaybackComponent::UTMOPWorldPlaybackComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

ATMOPSimulationDebugDirector* UTMOPWorldPlaybackComponent::Director() const
{
    return Cast<ATMOPSimulationDebugDirector>(GetOwner());
}

UTMOPWorldPlaybackComponent* UTMOPWorldPlaybackComponent::Find(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    if (World) for (TActorIterator<ATMOPSimulationDebugDirector> It(World); It; ++It)
        return It->WorldPlayback;
    return nullptr;
}

FString UTMOPWorldPlaybackComponent::GetTapePath() const
{
    return Director() ? FPaths::ChangeExtension(Director()->GetResolvedBakePath(), TEXT("tmopreplay")) : FString();
}

FString UTMOPWorldPlaybackComponent::ActorId(AActor* Actor) const
{
    if (!Actor) return FString();
    for (FName Tag : Actor->Tags)
        if (Tag.ToString().StartsWith(TEXT("TMOP_HistoryId="))) return Tag.ToString().Mid(15);
    if (const auto* Agent = Cast<ATMOPHistoricalAgent>(Actor))
        if (Agent->EntityIdentity && !Agent->EntityIdentity->EntityId.IsNone())
            return TEXT("person:") + Agent->EntityIdentity->EntityId.ToString();
    if (const auto* Vehicle = Cast<ATMOPVehicleBase>(Actor))
        if (!Vehicle->VehicleId.IsNone()) return TEXT("vehicle:") + Vehicle->VehicleId.ToString();
    return TEXT("actor:") + Actor->GetName();
}

void UTMOPWorldPlaybackComponent::RecordValue(FTMOPPlaybackObjectTrack& Track,
    FName Name, const FString& Value, double Time)
{
    auto* Property = Track.Properties.FindByPredicate([Name](const auto& P) { return P.Name == Name; });
    if (!Property)
    {
        FTMOPPlaybackProperty New;
        New.Name = Name;
        Property = &Track.Properties.Add_GetRef(MoveTemp(New));
    }
    if (Property->Keys.IsEmpty() || Property->Keys.Last().Value != Value)
    {
        FTMOPPlaybackValueKey Key;
        Key.Time = Time;
        Key.Value = Value;
        Property->Keys.Add(MoveTemp(Key));
    }
}

void UTMOPWorldPlaybackComponent::CaptureObject(UObject* Object, const FString& Id,
    const FString& ParentActorId, FName EntityId, bool bPlaced, double Time, TSet<FString>& Seen)
{
    if (!IsValid(Object)) return;
    Seen.Add(Id);
    int32* Existing = TrackIndices.Find(Id);
    if (!Existing)
    {
        FTMOPPlaybackObjectTrack Track;
        Track.Id = Id;
        Track.ActorId = ParentActorId;
        Track.ClassPath = Object->GetClass()->GetPathName();
        Track.EntityId = EntityId;
        Track.bPlacedActor = bPlaced;
        if (Object->IsA<UActorComponent>()) Track.ComponentName = Object->GetFName();
        const int32 Index = Tape.Tracks.Add(MoveTemp(Track));
        TrackIndices.Add(Id, Index);
        Existing = TrackIndices.Find(Id);
    }
    auto& Track = Tape.Tracks[*Existing];
    auto Value = [&](const TCHAR* Name, const FString& V) { RecordValue(Track, FName(Name), V, Time); };
    for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
        if (RecordProperty(*It))
        {
            FString Text;
            It->ExportText_InContainer(0, Text, Object, Object, Object, PPF_None);
            RecordValue(Track, It->GetFName(), Text, Time);
        }

    if (auto* Action = Cast<UTMOPActionExecutorComponent>(Object))
    {
        auto ActionState = Action->CapturePlaybackAction();
        Value(TEXT("$actionRemainingPath"), FString::SanitizeFloat(ActionState.RemainingPath));
        Value(TEXT("$actionRequiredSpeed"), FString::SanitizeFloat(ActionState.RequiredSpeed));
        // Dense numeric diagnostics must not duplicate the full schedule entry every frame.
        ActionState.RemainingPath = 0; ActionState.RequiredSpeed = 0;
        FString Json; FJsonObjectConverter::UStructToJsonObjectString(ActionState, Json);
        Value(TEXT("$action"), Json);
    }
    FTMOPPlaybackPoseKey Pose;
    Pose.Time = Time;
    if (auto* Actor = Cast<AActor>(Object))
    {
        Pose.Transform = Actor->GetActorTransform();
        Pose.Velocity = Actor->GetVelocity();
        Value(TEXT("$hidden"), Actor->IsHidden() ? TEXT("1") : TEXT("0"));
        Value(TEXT("$collision"), Actor->GetActorEnableCollision() ? TEXT("1") : TEXT("0"));
        if (auto* Agent = Cast<ATMOPHistoricalAgent>(Actor))
        {
            Value(TEXT("$speech"), Agent->CapturePlaybackSpeech());
            Value(TEXT("$uniqueAsset"), Agent->PlaybackUniqueAnimationAsset);
            Value(TEXT("$fade"), FString::SanitizeFloat(Agent->GetPlaybackFade()));
            if (auto* Movement = Agent->GetCharacterMovement())
            {
                Value(TEXT("$movementMode"), FString::FromInt(int32(Movement->MovementMode)));
                Value(TEXT("$customMovementMode"), FString::FromInt(Movement->CustomMovementMode));
                Value(TEXT("$maxWalkSpeed"), FString::SanitizeFloat(Movement->MaxWalkSpeed));
            }
        }
        FTMOPPlaybackAuxiliaryState Aux;
        if (auto* Groups = Cast<ATMOPGroupDirector>(Actor))
        {
            Aux.Groups = Groups->GetAllGroupSnapshots();
            for (auto& G : Aux.Groups)
                if (G.RemainingConversationSeconds > 0)
                    G.RemainingConversationSeconds = std::round((Time + G.RemainingConversationSeconds) * 10) / 10;
        }
        if (auto* Observations = Cast<ATMOPObservationDirector>(Actor)) Aux.Observations = Observations->GetAllObservationRuntime();
        if (auto* Signals = Cast<ATMOPTrafficSignalController>(Actor)) Aux.Signals = Signals->CapturePlaybackSignals();
        if (Actor->IsA<ATMOPGroupDirector>() || Actor->IsA<ATMOPObservationDirector>() || Actor->IsA<ATMOPTrafficSignalController>())
        {
            FString Json;
            FJsonObjectConverter::UStructToJsonObjectString(Aux, Json);
            Value(TEXT("$aux"), Json);
        }
        if (auto* Root = Actor->GetRootComponent())
        {
            auto* AttachParent = Root->GetAttachParent();
            Value(TEXT("$parent"), AttachParent ? ActorId(AttachParent->GetOwner()) + TEXT("/") + AttachParent->GetName() : TEXT(""));
            Value(TEXT("$socket"), Root->GetAttachSocketName().ToString());
        }
    }
    else if (auto* Scene = Cast<USceneComponent>(Object))
    {
        Pose.Transform = Scene->GetRelativeTransform();
        Value(TEXT("$visible"), Scene->IsVisible() ? TEXT("1") : TEXT("0"));
        Value(TEXT("$componentHidden"), Scene->bHiddenInGame ? TEXT("1") : TEXT("0"));
        auto* AttachParent = Scene->GetAttachParent();
        Value(TEXT("$parent"), AttachParent ? ActorId(AttachParent->GetOwner()) + TEXT("/") + AttachParent->GetName() : TEXT(""));
        Value(TEXT("$socket"), Scene->GetAttachSocketName().ToString());
        if (auto* Primitive = Cast<UPrimitiveComponent>(Scene))
        {
            FTMOPPlaybackCollisionState State;
            State.Mode = int32(Primitive->GetCollisionEnabled()); State.ObjectType = int32(Primitive->GetCollisionObjectType());
            for (int32 C=0; C<ECC_MAX; ++C) State.Responses.Add(uint8(Primitive->GetCollisionResponseToChannel(ECollisionChannel(C))));
            FString Json; FJsonObjectConverter::UStructToJsonObjectString(State, Json); Value(TEXT("$primitiveCollision"), Json);
        }
        if (auto* Seat = Cast<UTMOPVehicleSeatComponent>(Scene))
            Value(TEXT("$occupant"), ActorId(Seat->GetOccupant()));
        if (auto* Seat = Cast<UTMOPCinemaSeatComponent>(Scene)) Value(TEXT("$cinemaOccupant"), ActorId(Seat->GetOccupyingAgent()));
        if (auto* Static = Cast<UStaticMeshComponent>(Scene))
            Value(TEXT("$staticMesh"), GetPathNameSafe(Static->GetStaticMesh()));
        if (auto* Mesh = Cast<UMeshComponent>(Scene))
            for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
            {
                UMaterialInterface* Material = Mesh->GetMaterial(MaterialIndex);
                if (auto* Dynamic = Cast<UMaterialInstanceDynamic>(Material))
                {
                    FTMOPPlaybackMaterialState State;
                    State.bDynamic = true;
                    UMaterialInterface* ParentMaterial = Dynamic->Parent;
                    while (auto* ParentDynamic = Cast<UMaterialInstanceDynamic>(ParentMaterial)) ParentMaterial = ParentDynamic->Parent;
                    State.Parent = GetPathNameSafe(ParentMaterial);
                    TArray<FMaterialParameterInfo> Infos; TArray<FGuid> Ids;
                    Dynamic->GetAllScalarParameterInfo(Infos, Ids);
                    for (const auto& Info : Infos)
                    {
                        float Number = 0;
                        if (Dynamic->GetScalarParameterValue(FHashedMaterialParameterInfo(Info), Number, false)) State.Scalars.Add(Info.Name.ToString(), Number);
                    }
                    Infos.Reset(); Ids.Reset(); Dynamic->GetAllVectorParameterInfo(Infos, Ids);
                    for (const auto& Info : Infos)
                    {
                        FLinearColor Color;
                        if (Dynamic->GetVectorParameterValue(FHashedMaterialParameterInfo(Info), Color, false)) State.Vectors.Add(Info.Name.ToString(), Color);
                    }
                    FString Json; FJsonObjectConverter::UStructToJsonObjectString(State, Json);
                    RecordValue(Track, FName(*FString::Printf(TEXT("$materialState_%d"), MaterialIndex)), Json, Time);
                }
                else if (Material && !Material->HasAnyFlags(RF_Transient) && Material->IsAsset())
                {
                    FTMOPPlaybackMaterialState State; State.Parent = Material->GetPathName();
                    FString Json; FJsonObjectConverter::UStructToJsonObjectString(State, Json);
                    RecordValue(Track, FName(*FString::Printf(TEXT("$materialState_%d"), MaterialIndex)), Json, Time);
                }
            }
        if (auto* Mesh = Cast<USkeletalMeshComponent>(Scene))
        {
            Value(TEXT("$skeletalMesh"), GetPathNameSafe(Mesh->GetSkeletalMeshAsset()));
            FTMOPPlaybackMorphState Morphs;
            if (auto* Asset = Mesh->GetSkeletalMeshAsset())
                for (const TObjectPtr<UMorphTarget>& MorphPtr : Asset->GetMorphTargets())
                    if (UMorphTarget* Morph = MorphPtr.Get())
                        Morphs.Weights.Add(Morph->GetName(), Mesh->GetMorphTarget(Morph->GetFName()));
            FString MorphJson; FJsonObjectConverter::UStructToJsonObjectString(Morphs, MorphJson);
            Value(TEXT("$morphs"), MorphJson);
            Value(TEXT("$animClass"), GetPathNameSafe(Mesh->GetAnimClass()));
            Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            Mesh->bEnableUpdateRateOptimizations = false;
            auto* Single = Mesh->GetSingleNodeInstance();
            Value(TEXT("$singleAsset"), Single ? GetPathNameSafe(Single->GetCurrentAsset()) : TEXT("None"));
            if (Single) Value(TEXT("$singleTime"), FString::SanitizeFloat(Single->GetCurrentTime()));
        }
        if (auto* Light = Cast<ULightComponent>(Scene))
        {
            Value(TEXT("$intensity"), FString::SanitizeFloat(Light->Intensity));
            Value(TEXT("$lightColor"), Light->GetLightColor().ToString());
        }
    }
    const bool bAttachmentChanged = Track.Properties.ContainsByPredicate([Time](const auto& P)
    {
        return (P.Name == TEXT("$parent") || P.Name == TEXT("$socket")) &&
            P.Keys.Num() > 1 && P.Keys.Last().Time == Time;
    });
    Pose.bCut = bAttachmentChanged;
    // Stationary transforms take two keys, not one key per simulation tick.
    // Moving keys stay dense: do not simplify across corners or action changes.
    if (Track.Poses.Num() > 1)
    {
        auto& Last = Track.Poses.Last();
        const auto& Before = Track.Poses[Track.Poses.Num() - 2];
        if (!Pose.bCut && !Last.bCut && Last.bPresent && Before.bPresent && Last.Transform.Equals(Pose.Transform, 0.001f) &&
            Before.Transform.Equals(Pose.Transform, 0.001f) &&
            Last.Velocity.Equals(Pose.Velocity, 0.001f) && Before.Velocity.Equals(Pose.Velocity, 0.001f))
        {
            Last.Time = Time;
            return;
        }
    }
    if (!Track.Poses.IsEmpty())
    {
        const auto& Last = Track.Poses.Last();
        // A teleport must stay a discontinuity. Ordinary recorded motion at 20Hz
        // is interpolated, including after a seek; no navigation is restarted.
        const double MaxTravel = FMath::Max(50.0, Last.Velocity.Size() * (Time - Last.Time) * 2.0 + 10.0);
        Pose.bCut = Pose.bCut || !Last.bPresent || FVector::Dist(Last.Transform.GetLocation(), Pose.Transform.GetLocation()) > MaxTravel;
    }
    Track.Poses.Add(Pose);
}

void UTMOPWorldPlaybackComponent::Capture(double Time)
{
    TSet<FString> Seen;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor) || Actor->IsA<ATMOPPlayerCharacter>()) continue;
        const bool bPerson = Actor->IsA<ATMOPHistoricalAgent>();
        const bool bVehicle = Actor->IsA<ATMOPVehicleBase>();
        TArray<ULightComponent*> Lights;
        Actor->GetComponents(Lights);
        TArray<UTMOPCinemaSeatComponent*> CinemaSeats;
        Actor->GetComponents(CinemaSeats);
        if (!bPerson && !bVehicle && !HasHistoricalOwner(Actor) && Lights.IsEmpty() && CinemaSeats.IsEmpty() &&
            !Actor->IsA<ATMOPPalmeShotDirector>() && !Actor->IsA<ATMOPGroupDirector>() &&
            !Actor->IsA<ATMOPObservationDirector>() && !Actor->IsA<ATMOPTrafficSignalController>() &&
            !Actor->IsA<ATMOPVerticalTransport>() && !Actor->Tags.Contains(TEXT("TMOP_RecordHistory"))) continue;
        const FString Id = ActorId(Actor);
        FName Entity;
        if (auto* Agent = Cast<ATMOPHistoricalAgent>(Actor)) Entity = Agent->EntityIdentity->EntityId;
        if (auto* Vehicle = Cast<ATMOPVehicleBase>(Actor)) Entity = Vehicle->VehicleId;
        const bool bPlaced = !bPerson && !bVehicle && !HasHistoricalOwner(Actor);
        CaptureObject(Actor, Id, Id, Entity, bPlaced, Time, Seen);
        TArray<UActorComponent*> Components;
        Actor->GetComponents(Components);
        for (auto* Component : Components)
        {
            // Widgets/audio are presented by their dedicated clock-aware adapters.
            if (Component->IsA<UWidgetComponent>() || Component->IsA<UAudioComponent>() || Component->IsA<UTextRenderComponent>()) continue;
            if (Component->IsA<USceneComponent>() || Component->GetClass()->GetName().StartsWith(TEXT("TMOP")))
                CaptureObject(Component, Id + TEXT("/") + Component->GetName(), Id, Entity, bPlaced, Time, Seen);
        }
    }
    for (auto& Track : Tape.Tracks)
        if (!Seen.Contains(Track.Id) && !Track.Poses.IsEmpty() && Track.Poses.Last().bPresent)
        {
            auto End = Track.Poses.Last();
            End.Time = Time;
            End.bPresent = false;
            End.bCut = true;
            Track.Poses.Add(End);
        }
    if (auto* WorldState = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
    {
        FTMOPPlaybackWorldState Snapshot; Snapshot.Values = WorldState->CaptureHistoricalState();
        for (const auto& Pair : Snapshot.Values) Tape.WorldStateNames.Add(Pair.Key);
        FString Json; FJsonObjectConverter::UStructToJsonObjectString(Snapshot, Json);
        if (Tape.WorldStateKeys.IsEmpty() || Tape.WorldStateKeys.Last().Value != Json)
        {
            FTMOPPlaybackValueKey Key; Key.Time = Time; Key.Value = Json; Tape.WorldStateKeys.Add(Key);
        }
    }
    Tape.RecordedThrough = Time;
}

bool UTMOPWorldPlaybackComponent::StartRecording(const FString& Signature)
{
    if (bRecording || bReady || !GetWorld())
    {
        Status = TEXT("Starta en ny Play-session med Bake Entire Simulation för att skapa en ny bake.");
        return false;
    }
    auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    Tape = FTMOPPlaybackTape();
    Tape.Signature = Signature;
    Tape.StartSecond = Clock->GetLoopStartTime().ToSecondsFromMidnight();
    Tape.EndSecond = Clock->GetLoopEndTime().ToSecondsFromMidnight();
    TrackIndices.Reset();
    if (auto* Events = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        for (FName Id : Events->GetRegisteredEventIds())
        {
            FTMOPHistoricalEventRuntime Event;
            if (Events->TryGetEventRuntime(Id, Event)) Tape.Events.Add(Event);
        }
    bOldFixedStep = FApp::UseFixedTimeStep();
    OldFixedDelta = FApp::GetFixedDeltaTime();
    FApp::SetFixedDeltaTime(TMOPTimeTravel::RecordingStepSeconds);
    FApp::SetUseFixedTimeStep(true);
    Clock->bRecordingAuthoritativeBake = true;
    Clock->SetTimeScale(1.0f);
    Clock->ReleasePause(this, MissingTapePause);
    AuthoringPlayerCollisions.Reset();
    for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
    {
        AuthoringPlayerCollisions.Add(Player, Player->GetActorEnableCollision());
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorEnableCollision(false);
    }
    bRecording = true;
    Capture(Tape.StartSecond);
    Status = TEXT("Spelar in historiskt förlopp (20 Hz).");
    return true;
}

bool UTMOPWorldPlaybackComponent::Validate(FString& Error) const
{
    if (Tape.Version != TMOPTimeTravel::FormatVersion || Tape.Tracks.IsEmpty() ||
        Tape.StartSecond >= Tape.EndSecond || !FMath::IsFinite(Tape.RecordedThrough) || Tape.RecordedThrough < Tape.EndSecond || Tape.WorldStateKeys.IsEmpty())
    { Error = TEXT("Baken måste vara version 3 och täcka hela scenariot, inklusive sluttiden."); return false; }
    double WorldPrevious = -1;
    for (const auto& Key : Tape.WorldStateKeys)
    {
        if (!FMath::IsFinite(Key.Time) || Key.Time <= WorldPrevious || Key.Time < Tape.StartSecond || Key.Time > Tape.EndSecond)
        { Error = TEXT("Ogiltig tidsordning för världsflaggor."); return false; }
        WorldPrevious = Key.Time;
    }
    TSet<FString> Ids;
    for (const auto& Track : Tape.Tracks)
    {
        if (Track.Id.IsEmpty() || Ids.Contains(Track.Id) || Track.Poses.IsEmpty() || Track.ClassPath.IsEmpty() || (Track.ComponentName.IsNone() && Track.Id != Track.ActorId))
        { Error = TEXT("Baken innehåller saknade eller dubbla objektspår."); return false; }
        Ids.Add(Track.Id);
        double Previous = -1;
        for (const auto& Key : Track.Poses)
        {
            if (!FMath::IsFinite(Key.Time) || Key.Time < Tape.StartSecond || Key.Time > Tape.EndSecond + 0.000001 || Key.Time <= Previous || Key.Transform.ContainsNaN() || Key.Velocity.ContainsNaN())
            { Error = TEXT("Ogiltiga rörelsedata eller tidsordning i baken."); return false; }
            Previous = Key.Time;
        }
        TSet<FName> PropertyNames;
        for (const auto& Property : Track.Properties)
        {
            if (Property.Name.IsNone() || PropertyNames.Contains(Property.Name) || Property.Keys.IsEmpty())
            { Error = TEXT("Ogiltig eller dubblerad tillståndskanal."); return false; }
            PropertyNames.Add(Property.Name);
            Previous = -1;
            for (const auto& Key : Property.Keys)
            {
                if (!FMath::IsFinite(Key.Time) || Key.Time < Track.Poses[0].Time || Key.Time > Tape.EndSecond + 0.000001 || Key.Time <= Previous)
                { Error = TEXT("Ogiltig tidsordning för ett tillstånd i baken."); return false; }
                Previous = Key.Time;
            }
        }
    }
    for (const auto& Track : Tape.Tracks)
        if (!Ids.Contains(Track.ActorId))
        { Error = TEXT("Ett komponentspår saknar sitt ägarobjekt."); return false; }
    return true;
}

bool UTMOPWorldPlaybackComponent::FinishRecording(bool bSave)
{
    if (!bRecording) return false;
    bRecording = false;
    for (const auto& Pair : AuthoringPlayerCollisions) if (Pair.Key.IsValid()) Pair.Key->SetActorEnableCollision(Pair.Value);
    AuthoringPlayerCollisions.Reset();
    FApp::SetUseFixedTimeStep(bOldFixedStep);
    FApp::SetFixedDeltaTime(OldFixedDelta);
    if (auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        Clock->bRecordingAuthoritativeBake = false;
        Clock->bAuthoritativePlayback = true;
        Clock->RequestPause(this, MissingTapePause);
    }
    if (!bSave) return true;
    Tape.Events.Reset();
    if (auto* Events = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        for (FName Id : Events->GetRegisteredEventIds())
        {
            FTMOPHistoricalEventRuntime Event;
            if (Events->TryGetEventRuntime(Id, Event)) Tape.Events.Add(Event);
        }
    for (TActorIterator<ATMOPTimelineValidationDirector> It(GetWorld()); It; ++It)
        for (const auto& R : It->Records)
            if (R.Severity == ETMOPTimelineValidationSeverity::Error)
            {
                Status = TEXT("Baken avvisades: tidslinjevalideringen innehåller fel. Se valideringsrapporten.");
                UE_LOG(LogTemp, Error, TEXT("TMOP: %s First error: %s / %s: %s"), *Status, *R.EntityId.ToString(), *R.EntryId.ToString(), *R.Message);
                return false;
            }
    if (!Validate(Status)) return false;
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes, true);
    FObjectAndNameAsStringProxyArchive Archive(Writer, false);
    FTMOPPlaybackTape::StaticStruct()->SerializeItem(Archive, &Tape, nullptr);
    if (Archive.IsError()) { Status = TEXT("Kunde inte serialisera baken."); return false; }
    TArray<uint8> FileBytes;
    FMemoryWriter FileWriter(FileBytes, true);
    uint32 Magic = 0x544D5033;
    int64 Size = Bytes.Num();
    uint32 Checksum = FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());
    FileWriter << Magic << Size << Checksum;
    FileWriter.Serialize(Bytes.GetData(), Bytes.Num());
    const FString Path = GetTapePath();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
    if (!FFileHelper::SaveArrayToFile(FileBytes, *(Path + TEXT(".tmp"))) ||
        !IFileManager::Get().Move(*Path, *(Path + TEXT(".tmp")), true))
    { Status = TEXT("Kunde inte spara baken. Tidigare fil är kvar."); return false; }
    Status = TEXT("Bake sparad. Starta en ny Play-session för historisk uppspelning.");
    UE_LOG(LogTemp, Display, TEXT("TMOP: %s (%s, %lld bytes)"), *Status, *Path, int64(Bytes.Num()));
    return true;
}

void UTMOPWorldPlaybackComponent::Quiesce(AActor* Actor)
{
    Actor->Tags.AddUnique(TEXT("TMOP_AuthoritativeHistory"));
    Actor->SetActorTickEnabled(false);
    Actor->SetCanBeDamaged(false);
    GetWorld()->GetTimerManager().ClearAllTimersForObject(Actor);
    GetWorld()->GetLatentActionManager().RemoveActionsForObject(Actor);
    if (auto* Pawn = Cast<APawn>(Actor))
        if (auto* AI = Cast<AAIController>(Pawn->GetController()))
        {
            AI->StopMovement();
            if (AI->GetBrainComponent()) AI->GetBrainComponent()->StopLogic(TEXT("Authoritative historical playback"));
            AI->SetActorTickEnabled(false);
        }
    TArray<UActorComponent*> Components;
    Actor->GetComponents(Components);
    for (auto* Component : Components)
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(Component);
        GetWorld()->GetLatentActionManager().RemoveActionsForObject(Component);
        if (!Component->IsA<USkeletalMeshComponent>() && !Component->IsA<UWidgetComponent>())
            Component->SetComponentTickEnabled(false);
        if (auto* Primitive = Cast<UPrimitiveComponent>(Component))
        {
            Primitive->SetSimulatePhysics(false);
            Primitive->SetGenerateOverlapEvents(false);
            Primitive->SetCanEverAffectNavigation(false);
        }
    }
}

void UTMOPWorldPlaybackComponent::StopLiveDirectors()
{
    auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        if (IsDirector(*It))
        {
            It->SetActorTickEnabled(false);
            Clock->OnSecondChanged.RemoveAll(*It);
            Clock->OnLoopRestarted.RemoveAll(*It);
        }
    if (auto* Schedules = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPScheduleSubsystem>())
    {
        Clock->OnSecondChanged.RemoveAll(Schedules);
        Clock->OnLoopRestarted.RemoveAll(Schedules);
    }
    if (auto* Events = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
    {
        Clock->OnSecondChanged.RemoveAll(Events);
        Clock->OnLoopRestarted.RemoveAll(Events);
    }
}

bool UTMOPWorldPlaybackComponent::PrepareObjects()
{
    Objects.Reset();
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        const FString Id = ActorId(*It);
        Objects.Add(Id, *It);
        TArray<UActorComponent*> Components;
        It->GetComponents(Components);
        for (auto* C : Components) Objects.Add(Id + TEXT("/") + C->GetName(), C);
    }
    // Warm every recorded durable asset once. Seeking does not perform disk IO.
    Assets.Reset();
    for (const auto& Track : Tape.Tracks)
        for (const auto& Property : Track.Properties)
        {
            const FString N = Property.Name.ToString();
            const bool bAsset = N == TEXT("$staticMesh") || N == TEXT("$skeletalMesh") ||
                N == TEXT("$animClass") || N == TEXT("$singleAsset") || N == TEXT("$uniqueAsset") || (N.StartsWith(TEXT("$material")) && !N.StartsWith(TEXT("$materialState_")));
            if (N.StartsWith(TEXT("$materialState_")))
                for (const auto& Key : Property.Keys)
                {
                    FTMOPPlaybackMaterialState State;
                    if (!FJsonObjectConverter::JsonObjectStringToUStruct(Key.Value, &State)) return false;
                    if (!Assets.Contains(State.Parent))
                    {
                        auto* Parent = LoadObject<UMaterialInterface>(nullptr, *State.Parent);
                        if (!Parent) { Status = TEXT("Saknat material: ") + State.Parent; return false; }
                        Assets.Add(State.Parent, Parent);
                    }
                }
            if (N == TEXT("$speech"))
                for (const auto& Key : Property.Keys)
                {
                    TSharedPtr<FJsonObject> Json;
                    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Key.Value), Json) || !Json.IsValid())
                    { Status = TEXT("Ogiltiga replikdata: ") + Track.Id; return false; }
                    const FString Sound = Json->GetStringField(TEXT("sound"));
                    if (!Sound.IsEmpty() && Sound != TEXT("None") && !Assets.Contains(Sound))
                    {
                        UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *Sound);
                        if (!Asset) { Status = TEXT("Saknat replikljud: ") + Sound; return false; }
                        Assets.Add(Sound, Asset);
                    }
                }
            if (!bAsset) continue;
            for (const auto& Key : Property.Keys)
                if (!Key.Value.IsEmpty() && Key.Value != TEXT("None") && !Assets.Contains(Key.Value))
                {
                    UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *Key.Value);
                    if (!Asset) { Status = TEXT("Saknad bake-resurs: ") + Key.Value; return false; }
                    Assets.Add(Key.Value, Asset);
                }
        }
    // All required actors are prepared once while paused, not recreated at every
    // seek. This also guarantees parents/vehicles exist before attachments resolve.
    for (const auto& Track : Tape.Tracks)
    {
        if (!Track.ComponentName.IsNone()) continue;
        AActor* Actor = Cast<AActor>(Objects.FindRef(Track.Id));
        if (Actor && Actor->GetClass()->GetPathName() != Track.ClassPath)
        { Status = TEXT("Objektets klass skiljer sig från baken: ") + Track.Id; return false; }
        if (!Actor)
        {
            if (Track.bPlacedActor) { Status = TEXT("Saknat nivåobjekt: ") + Track.Id; return false; }
            UClass* Class = LoadObject<UClass>(nullptr, *Track.ClassPath);
            if (!Class || !Class->IsChildOf(AActor::StaticClass())) { Status = TEXT("Saknad aktörsklass: ") + Track.ClassPath; return false; }
            Actor = GetWorld()->SpawnActorDeferred<AActor>(Class, Track.Poses[0].Transform,
                GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
            if (!Actor) { Status = TEXT("Kunde inte skapa: ") + Track.Id; return false; }
            Actor->Tags.AddUnique(FName(*(TEXT("TMOP_HistoryId=") + Track.Id)));
            if (auto* Agent = Cast<ATMOPHistoricalAgent>(Actor))
            {
                Agent->EntityIdentity->EntityId = Track.EntityId;
                Agent->bEnableSpawnFade = false;
                Agent->bEnableDespawnFade = false;
            }
            if (auto* Vehicle = Cast<ATMOPVehicleBase>(Actor)) Vehicle->VehicleId = Track.EntityId;
            Actor->FinishSpawning(Track.Poses[0].Transform);
            if (auto* Agent = Cast<ATMOPHistoricalAgent>(Actor))
                if (Agent->PersonProfile && Agent->PersonProfile->LoadProfile()) Agent->MovementProfile = Agent->PersonProfile->Profile.MovementProfile;
            Objects.Add(Track.Id, Actor);
        }
        if (auto* Vehicle = Cast<ATMOPVehicleBase>(Actor))
            if (auto* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
                Registry->RegisterWorldObject(Vehicle->VehicleId, TEXT("HistoricalVehicle"), Vehicle);
        Quiesce(Actor);
        if (!Track.bPlacedActor)
        {
            TArray<USceneComponent*> Scenes;
            Actor->GetComponents(Scenes);
            for (auto* Scene : Scenes) Scene->SetMobility(EComponentMobility::Movable);
        }
        Actor->SetActorHiddenInGame(true);
        TArray<UActorComponent*> Components;
        Actor->GetComponents(Components);
        for (auto* Component : Components)
            Objects.Add(Track.Id + TEXT("/") + Component->GetName(), Component);
    }
    for (const auto& Track : Tape.Tracks)
    {
        if (Track.ComponentName.IsNone() || Objects.Contains(Track.Id)) continue;
        AActor* Actor = Cast<AActor>(Objects.FindRef(Track.ActorId));
        UClass* Class = LoadObject<UClass>(nullptr, *Track.ClassPath);
        if (!Actor || !Class || !Class->IsChildOf(USceneComponent::StaticClass()))
        { Status = TEXT("Saknad komponent utan återställningsadapter: ") + Track.Id; return false; }
        auto* Component = NewObject<USceneComponent>(Actor, Class, Track.ComponentName);
        Actor->AddInstanceComponent(Component);
        if (!Actor->GetRootComponent()) Actor->SetRootComponent(Component);
        else Component->SetupAttachment(Actor->GetRootComponent());
        Component->RegisterComponent();
        Component->SetComponentTickEnabled(false);
        Objects.Add(Track.Id, Component);
    }
    return true;
}

bool UTMOPWorldPlaybackComponent::LoadAndPrepare(const FString& Signature)
{
    if (bReady || bRecording) return bReady;
    if (bPreparationAttempted) { Status = TEXT("Förberedelsen misslyckades. Rätta felet och starta en ny Play-session."); return false; }
    auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    Clock->RequestPause(this, MissingTapePause);
    Clock->bAuthoritativePlayback = true;
    if (!ValidateFile(Signature)) return false;
    if (Tape.StartSecond != Clock->GetLoopStartTime().ToSecondsFromMidnight() || Tape.EndSecond != Clock->GetLoopEndTime().ToSecondsFromMidnight())
    { Status = TEXT("Bake och scenarioklocka har olika tidsintervall."); return false; }
    bPreparationAttempted = true;
    if (!PrepareObjects()) return false;
    StopLiveDirectors();
    bReady = true;
    Clock->bAuthoritativePlayback = true;
    if (!Evaluate(Clock->GetCurrentTimeSecondsExact(), true))
    { bReady = false; Status = TEXT("Baken kunde inte återställas. Världen är pausad."); return false; }
    Clock->ReleasePause(this, MissingTapePause);
    Status.Empty();
    return true;
}

bool UTMOPWorldPlaybackComponent::BeginScrub(UObject* Owner)
{
    if (!bReady || IsBusy() || !IsValid(Owner) || (ScrubOwner.IsValid() && ScrubOwner.Get() != Owner)) return false;
    ScrubOwner = Owner;
    bOwnsScrubPause = true;
    GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->RequestPause(this, SeekPause);
    return true;
}

void UTMOPWorldPlaybackComponent::CancelScrub(UObject* Owner)
{
    if (IsBusy() || ScrubOwner.Get() != Owner) return;
    ScrubOwner.Reset();
    bOwnsScrubPause = false;
    GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->ReleasePause(this, SeekPause);
}

bool UTMOPWorldPlaybackComponent::RequestSeek(double RequestedSecond, UObject* Owner)
{
    if (!bReady || IsBusy() || !FMath::IsFinite(RequestedSecond) ||
        (ScrubOwner.IsValid() && ScrubOwner.Get() != Owner)) return false;
    bOwnsScrubPause = false;
    PendingSecond = TMOPTimeTravel::Snap(RequestedSecond, Tape.StartSecond, Tape.EndSecond);
    GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->RequestPause(this, SeekPause);
    RestoreDelayFrames = 2; // Let Slate display the pending destination before mutation.
    Status = TEXT("Förflyttar till ") + FTMOPTime::FromSecondsFromMidnight(PendingSecond).ToDisplayString();
    return true;
}

bool UTMOPWorldPlaybackComponent::ApplyValue(UObject* Object, FName Name,
    const FString& Value, double Time, bool bSeeking)
{
    const FString N = Name.ToString();
    auto* Actor = Cast<AActor>(Object);
    auto* Scene = Cast<USceneComponent>(Object);
    if (N == TEXT("$hidden")) { if (Actor) Actor->SetActorHiddenInGame(Value == TEXT("1")); return true; }
    if (N == TEXT("$collision")) { if (Actor) Actor->SetActorEnableCollision(Value == TEXT("1")); return true; }
    if (N == TEXT("$visible")) { if (Scene) Scene->SetVisibility(Value == TEXT("1")); return true; }
    if (N == TEXT("$componentHidden")) { if (Scene) Scene->SetHiddenInGame(Value == TEXT("1")); return true; }
    if (N == TEXT("$fade"))
    { if (auto* Agent = Cast<ATMOPHistoricalAgent>(Object)) Agent->SetPlaybackFade(FCString::Atof(*Value)); return true; }
    if (N == TEXT("$primitiveCollision"))
    {
        FTMOPPlaybackCollisionState State;
        auto* Primitive = Cast<UPrimitiveComponent>(Object);
        if (!Primitive || !FJsonObjectConverter::JsonObjectStringToUStruct(Value, &State) || State.Responses.Num() != ECC_MAX) return false;
        Primitive->SetCollisionEnabled(ECollisionEnabled::Type(State.Mode));
        Primitive->SetCollisionObjectType(ECollisionChannel(State.ObjectType));
        FCollisionResponseContainer Response;
        for (int32 C=0; C<ECC_MAX; ++C) Response.SetResponse(ECollisionChannel(C), ECollisionResponse(State.Responses[C]));
        Primitive->SetCollisionResponseToChannels(Response);
        return true;
    }
    if (N == TEXT("$movementMode") || N == TEXT("$customMovementMode") || N == TEXT("$maxWalkSpeed"))
    {
        if (auto* Agent = Cast<ATMOPHistoricalAgent>(Object)) if (auto* Movement = Agent->GetCharacterMovement())
        {
            if (N == TEXT("$movementMode")) Movement->MovementMode = EMovementMode(FCString::Atoi(*Value));
            if (N == TEXT("$customMovementMode")) Movement->CustomMovementMode = uint8(FCString::Atoi(*Value));
            if (N == TEXT("$maxWalkSpeed")) Movement->MaxWalkSpeed = FCString::Atof(*Value);
        }
        return true;
    }
    if (N == TEXT("$occupant"))
    {
        if (auto* Seat = Cast<UTMOPVehicleSeatComponent>(Object))
        {
            auto* Character = Cast<ACharacter>(Objects.FindRef(Value));
            if (!Value.IsEmpty() && !Character) return false;
            if (auto* Player = Cast<ATMOPPlayerCharacter>(Seat->GetOccupantCharacter()))
                if (Character && Player->VehicleSession) Player->VehicleSession->ExitVehicle();
            Seat->RestoreHistoricalOccupant(Character);
        }
        return true;
    }
    if (N == TEXT("$cinemaOccupant"))
    {
        auto* Agent = Cast<ATMOPHistoricalAgent>(Objects.FindRef(Value));
        if (!Value.IsEmpty() && !Agent) return false;
        if (auto* Seat = Cast<UTMOPCinemaSeatComponent>(Object)) Seat->RestoreHistoricalOccupant(Agent);
        return true;
    }
    if (N == TEXT("$actionRemainingPath") || N == TEXT("$actionRequiredSpeed")) return true;
    if (N == TEXT("$action"))
    {
        FTMOPPlaybackActionState State;
        if (!FJsonObjectConverter::JsonObjectStringToUStruct(Value, &State)) return false;
        if (auto* Action = Cast<UTMOPActionExecutorComponent>(Object)) Action->RestorePlaybackAction(State);
        return true;
    }
    if (N == TEXT("$aux"))
    {
        FTMOPPlaybackAuxiliaryState Aux;
        if (!FJsonObjectConverter::JsonObjectStringToUStruct(Value, &Aux)) return false;
        if (auto* Groups = Cast<ATMOPGroupDirector>(Object)) Groups->RestorePlaybackGroups(Aux.Groups, Time);
        if (auto* Observations = Cast<ATMOPObservationDirector>(Object)) Observations->ApplyBakedObservationRuntime(Aux.Observations);
        if (auto* Signals = Cast<ATMOPTrafficSignalController>(Object)) Signals->RestorePlaybackSignals(Aux.Signals);
        return true;
    }
    if (N == TEXT("$speech"))
    { if (auto* Agent = Cast<ATMOPHistoricalAgent>(Object)) Agent->RestorePlaybackSpeech(Value, Time, bSeeking, !bVerifying); return true; }
    if (N == TEXT("$parent") || N == TEXT("$socket") || N == TEXT("$uniqueAsset")) return true; // Dependency-ordered pass below.
    if (N == TEXT("$staticMesh"))
    {
        if (auto* Mesh = Cast<UStaticMeshComponent>(Object))
        {
            auto* Asset = Value == TEXT("None") ? nullptr : Cast<UStaticMesh>(Assets.FindRef(Value));
            if (!Asset && Value != TEXT("None")) return false;
            Mesh->SetStaticMesh(Asset);
        }
        return true;
    }
    if (N == TEXT("$skeletalMesh"))
    {
        if (auto* Mesh = Cast<USkeletalMeshComponent>(Object))
        {
            auto* Asset = Value == TEXT("None") ? nullptr : Cast<USkeletalMesh>(Assets.FindRef(Value));
            if (!Asset && Value != TEXT("None")) return false;
            if (Mesh->GetSkeletalMeshAsset() != Asset) Mesh->SetSkeletalMesh(Asset);
        }
        return true;
    }
    if (N.StartsWith(TEXT("$materialState_")))
    {
        auto* Mesh = Cast<UMeshComponent>(Object);
        FTMOPPlaybackMaterialState State;
        if (!Mesh || !FJsonObjectConverter::JsonObjectStringToUStruct(Value, &State)) return false;
        const int32 Slot = FCString::Atoi(*N.Mid(15));
        auto* Parent = Cast<UMaterialInterface>(Assets.FindRef(State.Parent));
        if (!Parent) return false;
        if (!State.bDynamic) { Mesh->SetMaterial(Slot, Parent); return true; }
        auto* Dynamic = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(Slot));
        if (!Dynamic || Dynamic->Parent != Parent)
        {
            Dynamic = UMaterialInstanceDynamic::Create(Parent, Mesh);
            Mesh->SetMaterial(Slot, Dynamic);
        }
        for (const auto& Pair : State.Scalars) Dynamic->SetScalarParameterValue(FName(*Pair.Key), Pair.Value);
        for (const auto& Pair : State.Vectors) Dynamic->SetVectorParameterValue(FName(*Pair.Key), Pair.Value);
        return true;
    }
    if (N == TEXT("$morphs"))
    {
        FTMOPPlaybackMorphState State;
        auto* Mesh = Cast<USkeletalMeshComponent>(Object);
        if (!Mesh || !FJsonObjectConverter::JsonObjectStringToUStruct(Value, &State)) return false;
        Mesh->ClearMorphTargets();
        for (const auto& Pair : State.Weights) Mesh->SetMorphTarget(FName(*Pair.Key), Pair.Value);
        return true;
    }
    if (N.StartsWith(TEXT("$material")))
    {
        if (auto* Mesh = Cast<UMeshComponent>(Object))
        {
            auto* Material = Cast<UMaterialInterface>(Assets.FindRef(Value));
            if (!Material) return false;
            Mesh->SetMaterial(FCString::Atoi(*N.Mid(9)), Material);
        }
        return true;
    }
    if (N == TEXT("$intensity")) { if (auto* L = Cast<ULightComponent>(Object)) L->SetIntensity(FCString::Atof(*Value)); return true; }
    if (N == TEXT("$lightColor"))
    { FLinearColor Color; Color.InitFromString(Value); if (auto* L = Cast<ULightComponent>(Object)) L->SetLightColor(Color); return true; }
    if (N == TEXT("$animClass") || N == TEXT("$singleAsset") || N == TEXT("$singleTime")) return true;
    FProperty* P = FindFProperty<FProperty>(Object->GetClass(), Name);
    return P && RecordProperty(P) && P->ImportText_Direct(*Value, P->ContainerPtrToValuePtr<void>(Object), Object, PPF_None) != nullptr;
}

bool UTMOPWorldPlaybackComponent::Evaluate(double Time, bool bSeeking)
{
    const int32 WorldKey = Floor(Tape.WorldStateKeys, Time);
    if (WorldKey >= 0 && (bSeeking || WorldKey != AppliedWorldStateKey))
    {
        FTMOPPlaybackWorldState State;
        if (!FJsonObjectConverter::JsonObjectStringToUStruct(Tape.WorldStateKeys[WorldKey].Value, &State)) return false;
        GetWorld()->GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>()->RestoreHistoricalState(State.Values, Tape.WorldStateNames);
        AppliedWorldStateKey = WorldKey;
    }
    // Preflight the complete graph before exposing a new clock or changing anything.
    for (const auto& Track : Tape.Tracks) if (!IsValid(Objects.FindRef(Track.Id))) return false;
    if (AppliedPropertyKeys.Num() != Tape.Tracks.Num())
    {
        AppliedPropertyKeys.SetNum(Tape.Tracks.Num());
        for (int32 T=0; T<Tape.Tracks.Num(); ++T) AppliedPropertyKeys[T].Init(INDEX_NONE, Tape.Tracks[T].Properties.Num());
    }
    if (bSeeking) for (auto& Keys : AppliedPropertyKeys) for (int32& K : Keys) K = INDEX_NONE;
    auto ValueAt = [Time](const FTMOPPlaybackObjectTrack& Track, const TCHAR* Name)
    {
        const auto* P = Track.Properties.FindByPredicate([Name](const auto& V) { return V.Name == FName(Name); });
        const int32 I = P ? Floor(P->Keys, Time) : INDEX_NONE;
        return I >= 0 ? P->Keys[I].Value : FString();
    };
    // Registry-backed group restoration must see the target lifetimes for all
    // people, regardless of whether a group track precedes a person track.
    for (const auto& Track : Tape.Tracks)
        if (auto* Actor = Cast<AActor>(Objects.FindRef(Track.Id)))
        {
            const int32 I = Floor(Track.Poses, Time);
            if (I >= 0 && Track.Poses[I].bPresent) Actor->Tags.Remove(TEXT("TMOP_HistoryAbsent"));
            else Actor->Tags.AddUnique(TEXT("TMOP_HistoryAbsent"));
        }
    for (const auto& Track : Tape.Tracks)
    {
        UObject* Object = Objects.FindRef(Track.Id);
        const int32 Index = Floor(Track.Poses, Time);
        const bool bPresent = Index >= 0 && Track.Poses[Index].bPresent;
        if (auto* Actor = Cast<AActor>(Object))
        {
            if (bPresent) Actor->Tags.Remove(TEXT("TMOP_HistoryAbsent"));
            else Actor->Tags.AddUnique(TEXT("TMOP_HistoryAbsent"));
            Actor->SetActorHiddenInGame(!bPresent || ValueAt(Track, TEXT("$hidden")) == TEXT("1"));
            Actor->SetActorEnableCollision(bPresent && ValueAt(Track, TEXT("$collision")) == TEXT("1"));
        }
        if (auto* Scene = Cast<USceneComponent>(Object))
            Scene->SetVisibility(bPresent && ValueAt(Track, TEXT("$visible")) == TEXT("1"));
        if (!bPresent)
        {
            for (int32& Applied : AppliedPropertyKeys[&Track - Tape.Tracks.GetData()]) Applied = INDEX_NONE;
            if (auto* Agent = Cast<ATMOPHistoricalAgent>(Object)) Agent->HideAutomaticSpeech();
            if (auto* Seat = Cast<UTMOPVehicleSeatComponent>(Object)) Seat->RestoreHistoricalOccupant(nullptr);
            if (auto* Seat = Cast<UTMOPCinemaSeatComponent>(Object)) Seat->RestoreHistoricalOccupant(nullptr);
            continue;
        }
        for (const auto& Property : Track.Properties)
        {
            if (Property.Name != TEXT("$skeletalMesh") && Property.Name != TEXT("$staticMesh")) continue;
            const int32 I = Floor(Property.Keys, Time);
            if (I < 0) continue;
            int32& Applied = AppliedPropertyKeys[&Track - Tape.Tracks.GetData()][&Property - Track.Properties.GetData()];
            if (Applied != I)
            {
                if (!ApplyValue(Object, Property.Name, Property.Keys[I].Value, Time, bSeeking)) return false;
                Applied = I;
            }
        }
        for (const auto& Property : Track.Properties)
        {
            const int32 I = Floor(Property.Keys, Time);
            if (I < 0) continue;
            int32& Applied = AppliedPropertyKeys[&Track - Tape.Tracks.GetData()][&Property - Track.Properties.GetData()];
            const FString& Value = Property.Keys[I].Value;
            if (Applied != I)
            {
                if (!ApplyValue(Object, Property.Name, Value, Time, bSeeking)) return false;
                Applied = I;
            }
        }
    }
    // Attachments first, then world transforms. No ExitSeat/EnterSeat callbacks
    // are replayed; those would start new actions or broadcast historical events.
    // Remove obsolete links first. A valid target graph must not be rejected
    // just because an edge in the previous graph temporarily points the other way.
    for (const auto& Track : Tape.Tracks)
    {
        const int32 Index = Floor(Track.Poses, Time);
        if (Index < 0 || !Track.Poses[Index].bPresent) continue;
        UObject* Object = Objects.FindRef(Track.Id);
        auto* Actor = Cast<AActor>(Object);
        auto* Scene = Actor ? Actor->GetRootComponent() : Cast<USceneComponent>(Object);
        if (!Scene || !Scene->GetAttachParent()) continue;
        auto* Parent = Cast<USceneComponent>(Objects.FindRef(ValueAt(Track, TEXT("$parent"))));
        const FName Socket(*ValueAt(Track, TEXT("$socket")));
        if (Scene->GetAttachParent() != Parent || Scene->GetAttachSocketName() != Socket)
            Scene->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    }
    for (const auto& Track : Tape.Tracks)
    {
        UObject* Object = Objects.FindRef(Track.Id);
        const int32 Index = Floor(Track.Poses, Time);
        if (Index < 0 || !Track.Poses[Index].bPresent) continue;
        auto* Actor = Cast<AActor>(Object);
        auto* Scene = Actor ? Actor->GetRootComponent() : Cast<USceneComponent>(Object);
        if (!Scene) continue;
        const FString ParentId = ValueAt(Track, TEXT("$parent"));
        auto* Parent = Cast<USceneComponent>(Objects.FindRef(ParentId));
        const FName Socket(*ValueAt(Track, TEXT("$socket")));
        if (!ParentId.IsEmpty() && !Parent) return false;
        if (Parent == Scene) return false;
        for (auto* Ancestor = Parent; Ancestor; Ancestor = Ancestor->GetAttachParent()) if (Ancestor == Scene) return false;
        if (Parent && (Scene->GetAttachParent() != Parent || Scene->GetAttachSocketName() != Socket))
            Scene->AttachToComponent(Parent, FAttachmentTransformRules::KeepWorldTransform, Socket);
        else if (!Parent && Scene->GetAttachParent()) Scene->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    }
    TArray<const FTMOPPlaybackObjectTrack*> Ordered;
    for (const auto& T : Tape.Tracks) Ordered.Add(&T);
    auto Depth = [this](const FTMOPPlaybackObjectTrack& T)
    {
        AActor* A = Cast<AActor>(Objects.FindRef(T.Id));
        if (!A) return -1; // Relative component offsets before world transforms.
        int32 D = 0;
        while (A->GetAttachParentActor() && D < 64) { A = A->GetAttachParentActor(); ++D; }
        return D;
    };
    Ordered.StableSort([&](const auto& A, const auto& B) { return Depth(A) < Depth(B); });
    for (const auto* TrackPtr : Ordered)
    {
        const auto& Track = *TrackPtr;
        UObject* Object = Objects.FindRef(Track.Id);
        const int32 Index = Floor(Track.Poses, Time);
        if (Index < 0 || !Track.Poses[Index].bPresent) continue;
        const auto& A = Track.Poses[Index];
        FTransform Pose = A.Transform;
        FVector Velocity = A.Velocity;
        if (Track.Poses.IsValidIndex(Index + 1))
        {
            const auto& B = Track.Poses[Index + 1];
            if (B.bPresent && !B.bCut && B.Time > A.Time)
            {
                const float Alpha = float(TMOPTimeTravel::BlendAlpha(A.Time, B.Time, Time, B.bPresent, B.bCut));
                Pose.Blend(A.Transform, B.Transform, Alpha);
                Velocity = FMath::Lerp(A.Velocity, B.Velocity, Alpha);
            }
        }
        if (auto* Actor = Cast<AActor>(Object))
        {
            if (!Actor->GetActorTransform().Equals(Pose, 0.000001f))
                Actor->SetActorTransform(Pose, false, nullptr, ETeleportType::TeleportPhysics);
            if (auto* Agent = Cast<ATMOPHistoricalAgent>(Actor))
            {
                Agent->GetCharacterMovement()->Velocity = Velocity;
                Agent->UpdatePlaybackPresentation(Time, bSeeking);
            }
        }
        else if (auto* Scene = Cast<USceneComponent>(Object))
        {
            // Root transform is set by the actor track; setting it again as a
            // relative transform would apply a seated person's offset twice.
            if (Scene != Scene->GetOwner()->GetRootComponent() && !Scene->GetRelativeTransform().Equals(Pose, 0.000001f)) Scene->SetRelativeTransform(Pose);
        }
        if (auto* Action = Cast<UTMOPActionExecutorComponent>(Object))
            Action->RestorePlaybackMoveDiagnostics(FCString::Atof(*ValueAt(Track, TEXT("$actionRemainingPath"))), FCString::Atof(*ValueAt(Track, TEXT("$actionRequiredSpeed"))));
        if (auto* Mesh = Cast<USkeletalMeshComponent>(Object))
        {
            const FString SingleAsset = ValueAt(Track, TEXT("$singleAsset"));
            if (!SingleAsset.IsEmpty() && SingleAsset != TEXT("None"))
            {
                auto* Asset = Cast<UAnimationAsset>(Assets.FindRef(SingleAsset));
                if (!Asset) return false;
                auto* Single = Mesh->GetSingleNodeInstance();
                if (!Single || Single->GetCurrentAsset() != Asset) Mesh->PlayAnimation(Asset, false);
                float ClipTime = FCString::Atof(*ValueAt(Track, TEXT("$singleTime")));
                const auto* Cursor = Track.Properties.FindByPredicate([](const auto& P) { return P.Name == TEXT("$singleTime"); });
                const int32 CursorIndex = Cursor ? Floor(Cursor->Keys, Time) : INDEX_NONE;
                if (CursorIndex >= 0 && Cursor->Keys.IsValidIndex(CursorIndex + 1))
                {
                    const auto& Next = Cursor->Keys[CursorIndex + 1];
                    const float NextPosition = FCString::Atof(*Next.Value);
                    const double Span = Next.Time - Cursor->Keys[CursorIndex].Time;
                    if (Span > 0 && Span < 0.1 && NextPosition >= ClipTime && NextPosition - ClipTime < 0.2f)
                        ClipTime = FMath::Lerp(ClipTime, NextPosition, float((Time - Cursor->Keys[CursorIndex].Time) / Span));
                }
                Mesh->SetPosition(ClipTime, false);
                Mesh->SetPlayRate(0.0f);
            }
            else if (Mesh->GetSingleNodeInstance())
            {
                const FString Class = ValueAt(Track, TEXT("$animClass"));
                if (!Class.IsEmpty() && Class != TEXT("None")) Mesh->SetAnimInstanceClass(Cast<UClass>(Assets.FindRef(Class)));
            }
        }
    }
    if (bSeeking || int32(Time) != int32(LastEvaluatedTime))
        if (auto* Events = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
            Events->ApplyBakedEventRuntime(Tape.Events, FTMOPTime::FromSecondsFromMidnight(int32(Time)));
    for (TActorIterator<ATMOPPalmeShotDirector> It(GetWorld()); It; ++It)
        if (!bVerifying) It->EvaluatePlaybackAudio(LastEvaluatedTime, Time, bSeeking);
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It) It->UpdatePlaybackTime(Time);
    LastEvaluatedTime = Time;
    return true;
}

void UTMOPWorldPlaybackComponent::TickComponent(float Delta, ELevelTick Type, FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta, Type, Tick);
    auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    if (bRecording)
    {
        const double Time = Clock->GetCurrentTimeSecondsExact();
        if (Time > Tape.RecordedThrough + 0.000001) Capture(Time);
        if (Time >= Tape.EndSecond) FinishRecording(true);
        return;
    }
    if (!bReady) return;
    if (bOwnsScrubPause && !ScrubOwner.IsValid())
    {
        bOwnsScrubPause = false;
        Clock->ReleasePause(this, SeekPause);
    }
    for (const auto& Track : Tape.Tracks) if (Track.ComponentName.IsNone())
    {
        if (auto* Agent = Cast<ATMOPHistoricalAgent>(Objects.FindRef(Track.Id))) Agent->UpdatePlaybackNameLabel();
        if (auto* Vehicle = Cast<ATMOPVehicleBase>(Objects.FindRef(Track.Id))) Vehicle->UpdatePlaybackNameLabel();
    }
    if (PendingSecond != INDEX_NONE)
    {
        if (--RestoreDelayFrames > 0) return;
        const double RestoreStarted = FPlatformTime::Seconds();
        const double Previous = LastEvaluatedTime;
        // Detach local players before any historical attachment graph changes.
        for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
            if (Player->VehicleSession) Player->VehicleSession->ExitVehicle();
        if (!Evaluate(PendingSecond, true))
        {
            Evaluate(Previous, true);
            PendingSecond = INDEX_NONE;
            ScrubOwner.Reset();
            Status = TEXT("Tidsförflyttningen misslyckades. Världen är pausad.");
            return; // Fail closed: never resume a partially restored world.
        }
        UE_LOG(LogTemp, Display, TEXT("TMOP seek %d: %.2f ms, %d tracks"), PendingSecond,
            (FPlatformTime::Seconds() - RestoreStarted) * 1000, Tape.Tracks.Num());
        Clock->CommitHistoricalTime(FTMOPTime::FromSecondsFromMidnight(PendingSecond));
        for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
            if (auto* Radio = Player->FindComponentByClass<UTMOPPlayerRadioComponent>()) Radio->RestoreAfterTimeSeek();
        for (TActorIterator<ATMOPRecordedCallDirector> It(GetWorld()); It; ++It) It->SynchronizeToClock();
        PendingSecond = INDEX_NONE;
        ScrubOwner.Reset();
        Status.Empty();
        Clock->ReleasePause(this, SeekPause);
    }
    else if (Clock->GetCurrentTimeSecondsExact() != LastEvaluatedTime)
    {
        if (!Evaluate(Clock->GetCurrentTimeSecondsExact(), false))
        {
            Clock->RequestPause(this, SeekPause);
            Status = TEXT("Ett historiskt objekt saknas. Uppspelningen har pausats.");
        }
    }
    if (!IsBusy() && !ScrubOwner.IsValid()) Clock->PublishHistoricalTime();
}

void UTMOPWorldPlaybackComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (bRecording) FinishRecording(false);
    if (GetWorld() && GetWorld()->GetGameInstance())
    {
        auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
        Clock->bAuthoritativePlayback = false;
        Clock->ReleaseAllPauses(this);
    }
    Super::EndPlay(Reason);
}

bool UTMOPWorldPlaybackComponent::ValidateFile(const FString& Signature)
{
    if (bRecording) { Status = TEXT("Inspelningen måste bli klar före validering."); return false; }
    if (bReady)
    {
        if (Tape.Signature != Signature) { Status = TEXT("Källorna har ändrats. Starta om och skapa en ny bake."); return false; }
        return Validate(Status);
    }
    TArray<uint8> Bytes;
    FString Path = GetTapePath();
    if (!IFileManager::Get().FileExists(*Path))
        Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("TMOP/Bakes"), FPaths::GetCleanFilename(Path));
    if (!FFileHelper::LoadFileToArray(Bytes, *Path))
    { Status = TEXT("Historisk bake saknas. Kör Bake Entire Simulation först."); return false; }
    if (Bytes.Num() < 16) { Status = TEXT("Ofullständig bake-fil."); return false; }
    FMemoryReader Reader(Bytes, true);
    uint32 Magic = 0, Checksum = 0;
    int64 Size = 0;
    Reader << Magic << Size << Checksum;
    if (Magic != 0x544D5033 || Size != Bytes.Num() - Reader.Tell() ||
        FCrc::MemCrc32(Bytes.GetData() + Reader.Tell(), int32(Size)) != Checksum)
    { Status = TEXT("Bake-filens version, storlek eller kontrollsumma är fel."); return false; }
    TapeChecksum = Checksum;
    Tape = FTMOPPlaybackTape();
    FObjectAndNameAsStringProxyArchive Archive(Reader, true);
    FTMOPPlaybackTape::StaticStruct()->SerializeItem(Archive, &Tape, nullptr);
    if (Archive.IsError() || !Validate(Status)) return false;
    if (Tape.Signature != Signature)
    { Status = TEXT("Baken är inaktuell. Skapa en ny efter ändringarna."); return false; }
    Status = TEXT("Bake-data och källsignatur är giltiga.");
    return true;
}

FString UTMOPWorldPlaybackComponent::HashCurrentWorld() const
{
    FString State;
    for (const auto& Track : Tape.Tracks)
    {
        UObject* Object = Objects.FindRef(Track.Id);
        if (!IsValid(Object)) return TEXT("MISSING:") + Track.Id;
        const int32 PoseIndex = Floor(Track.Poses, LastEvaluatedTime);
        State += Track.Id;
        if (PoseIndex < 0 || !Track.Poses[PoseIndex].bPresent)
        {
            if (auto* Actor = Cast<AActor>(Object))
                if (!Actor->IsHidden() || Actor->GetActorEnableCollision()) return TEXT("ACTIVE_ABSENT:") + Track.Id;
            State += TEXT("absent;");
            continue;
        }
        if (auto* Actor = Cast<AActor>(Object))
        {
            State += Actor->GetActorTransform().ToString();
            State += Actor->IsHidden() ? TEXT("hidden") : TEXT("visible");
        }
        if (auto* Scene = Cast<USceneComponent>(Object))
        {
            State += Scene->GetRelativeTransform().ToString();
            State += Scene->IsVisible() ? TEXT("visible") : TEXT("hidden");
            State += GetNameSafe(Scene->GetAttachParent());
            State += Scene->GetAttachSocketName().ToString();
        }
        for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
            if (RecordProperty(*It)) It->ExportText_InContainer(0, State, Object, Object, Object, PPF_None);
        if (auto* Seat = Cast<UTMOPVehicleSeatComponent>(Object)) State += ActorId(Seat->GetOccupant());
        if (auto* Agent = Cast<ATMOPHistoricalAgent>(Object)) State += Agent->CapturePlaybackSpeech();
        if (auto* Seat = Cast<UTMOPCinemaSeatComponent>(Object)) State += ActorId(Seat->GetOccupyingAgent());
        if (auto* Action = Cast<UTMOPActionExecutorComponent>(Object))
        { FString Json; FJsonObjectConverter::UStructToJsonObjectString(Action->CapturePlaybackAction(), Json); State += Json; }
        if (auto* Group = Cast<ATMOPGroupDirector>(Object))
        { FTMOPPlaybackAuxiliaryState Aux; Aux.Groups = Group->GetAllGroupSnapshots(); FString Json; FJsonObjectConverter::UStructToJsonObjectString(Aux, Json); State += Json; }
        if (auto* Mesh = Cast<UStaticMeshComponent>(Object)) State += GetPathNameSafe(Mesh->GetStaticMesh());
        if (auto* Mesh = Cast<USkeletalMeshComponent>(Object))
        {
            State += GetPathNameSafe(Mesh->GetSkeletalMeshAsset());
            if (auto* Single = Mesh->GetSingleNodeInstance()) State += FString::SanitizeFloat(Single->GetCurrentTime());
        }
        if (auto* Light = Cast<ULightComponent>(Object)) State += FString::SanitizeFloat(Light->Intensity) + Light->GetLightColor().ToString();
    }
    if (auto* World = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
    {
        TArray<FName> Names = Tape.WorldStateNames.Array(); Names.Sort(FNameLexicalLess());
        for (FName Name : Names)
        {
            FTMOPWorldStateValue Value;
            State += Name.ToString();
            State += World->TryGetWorldState(Name, Value) ? Value.ToDebugString() : TEXT("absent");
        }
    }
    if (auto* Events = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        for (const auto& E : Tape.Events)
        {
            FTMOPHistoricalEventRuntime Runtime;
            if (Events->TryGetEventRuntime(E.EventId, Runtime))
                State += E.EventId.ToString() + FString::FromInt(int32(Runtime.State)) + Runtime.ResolvedTime.ToDisplayString();
        }
    return FMD5::HashAnsiString(*State);
}

bool UTMOPWorldPlaybackComponent::VerifyRepeatability()
{
    if (!bReady || IsBusy() || IsScrubbing()) { Status = TEXT("Ladda en bake i Play före verifieringen."); return false; }
    for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
        if (Player->VehicleSession && Player->VehicleSession->IsInVehicle())
        { Status = TEXT("Lämna fordonet före verifieringen."); return false; }
    auto* Clock = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    Clock->RequestPause(this, TEXT("VerifyHistory"));
    const double Original = LastEvaluatedTime;
    TGuardValue<bool> VerifyGuard(bVerifying, true);
    const FString ReportPath = GetTapePath() + TEXT(".verification.json");
    const FString Identity = Tape.Signature + FString::Printf(TEXT(":%u"), TapeChecksum);
    FString PreviousReport;
    TSharedPtr<FJsonObject> Baseline;
    if (FFileHelper::LoadFileToString(PreviousReport, *ReportPath))
        FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(PreviousReport), Baseline);
    FString PreviousIdentity;
    const bool bCompareSession = Baseline.IsValid() && Baseline->TryGetStringField(TEXT("bake"), PreviousIdentity) && PreviousIdentity == Identity;
    auto Results = MakeShared<FJsonObject>();
    Results->SetStringField(TEXT("bake"), Identity);
    bool bPassed = true;
    for (int32 A = Tape.StartSecond; A <= Tape.EndSecond; A += TMOPTimeTravel::SeekStepSeconds)
    {
        if (!Evaluate(A, true)) { bPassed = false; break; }
        const FString Expected = HashCurrentWorld();
        const double Next = FMath::Min(double(Tape.EndSecond), A + 0.075);
        if (!Evaluate(Next, false)) { bPassed = false; break; }
        const FString Continuation = HashCurrentWorld();
        const FString Key = FString::FromInt(A);
        const FString Digest = Expected + TEXT(":") + Continuation;
        Results->SetStringField(Key, Digest);
        FString PreviousDigest;
        if (bCompareSession && (!Baseline->TryGetStringField(Key, PreviousDigest) || PreviousDigest != Digest))
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP playback differs from the saved session at %d."), A);
            bPassed = false; break;
        }
        const int32 B = A < (Tape.StartSecond + Tape.EndSecond) / 2 ? Tape.EndSecond : Tape.StartSecond;
        if (!Evaluate(B, true) || !Evaluate(A, true) || HashCurrentWorld() != Expected ||
            !Evaluate(Next, false) || HashCurrentWorld() != Continuation)
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP replay round-trip differs at %d."), A);
            bPassed = false;
            break;
        }
    }
    bVerifying = false;
    const bool bRestored = Evaluate(Original, true);
    if (bPassed && bRestored)
    {
        FString Json; FJsonSerializer::Serialize(Results, TJsonWriterFactory<>::Create(&Json));
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(ReportPath), true);
        if (!FFileHelper::SaveStringToFile(Json, *ReportPath)) bPassed = false;
        UE_LOG(LogTemp, Display, TEXT("TMOP session baseline: %s"), bCompareSession ? TEXT("MATCH") : TEXT("CREATED; rerun in a fresh Play session"));
    }
    if (bRestored) Clock->ReleasePause(this, TEXT("VerifyHistory"));
    Status = bPassed && bRestored ? TEXT("Återkomst och fortsättning matchar vid alla femsekunderssteg.") : TEXT("Återkomsttest misslyckades. Se Output Log.");
    UE_LOG(LogTemp, Display, TEXT("TMOP: %s"), *Status);
    return bPassed && bRestored;
}
