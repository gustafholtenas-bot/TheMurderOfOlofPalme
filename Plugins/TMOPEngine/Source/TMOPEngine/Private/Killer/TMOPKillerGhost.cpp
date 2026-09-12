#include "Killer/TMOPKillerGhost.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ATMOPKillerGhost::ATMOPKillerGhost()
{
    PrimaryActorTick.bCanEverTick = false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("GhostRoot")));
    SetActorEnableCollision(false);
    Tags.Add(TEXT("TMOP_KILLER_GHOST"));
}

bool ATMOPKillerGhost::InitializeVisual(AActor* Source, UAnimSequence* Animation,
    UMaterialInterface* Material, const FLinearColor& Color)
{
    const auto* Original = Cast<ATMOPHistoricalAgent>(Source);
    const auto* Template = Cast<ATMOPKillerGhost>(Source);
    // Never copy a suspect or another historical person into this system.
    if (!IsValid(Source) || !IsValid(Animation) || !IsValid(Material) ||
        (!Template && (!Original || !IsValid(Original->EntityIdentity.Get()) ||
            Original->EntityIdentity->EntityId != FName(TEXT("THE_KILLER"))))) return false;
    USkeletalMeshComponent* SourceBody = Template ? Template->Body.Get() : Original->GetMesh();
    if (!IsValid(SourceBody) || !IsValid(SourceBody->GetSkeletalMeshAsset())) return false;
    GroundOffset = Template ? Template->GroundOffset : Original->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    SetActorScale3D(Source->GetActorScale3D());
    RunAnimation = Animation;
    GhostMaterial = UMaterialInstanceDynamic::Create(Material, this);
    if (!IsValid(GhostMaterial)) return false;
    GhostMaterial->SetVectorParameterValue(TEXT("GhostColor"), Color);

    TMap<const USceneComponent*, USkeletalMeshComponent*> Copies;
    TArray<USkeletalMeshComponent*> Parts;
    Source->GetComponents<USkeletalMeshComponent>(Parts);
    Parts.Remove(SourceBody);
    Parts.Insert(SourceBody, 0);
    for (USkeletalMeshComponent* Part : Parts)
    {
        if (!IsValid(Part) || !IsValid(Part->GetSkeletalMeshAsset()) ||
            (Part != SourceBody && !Part->IsVisible())) continue;
        auto* Copy = NewObject<USkeletalMeshComponent>(this);
        AddInstanceComponent(Copy);
        Copy->SetupAttachment(GetRootComponent());
        Copy->SetRelativeTransform(Part->GetComponentTransform().GetRelativeTransform(Source->GetActorTransform()));
        Copy->SetSkeletalMeshAsset(Part->GetSkeletalMeshAsset());
        Copy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Copy->SetCanEverAffectNavigation(false);
        Copy->SetCastShadow(false);
        Copy->SetVisibility(Part->IsVisible());
        for (int32 Slot = 0; Slot < Copy->GetNumMaterials(); ++Slot) Copy->SetMaterial(Slot, GhostMaterial);
        Copy->RegisterComponent();
        Copies.Add(Part, Copy);
        if (Part == SourceBody)
        {
            Body = Copy;
            Body->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            Body->PlayAnimation(Animation, true);
            Body->SetComponentTickEnabled(false);
            Body->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        }
        else Copy->SetLeaderPoseComponent(Body);
    }
    TArray<UStaticMeshComponent*> Props;
    Source->GetComponents<UStaticMeshComponent>(Props);
    for (UStaticMeshComponent* Prop : Props)
    {
        if (!IsValid(Prop) || !IsValid(Prop->GetStaticMesh()) || !Prop->IsVisible()) continue;
        auto* Copy = NewObject<UStaticMeshComponent>(this);
        AddInstanceComponent(Copy);
        if (USkeletalMeshComponent** Parent = Copies.Find(Prop->GetAttachParent()))
        {
            Copy->SetupAttachment(*Parent, Prop->GetAttachSocketName());
            Copy->SetRelativeTransform(Prop->GetRelativeTransform());
        }
        else
        {
            Copy->SetupAttachment(GetRootComponent());
            Copy->SetRelativeTransform(Prop->GetComponentTransform().GetRelativeTransform(Source->GetActorTransform()));
        }
        Copy->SetStaticMesh(Prop->GetStaticMesh());
        Copy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Copy->SetCanEverAffectNavigation(false);
        Copy->SetCastShadow(false);
        for (int32 Slot = 0; Slot < Copy->GetNumMaterials(); ++Slot) Copy->SetMaterial(Slot, GhostMaterial);
        Copy->RegisterComponent();
    }
    SampleAnimation(0.0, 1.0f, 0.0f);
    return IsValid(Body);
}

void ATMOPKillerGhost::SampleAnimation(double ElapsedSeconds, float PlayRate, float Opacity)
{
    if (IsValid(GhostMaterial)) GhostMaterial->SetScalarParameterValue(TEXT("GhostOpacity"), FMath::Clamp(Opacity, 0.0f, 1.0f));
    if (!IsValid(Body) || !IsValid(RunAnimation)) return;
    if (UAnimSingleNodeInstance* Single = Body->GetSingleNodeInstance())
    {
        const double Length = FMath::Max(0.01, static_cast<double>(RunAnimation->GetPlayLength()));
        const float Position = static_cast<float>(FMath::Fmod(FMath::Max(0.0, ElapsedSeconds) * PlayRate, Length));
        Single->SetPosition(Position, false); // No animation notifies / duplicate historical actions.
        Body->TickAnimation(0.0f, false);
        Body->RefreshBoneTransforms();
    }
}
