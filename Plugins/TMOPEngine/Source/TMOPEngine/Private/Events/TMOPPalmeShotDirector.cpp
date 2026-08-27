#include "Events/TMOPPalmeShotDirector.h"

#include "Actions/TMOPActionExecutorComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Sound/SoundBase.h"
#include "Time/TMOPClockSubsystem.h"

ATMOPPalmeShotDirector::ATMOPPalmeShotDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    BloodPoolComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BloodPool"));
    SetRootComponent(BloodPoolComponent);
    BloodPoolComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BloodPoolComponent->SetGenerateOverlapEvents(false);
    BloodPoolComponent->SetCastShadow(false);
    BloodPoolComponent->SetHiddenInGame(true);
}

void ATMOPPalmeShotDirector::BeginPlay()
{
    Super::BeginPlay();
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
            Clock->OnSecondChanged.AddDynamic(this,
                &ATMOPPalmeShotDirector::HandleSecondChanged);
        if (UTMOPHistoricalEventSubsystem* Events =
            GameInstance->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        {
            Events->OnHistoricalEventTriggered.AddDynamic(this,
                &ATMOPPalmeShotDirector::HandleHistoricalEventTriggered);
            Events->OnHistoricalEventsReset.AddDynamic(this,
                &ATMOPPalmeShotDirector::HandleHistoricalEventsReset);
            if (Events->HasEventTriggered(ShotEventId)) StartSequence();
        }
    }
}

void ATMOPPalmeShotDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RestoreSlowMotion();
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
            Clock->OnSecondChanged.RemoveDynamic(this,
                &ATMOPPalmeShotDirector::HandleSecondChanged);
        if (UTMOPHistoricalEventSubsystem* Events =
            GameInstance->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        {
            Events->OnHistoricalEventTriggered.RemoveDynamic(this,
                &ATMOPPalmeShotDirector::HandleHistoricalEventTriggered);
            Events->OnHistoricalEventsReset.RemoveDynamic(this,
                &ATMOPPalmeShotDirector::HandleHistoricalEventsReset);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void ATMOPPalmeShotDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bSequenceActive) return;
    const UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const float SimulationScale = Clock != nullptr ? Clock->GetTimeScale() : 1.0f;
    const float WorldDilation = FMath::Max(
        UGameplayStatics::GetGlobalTimeDilation(this), KINDA_SMALL_NUMBER);
    // DeltaSeconds and skeletal playback already contain global dilation. Only
    // apply the remaining clock multiplier so dramatic slow motion is not squared.
    const float RelativeClockScale = SimulationScale / WorldDilation;
    if (OlofAgent.IsValid() && IsValid(OlofAgent->BodyMesh))
        OlofAgent->BodyMesh->SetPlayRate(RelativeClockScale);
    if (KillerAgent.IsValid() && IsValid(KillerAgent->BodyMesh))
        KillerAgent->BodyMesh->SetPlayRate(RelativeClockScale);
    SequenceTime += DeltaSeconds * RelativeClockScale;
    if (!bFirstShotFired && SequenceTime >= FirstShotTimeSeconds) FireFirstShot();
    if (!bSecondShotFired && SequenceTime >= SecondShotTimeSeconds) FireSecondShot();
    if (SequenceTime >= SequenceDurationSeconds) FinishSequence();
}

void ATMOPPalmeShotDirector::HandleHistoricalEventTriggered(const FName EventId,
    const FTMOPTime TriggerTime)
{
    if (EventId == ShotEventId) StartSequence();
}

void ATMOPPalmeShotDirector::HandleHistoricalEventsReset(const int32 LoopNumber)
{
    ResetSequence();
}

void ATMOPPalmeShotDirector::HandleSecondChanged(const FTMOPTime NewTime)
{
    if (GetGameInstance() == nullptr) return;
    const UTMOPHistoricalEventSubsystem* Events =
        GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>();
    FTMOPHistoricalEventRuntime Runtime;
    if (Events == nullptr || !Events->TryGetEventRuntime(ShotEventId, Runtime) ||
        !Runtime.bHasResolvedTime) return;

    TryActivateProximitySlowMotion(NewTime, Runtime.ResolvedTime);
    if (bSlowMotionActive &&
        NewTime.ToSecondsFromMidnight() >= SlowMotionEndTimeSeconds)
        RestoreSlowMotion();

    if (bSequenceActive) return;
    // The action begins before the first gunshot. Starting on the whole second
    // two seconds before the event places frame 51 at 23:21:29.7, i.e. the
    // clock's 23:21:30 shot second without skipping Blender frames.
    const int32 LeadSeconds = FMath::CeilToInt(FirstShotTimeSeconds);
    if (NewTime.ToSecondsFromMidnight() ==
        Runtime.ResolvedTime.ToSecondsFromMidnight() - LeadSeconds)
        StartSequence();
}

void ATMOPPalmeShotDirector::TryActivateProximitySlowMotion(
    const FTMOPTime& NewTime, const FTMOPTime& ShotTime)
{
    if (!bEnableProximitySlowMotion || bSlowMotionEvaluated || GetWorld() == nullptr)
        return;
    const int32 ShotSeconds = ShotTime.ToSecondsFromMidnight();
    const int32 EvaluationSeconds = ShotSeconds -
        FMath::RoundToInt(SlowMotionLeadSeconds);
    if (NewTime.ToSecondsFromMidnight() != EvaluationSeconds) return;

    bSlowMotionEvaluated = true;
    const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    const FVector MurderLocation = ResolveAnchorLocation(OlofStartAnchorId,
        OlofAgent.IsValid() ? OlofAgent->GetActorLocation() : GetActorLocation());
    const float RadiusCm = SlowMotionActivationRadiusMeters * 100.0f;
    if (!IsValid(PlayerPawn) ||
        FVector::DistSquared2D(PlayerPawn->GetActorLocation(), MurderLocation) >
        FMath::Square(RadiusCm)) return;

    if (UTMOPClockSubsystem* Clock =
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        SavedTimeScale = Clock->GetTimeScale();
        SavedGlobalTimeDilation = UGameplayStatics::GetGlobalTimeDilation(this);
        const float Factor = FMath::Clamp(SlowMotionFactor, 0.01f, 1.0f);
        Clock->SetTimeScale(SavedTimeScale * Factor);
        UGameplayStatics::SetGlobalTimeDilation(this, SavedGlobalTimeDilation * Factor);
        SlowMotionEndTimeSeconds = ShotSeconds +
            FMath::RoundToInt(SlowMotionEndSecondsAfterFirstShot);
        bSlowMotionActive = true;
    }
}

void ATMOPPalmeShotDirector::RestoreSlowMotion()
{
    if (!bSlowMotionActive) return;
    if (GetGameInstance() != nullptr)
        if (UTMOPClockSubsystem* Clock =
            GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
            Clock->SetTimeScale(SavedTimeScale);
    UGameplayStatics::SetGlobalTimeDilation(this, SavedGlobalTimeDilation);
    bSlowMotionActive = false;
    SlowMotionEndTimeSeconds = INDEX_NONE;
}

ATMOPHistoricalAgent* ATMOPPalmeShotDirector::FindAgent(const FName EntityId) const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPPersonRegistryDirector> It(GetWorld()); It; ++It)
        return It->FindSpawnedPerson(EntityId);
    return nullptr;
}

FVector ATMOPPalmeShotDirector::ResolveAnchorLocation(const FName AnchorId,
    const FVector& Fallback) const
{
    if (AnchorId.IsNone() || GetGameInstance() == nullptr) return Fallback;
    const UTMOPAnchorSubsystem* Anchors =
        GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
    const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
        ? Anchors->FindAnchor(AnchorId) : nullptr;
    return IsValid(Anchor) ? Anchor->GetActorLocation() : Fallback;
}

void ATMOPPalmeShotDirector::StartSequence()
{
    if (bSequenceActive) return;
    OlofAgent = FindAgent(OlofEntityId);
    KillerAgent = FindAgent(KillerEntityId);
    if (!OlofAgent.IsValid() || !KillerAgent.IsValid())
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Palme shot: Olof or killer is not spawned at event time."));
        return;
    }
    const auto PrepareAgent = [](ATMOPHistoricalAgent* Agent, UAnimSequence* Animation,
        const FName AnchorId, const ATMOPPalmeShotDirector* Director)
    {
        if (!IsValid(Agent)) return;
        if (IsValid(Agent->ActionExecutor)) Agent->ActionExecutor->CancelCurrentAction();
        if (AController* Controller = Agent->GetController()) Controller->StopMovement();
        if (UCharacterMovementComponent* Movement = Agent->GetCharacterMovement())
            Movement->DisableMovement();
        if (!AnchorId.IsNone() && Director->GetGameInstance() != nullptr)
        {
            const UTMOPAnchorSubsystem* Anchors =
                Director->GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
            const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
                ? Anchors->FindAnchor(AnchorId) : nullptr;
            if (IsValid(Anchor)) Agent->SetActorTransform(Anchor->GetActorTransform(),
                false, nullptr, ETeleportType::TeleportPhysics);
        }
        Agent->SetActivityState(ETMOPAgentActivityState::Interacting);
        if (IsValid(Animation) && IsValid(Agent->BodyMesh))
        {
            Agent->BodyMesh->PlayAnimation(Animation, false);
            const UTMOPClockSubsystem* Clock = Director->GetGameInstance() != nullptr
                ? Director->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
            const float ClockScale = Clock != nullptr ? Clock->GetTimeScale() : 1.0f;
            const float WorldDilation = FMath::Max(
                UGameplayStatics::GetGlobalTimeDilation(Director), KINDA_SMALL_NUMBER);
            Agent->BodyMesh->SetPlayRate(ClockScale / WorldDilation);
        }
    };
    PrepareAgent(OlofAgent.Get(), OlofShotAnimation, OlofStartAnchorId, this);
    PrepareAgent(KillerAgent.Get(), KillerShotAnimation, KillerStartAnchorId, this);
    SequenceTime = 0.0f;
    bFirstShotFired = false;
    bSecondShotFired = false;
    bSequenceActive = true;
}

FVector ATMOPPalmeShotDirector::GetMuzzleLocation() const
{
    if (!KillerAgent.IsValid() || !IsValid(KillerAgent->BodyMesh))
        return GetActorLocation();
    return KillerAgent->BodyMesh->DoesSocketExist(KillerMuzzleSocket)
        ? KillerAgent->BodyMesh->GetSocketLocation(KillerMuzzleSocket)
        : KillerAgent->BodyMesh->GetComponentLocation();
}

void ATMOPPalmeShotDirector::SpawnEffect(UNiagaraSystem* Effect,
    const FVector& Location, const FVector& Direction)
{
    if (!IsValid(Effect) || GetWorld() == nullptr) return;
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, Location,
        Direction.Rotation(), FVector::OneVector, true, true,
        ENCPoolMethod::AutoRelease, true);
}

void ATMOPPalmeShotDirector::SpawnTrail(const FVector& Start, const FVector& End)
{
    if (!IsValid(BulletLightTrailEffect) || GetWorld() == nullptr) return;
    UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), BulletLightTrailEffect, Start, (End - Start).Rotation(),
        FVector::OneVector, true, false, ENCPoolMethod::AutoRelease, true);
    if (IsValid(Trail))
    {
        Trail->SetVariableVec3(TEXT("User.Start"), Start);
        Trail->SetVariableVec3(TEXT("User.End"), End);
        Trail->Activate(true);
    }
}

void ATMOPPalmeShotDirector::FireFirstShot()
{
    bFirstShotFired = true;
    if (bFirstShotUsesWallRoute) FireWallRound(FirstShotSound);
    else FireSnowRound(FirstShotSound);
}

void ATMOPPalmeShotDirector::FireSecondShot()
{
    bSecondShotFired = true;
    if (bFirstShotUsesWallRoute) FireSnowRound(SecondShotSound);
    else FireWallRound(SecondShotSound);
}

void ATMOPPalmeShotDirector::FireWallRound(USoundBase* Sound)
{
    const FVector Start = GetMuzzleLocation();
    const FVector Wall = ResolveAnchorLocation(FirstShotWallImpactAnchorId,
        Start + KillerAgent->GetActorForwardVector() * 1000.0f);
    SpawnTrail(Start, Wall);
    SpawnEffect(MuzzleSmokeEffect, Start, Wall - Start);
    SpawnEffect(WallImpactEffect, Wall, Start - Wall);
    const FVector RicochetEnd = ResolveAnchorLocation(FirstShotRicochetEndAnchorId, Wall);
    if (!RicochetEnd.Equals(Wall, 1.0f)) SpawnTrail(Wall, RicochetEnd);
    if (IsValid(Sound)) UGameplayStatics::PlaySoundAtLocation(this, Sound, Start);
}

void ATMOPPalmeShotDirector::FireSnowRound(USoundBase* Sound)
{
    const FVector Start = GetMuzzleLocation();
    const FVector Snow = ResolveAnchorLocation(SecondShotSnowImpactAnchorId,
        Start + KillerAgent->GetActorForwardVector() * 1200.0f);
    SpawnTrail(Start, Snow);
    SpawnEffect(MuzzleSmokeEffect, Start, Snow - Start);
    SpawnEffect(SnowImpactEffect, Snow, Start - Snow);
    if (IsValid(Sound)) UGameplayStatics::PlaySoundAtLocation(this, Sound, Start);
}

void ATMOPPalmeShotDirector::FinishSequence()
{
    if (!bSequenceActive) return;
    bSequenceActive = false;
    if (OlofAgent.IsValid())
    {
        OlofAgent->SetLifeState(ETMOPAgentLifeState::Dead);
        // Keep the final frame visible; the grounded AnimBP may take over later
        // when another system explicitly changes animation mode.
    }
    ShowBloodPool();
    if (KillerAgent.IsValid())
    {
        if (IsValid(KillerAgent->BodyMesh))
            KillerAgent->BodyMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        if (UCharacterMovementComponent* Movement = KillerAgent->GetCharacterMovement())
            Movement->SetMovementMode(MOVE_Walking);
        KillerAgent->SetActivityState(ETMOPAgentActivityState::Fleeing);
    }
}

void ATMOPPalmeShotDirector::ShowBloodPool()
{
    if (!IsValid(BloodPoolComponent) || !IsValid(BloodPoolMesh)) return;
    BloodPoolComponent->SetStaticMesh(BloodPoolMesh);
    FTransform BaseTransform = OlofAgent.IsValid()
        ? OlofAgent->GetActorTransform() : GetActorTransform();
    if (!BloodPoolAnchorId.IsNone() && GetGameInstance() != nullptr)
    {
        const UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(BloodPoolAnchorId) : nullptr;
        if (IsValid(Anchor)) BaseTransform = Anchor->GetActorTransform();
    }
    BloodPoolComponent->SetWorldTransform(BloodPoolLocalTransform * BaseTransform);
    BloodPoolComponent->SetHiddenInGame(false);
}

void ATMOPPalmeShotDirector::ResetSequence()
{
    RestoreSlowMotion();
    const auto RestoreAgent = [](ATMOPHistoricalAgent* Agent)
    {
        if (!IsValid(Agent)) return;
        if (IsValid(Agent->BodyMesh))
            Agent->BodyMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        if (UCharacterMovementComponent* Movement = Agent->GetCharacterMovement())
            Movement->SetMovementMode(MOVE_Walking);
    };
    RestoreAgent(OlofAgent.Get());
    RestoreAgent(KillerAgent.Get());
    bSequenceActive = false;
    bFirstShotFired = false;
    bSecondShotFired = false;
    SequenceTime = 0.0f;
    bSlowMotionEvaluated = false;
    SlowMotionEndTimeSeconds = INDEX_NONE;
    if (IsValid(BloodPoolComponent))
    {
        BloodPoolComponent->SetHiddenInGame(true);
        BloodPoolComponent->SetStaticMesh(nullptr);
    }
    OlofAgent.Reset();
    KillerAgent.Reset();
}
