#include "UI/TMOPPlayerAppearancePanel.h"
#include "Localization/TMOPLocalization.h"
#include "People/TMOPPlayerAppearanceDirector.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/MorphTarget.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void STMOPPlayerAppearancePanel::Construct(const FArguments& Args)
{
    Player = Args._Player;
    bStartup = Args._Startup;
    Ready = Args._Ready;
    OnEdited = Args._OnEdited;
    OnReady = Args._OnReady;
    Director = ATMOPPlayerAppearanceDirector::ForCharacter(Player.Get());
    if (!Director.IsValid() || !Director->GetEditableProfile(Draft))
    {
        ChildSlot[SNew(STextBlock).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.d6871bfa183a1b9f", "Starta spelet först för att ändra din spelare.")))];
        return;
    }
    Opening = Accepted = Draft;
    RebuildOptions(false);
    PreviewBrush.ImageSize = FVector2D(320, 580);
    PreviewBrush.DrawAs = ESlateBrushDrawType::Image;
    TSharedRef<SVerticalBox> Controls = SNew(SVerticalBox);
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock)
        .Text(FTMOPLocalization::Text(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.779a0a01fd87dfda", "Player appearance — spelare {0}"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Director->PlayerIndex + 1)))))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FTMOPLocalization::Bind([this] {
        return FText::FromString(Draft.Gender == ETMOPPersonGender::Female ? NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.2d3006ee4dca6da2", "Kön: kvinna / Quinn").ToString() : NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.9d1b9a3849eecfc7", "Kön: man / Manny").ToString());
    })).OnClicked(this, &STMOPPlayerAppearancePanel::ChangeGender)];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.7ae7a35ff52d6103", "Längd (cm)")))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SSpinBox<float>).MinValue(120).MaxValue(205).Delta(1)
        .Value_Lambda([this] { return Draft.GetResolvedHeightCentimeters(); })
        .OnValueCommitted_Lambda([this](float V, ETextCommit::Type) {
            Draft.AppearanceProfile.HeightOverrideCentimeters = FMath::Clamp(V, 120.f, 205.f); Apply();
        })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FTMOPLocalization::Bind([this] {
        return FText::FromString(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.b704ca5ff0017d63", "Kroppstyp: ").ToString() + StaticEnum<ETMOPBodyBuild>()->GetNameStringByValue(static_cast<int64>(Draft.GetResolvedBodyBuild())));
    })).OnClicked(this, &STMOPPlayerAppearancePanel::ChangeBuild)];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FTMOPLocalization::Bind([this] {
        return FText::FromString(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.b9d6062ead253e51", "Hårfärg: ").ToString() + StaticEnum<ETMOPHairColor>()->GetNameStringByValue(static_cast<int64>(Draft.HairColorCategory)));
    })).OnClicked(this, &STMOPPlayerAppearancePanel::ChangeColor)];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::UpperBody, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.7092e5f57ef437bf", "Tröja"))];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::Glasses, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.49345c1872c246e0", "Glasögon"))];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::FacialHair, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.056c6a86b897adaf", "Skägg/mustasch"))];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::Scarf, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.0d474ad3c36de3d1", "Halsduk"))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.a25564d5ff9d9f9e", "Rotera förhandsvisning 45°")))
        .OnClicked_Lambda([this] { PreviewYaw += 45; RefreshPreview(); return FReply::Handled(); })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FTMOPLocalization::Bind([this] {
        return FText::FromString(bStartup ? (Ready.Get(false) ? NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.8176674684dcb640", "Klar ✓ – väntar på övriga").ToString() : NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.adb8011c80b7ccd0", "Klar").ToString()) : NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.1c23ea2e9c2161f4", "Spara mitt utseende").ToString());
    }))
        .OnClicked_Lambda([this] {
            if (bStartup)
            {
                if (Apply()) OnReady.ExecuteIfBound();
                return FReply::Handled();
            }
            if (Apply() && Director.IsValid() && Director->SaveEditedProfile(Status)) {
                Opening = Draft; Status = NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.b780234f28185295", "Sparat för denna lokalspelare.").ToString();
            }
            return FReply::Handled();
        })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.25b8be7bdf9175e6", "Återställ till när menyn öppnades")))
        .OnClicked_Lambda([this] { Draft = Opening; RebuildOptions(false); Apply(); return FReply::Handled(); })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true)
        .Text(FTMOPLocalization::Text(FText::FromString(bStartup
            ? NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.631c2f59ce338425", "Tryck Klar när du är nöjd. Spelet startar först när alla är klara. Ändringar efter Klar kräver ny bekräftelse.").ToString()
            : NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.1fd017ad05248ca2", "Ändringar visas direkt. Spara för nästa spelstart. Längd ändrar utseendet, inte spelkollisionen. Strong och Athletic delar grundform.").ToString())))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true)
        .Text(FTMOPLocalization::Bind([this] { return FText::FromString(Status); }))];

    TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
    Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(110, 0, 320, 580))[SNew(SImage).Image(&PreviewBrush)];
    struct FPosition { ETMOPAppearancePartType Type; FText Label; float Y; };
    const FPosition Positions[] = {
        {ETMOPAppearancePartType::Headwear, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.982bde2ea354fe6e", "Hatt/mössa"), 8},
        {ETMOPAppearancePartType::Hair, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.512b02ac8fdf5977", "Hår"), 54},
        {ETMOPAppearancePartType::Face, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.24b285d6f4a63b48", "Ansikte"), 100},
        {ETMOPAppearancePartType::Outerwear, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.4bcb97a3ca987946", "Jacka"), 235},
        {ETMOPAppearancePartType::Gloves, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.8b1c33292fa14955", "Handskar"), 320},
        {ETMOPAppearancePartType::Trousers, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.95d1a82001682ef5", "Byxor"), 415},
        {ETMOPAppearancePartType::Footwear, NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.1aa3387983b1f870", "Skor"), 532}
    };
    for (const auto& P : Positions)
    {
        Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(10, P.Y, 90, 40))[SNew(SButton).Text(FTMOPLocalization::Text(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "AppearancePreviousPart", "◀ {0}"), P.Label)))
            .OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, P.Type, -1)];
        Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(440, P.Y, 40, 40))[SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("▶"))))
            .OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, P.Type, 1)];
        Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(485, P.Y, 160, 44))[SNew(STextBlock).AutoWrapText(true)
            .Text(FTMOPLocalization::Bind([this, Type=P.Type] { return PartLabel(Type); }))];
    }
    ChildSlot[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.025f,0.025f,0.03f,1)).Padding(12)
        [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
        [SNew(SBox).WidthOverride(930).HeightOverride(680)
        [SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,16,0)[SNew(SBox).WidthOverride(260)
            [SNew(SScrollBox) + SScrollBox::Slot()[Controls]]]
        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
            [SNew(SBox).HeightOverride(590)[Canvas]]]]]];
    RefreshPreview();
}

TSharedRef<SWidget> STMOPPlayerAppearancePanel::PartRow(ETMOPAppearancePartType Type, const FText& Label)
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FTMOPLocalization::Text(Label))]
        + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("◀")))).OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, Type, -1)]
            + SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).AutoWrapText(true).Text(FTMOPLocalization::Bind([this, Type] { return PartLabel(Type); }))]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("▶")))).OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, Type, 1)]];
}

FText STMOPPlayerAppearancePanel::PartLabel(ETMOPAppearancePartType Type) const
{
    auto Copy = Draft;
    const auto& Part = ATMOPPlayerAppearanceDirector::Choice(Copy, Type);
    return FText::FromString(Part.bHidden ? NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.5255869d3a92ab2d", "Inget").ToString() : Part.CatalogId.IsNone() ? NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.3f19051c2214a332", "Automatiskt").ToString() : Part.CatalogId.ToString());
}

void STMOPPlayerAppearancePanel::RebuildOptions(bool bRepair)
{
    if (!Director.IsValid()) return;
    for (int32 I=1; I<=static_cast<int32>(ETMOPAppearancePartType::Glasses); ++I)
    {
        auto Type = static_cast<ETMOPAppearancePartType>(I);
        auto& List = Options.FindOrAdd(Type);
        Director->GetSelectableAssets(Draft, Type, List);
        auto& Part = ATMOPPlayerAppearanceDirector::Choice(Draft, Type);
        if (bRepair && (!Part.MeshOverride.IsNull() || !Part.StaticMeshOverride.IsNull() ||
            (!Part.CatalogId.IsNone() && !List.Contains(Part.CatalogId))))
        {
            Part = FTMOPAppearancePartChoice();
            if (List.IsEmpty()) Part.bHidden = true;
            else Part.CatalogId = List[0];
        }
    }
}

bool STMOPPlayerAppearancePanel::Apply()
{
    if (!Director.IsValid()) return false;
    Draft.AppearanceProfile.GenerationMode = ETMOPAppearanceGenerationMode::Manual;
    Draft.AppearanceProfile.bUseMetaHumanHybridHead = false;
    if (!Director->PreviewProfile(Draft, Status))
    { Draft = Accepted; RebuildOptions(false); return false; }
    Accepted = Draft;
    OnEdited.ExecuteIfBound();
    if (Status.IsEmpty()) Status = NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.aff251c9d533b592", "Förhandsvisning uppdaterad — tryck Spara för att behålla efter omstart.").ToString();
    RefreshPreview();
    return true;
}

FReply STMOPPlayerAppearancePanel::Cycle(ETMOPAppearancePartType Type, int32 Direction)
{
    const auto* List = Options.Find(Type);
    if (!List) return FReply::Handled();
    auto& Part = ATMOPPlayerAppearanceDirector::Choice(Draft, Type);
    // Index zero is deliberately empty; there is no random fallback on removal.
    int32 Index = Part.bHidden ? 0 : List->IndexOfByKey(Part.CatalogId) + 1;
    Index = (Index + Direction + List->Num() + 1) % (List->Num() + 1);
    Part = FTMOPAppearancePartChoice();
    Part.bHidden = Index == 0;
    if (Index > 0) Part.CatalogId = (*List)[Index - 1];
    Apply();
    return FReply::Handled();
}

FReply STMOPPlayerAppearancePanel::ChangeGender()
{
    Draft.Gender = Draft.Gender == ETMOPPersonGender::Female ? ETMOPPersonGender::Male : ETMOPPersonGender::Female;
    Draft.AppearanceProfile.Body = FTMOPAppearancePartChoice();
    Draft.AppearanceProfile.bUseMetaHumanHybridHead = false;
    RebuildOptions(true); Apply(); return FReply::Handled();
}
FReply STMOPPlayerAppearancePanel::ChangeBuild()
{
    Draft.AppearanceProfile.BodyBuildOverride = static_cast<ETMOPBodyBuild>(static_cast<int32>(Draft.GetResolvedBodyBuild()) % 6 + 1);
    Draft.AppearanceProfile.BodyWeightMorph = 0;
    Draft.AppearanceProfile.MuscularityMorph = 0;
    RebuildOptions(true); Apply(); return FReply::Handled();
}
FReply STMOPPlayerAppearancePanel::ChangeColor()
{
    Draft.HairColorCategory = static_cast<ETMOPHairColor>(static_cast<int32>(Draft.HairColorCategory) % 7 + 1);
    Draft.Hair = FTMOPAppearanceSlot();
    Draft.AppearanceProfile.Hair.MaterialOverride.Reset();
    Apply(); return FReply::Handled();
}

STMOPPlayerAppearancePanel::~STMOPPlayerAppearancePanel() { DestroyPreview(); }
void STMOPPlayerAppearancePanel::DestroyPreview()
{
    PendingCapture.Reset();
    if (PreviewActor.IsValid()) PreviewActor->Destroy();
    PreviewActor.Reset();
}

void STMOPPlayerAppearancePanel::RefreshPreview()
{
    DestroyPreview();
    if (!Player.IsValid() || !Player->GetWorld()) return;
    UWorld* World = Player->GetWorld();
    // Appearance changes must render while the startup world is paused.
    // Evaluate only the visual pose with zero elapsed time; do not tick movement.
    if (USkeletalMeshComponent* Body = Player->GetMesh())
    {
        Body->TickAnimation(0.0f, false);
        Body->RefreshBoneTransforms();
    }
    if (!Target.IsValid())
    {
        Target.Reset(NewObject<UTextureRenderTarget2D>());
        Target->ClearColor = FLinearColor(0.02f, 0.02f, 0.025f, 1);
        Target->InitAutoFormat(512, 928);
        PreviewBrush.SetResourceObject(Target.Get());
    }
    PreviewActor.Reset(World->SpawnActor<AActor>());
    if (!PreviewActor.IsValid()) return;
    auto* Root = NewObject<USceneComponent>(PreviewActor.Get());
    PreviewActor->AddInstanceComponent(Root);
    PreviewActor->SetRootComponent(Root);
    Root->RegisterComponent();
    PreviewActor->SetActorLocation(Player->GetActorLocation() + FVector(0, 0, 10000));
    PreviewActor->SetActorRotation(FRotator(0, PreviewYaw, 0));
    auto* Capture = NewObject<USceneCaptureComponent2D>(PreviewActor.Get());
    PreviewActor->AddInstanceComponent(Capture);
    Capture->SetupAttachment(Root);
    Capture->TextureTarget = Target.Get();
    Capture->bCaptureEveryFrame = false;
    Capture->bCaptureOnMovement = false;
    Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    Capture->ProjectionType = ECameraProjectionMode::Perspective;
    Capture->FOVAngle = 30;
    Capture->ShowFlags.SetLighting(false);
    Capture->ShowFlags.SetAtmosphere(false);
    Capture->ShowFlags.SetFog(false);
    Capture->RegisterComponent();
    // Camera remains in front while the mannequin root rotates.
    Capture->SetWorldLocation(PreviewActor->GetActorLocation() + FVector(400, 0, 0));
    Capture->SetWorldRotation(FRotator(0, 180, 0));
    TArray<UMeshComponent*> Sources;
    FBox PreviewBounds(ForceInit);
    Player->GetComponents<UMeshComponent>(Sources);
    for (UMeshComponent* Source : Sources)
    {
        if (!Source || !Source->IsVisible() || Source->bHiddenInGame) continue;
        if (Source != Player->GetMesh() && !Source->GetName().StartsWith(TEXT("TMOP_Player_"))) continue;
        UMeshComponent* Copy = nullptr;
        if (auto* Skeletal = Cast<USkeletalMeshComponent>(Source))
        {
            if (!Skeletal->GetSkeletalMeshAsset()) continue;
            auto* NewMesh = NewObject<USkeletalMeshComponent>(PreviewActor.Get());
            NewMesh->SetSkeletalMesh(Skeletal->GetSkeletalMeshAsset());
            // Independent reference pose: never depend on a hidden/paused in-world leader.
            NewMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            for (UMorphTarget* Morph : Skeletal->GetSkeletalMeshAsset()->GetMorphTargets())
                if (Morph) NewMesh->SetMorphTarget(Morph->GetFName(), Skeletal->GetMorphTarget(Morph->GetFName()));
            Copy = NewMesh;
        }
        else if (auto* Static = Cast<UStaticMeshComponent>(Source))
        {
            if (!Static->GetStaticMesh()) continue;
            auto* NewMesh = NewObject<UStaticMeshComponent>(PreviewActor.Get());
            NewMesh->SetStaticMesh(Static->GetStaticMesh());
            Copy = NewMesh;
        }
        if (!Copy) continue;
        PreviewActor->AddInstanceComponent(Copy);
        Copy->SetupAttachment(Root);
        Copy->SetRelativeTransform(Source->GetComponentTransform().GetRelativeTransform(Player->GetActorTransform()));
        for (int32 I=0; I<Source->GetNumMaterials(); ++I) Copy->SetMaterial(I, Source->GetMaterial(I));
        Copy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Copy->SetCastShadow(false);
        Copy->SetOnlyOwnerSee(false);
        Copy->SetOwnerNoSee(false);
        Copy->SetVisibleInSceneCaptureOnly(true);
        Copy->RegisterComponent();
        if (auto* SkinnedCopy = Cast<USkeletalMeshComponent>(Copy))
            SkinnedCopy->RefreshBoneTransforms();
        Capture->ShowOnlyComponent(Copy);
        Copy->UpdateBounds();
        PreviewBounds += Copy->Bounds.GetBox();
    }
    if (PreviewBounds.IsValid)
    {
        const FVector Extent = PreviewBounds.GetExtent();
        const double Aspect = 512.0/928.0;
        const double Distance = Extent.X + 1.2 * FMath::Max(Extent.Y, Extent.Z * Aspect) / FMath::Tan(FMath::DegreesToRadians(15.0));
        Capture->SetWorldLocation(PreviewBounds.GetCenter() + FVector(FMath::Max(100.0, Distance),0,0));
    }
    else Status = NSLOCTEXT("TMOP", "TMOPPlayerAppearancePanel.78cad915e75a282f", "Ingen synlig karaktärsmesh hittades. Kontrollera spelarens appearance-assets.").ToString();
    PendingCapture = Capture;
    CaptureDelayFrames = 2;
}

void STMOPPlayerAppearancePanel::Tick(const FGeometry& Geometry, double CurrentTime, float DeltaTime)
{
    SCompoundWidget::Tick(Geometry, CurrentTime, DeltaTime);
    // Slate continues while the world is paused. Let new mesh render states finish first.
    if (PendingCapture.IsValid() && --CaptureDelayFrames <= 0)
    {
        if (UWorld* World = PendingCapture->GetWorld()) World->SendAllEndOfFrameUpdates();
        PendingCapture->CaptureScene();
        PendingCapture.Reset();
    }
}
