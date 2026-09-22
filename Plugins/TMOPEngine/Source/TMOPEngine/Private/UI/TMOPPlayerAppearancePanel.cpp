#include "UI/TMOPPlayerAppearancePanel.h"
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
        ChildSlot[SNew(STextBlock).Text(FText::FromString(TEXT("Starta spelet först för att ändra din spelare.")))];
        return;
    }
    Opening = Accepted = Draft;
    RebuildOptions(false);
    PreviewBrush.ImageSize = FVector2D(320, 580);
    PreviewBrush.DrawAs = ESlateBrushDrawType::Image;
    TSharedRef<SVerticalBox> Controls = SNew(SVerticalBox);
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock)
        .Text(FText::FromString(FString::Printf(TEXT("Player appearance — spelare %d"), Director->PlayerIndex + 1)))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text_Lambda([this] {
        return FText::FromString(Draft.Gender == ETMOPPersonGender::Female ? TEXT("Kön: kvinna / Quinn") : TEXT("Kön: man / Manny"));
    }).OnClicked(this, &STMOPPlayerAppearancePanel::ChangeGender)];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock).Text(FText::FromString(TEXT("Längd (cm)")))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SSpinBox<float>).MinValue(120).MaxValue(205).Delta(1)
        .Value_Lambda([this] { return Draft.GetResolvedHeightCentimeters(); })
        .OnValueCommitted_Lambda([this](float V, ETextCommit::Type) {
            Draft.AppearanceProfile.HeightOverrideCentimeters = FMath::Clamp(V, 120.f, 205.f); Apply();
        })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text_Lambda([this] {
        return FText::FromString(TEXT("Kroppstyp: ") + StaticEnum<ETMOPBodyBuild>()->GetNameStringByValue(static_cast<int64>(Draft.GetResolvedBodyBuild())));
    }).OnClicked(this, &STMOPPlayerAppearancePanel::ChangeBuild)];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text_Lambda([this] {
        return FText::FromString(TEXT("Hårfärg: ") + StaticEnum<ETMOPHairColor>()->GetNameStringByValue(static_cast<int64>(Draft.HairColorCategory)));
    }).OnClicked(this, &STMOPPlayerAppearancePanel::ChangeColor)];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::UpperBody, TEXT("Tröja"))];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::Glasses, TEXT("Glasögon"))];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::FacialHair, TEXT("Skägg/mustasch"))];
    Controls->AddSlot().AutoHeight().Padding(4)[PartRow(ETMOPAppearancePartType::Scarf, TEXT("Halsduk"))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FText::FromString(TEXT("Rotera förhandsvisning 45°")))
        .OnClicked_Lambda([this] { PreviewYaw += 45; RefreshPreview(); return FReply::Handled(); })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text_Lambda([this] {
        return FText::FromString(bStartup ? (Ready.Get(false) ? TEXT("Klar ✓ – väntar på övriga") : TEXT("Klar")) : TEXT("Spara mitt utseende"));
    })
        .OnClicked_Lambda([this] {
            if (bStartup)
            {
                if (Apply()) OnReady.ExecuteIfBound();
                return FReply::Handled();
            }
            if (Apply() && Director.IsValid() && Director->SaveEditedProfile(Status)) {
                Opening = Draft; Status = TEXT("Sparat för denna lokalspelare.");
            }
            return FReply::Handled();
        })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(SButton).Text(FText::FromString(TEXT("Återställ till när menyn öppnades")))
        .OnClicked_Lambda([this] { Draft = Opening; RebuildOptions(false); Apply(); return FReply::Handled(); })];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true)
        .Text(FText::FromString(bStartup
            ? TEXT("Tryck Klar när du är nöjd. Spelet startar först när alla är klara. Ändringar efter Klar kräver ny bekräftelse.")
            : TEXT("Ändringar visas direkt. Spara för nästa spelstart. Längd ändrar utseendet, inte spelkollisionen. Strong och Athletic delar grundform.")))];
    Controls->AddSlot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true)
        .Text_Lambda([this] { return FText::FromString(Status); })];

    TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
    Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(110, 0, 320, 580))[SNew(SImage).Image(&PreviewBrush)];
    struct FPosition { ETMOPAppearancePartType Type; const TCHAR* Label; float Y; };
    const FPosition Positions[] = {
        {ETMOPAppearancePartType::Headwear, TEXT("Hatt/mössa"), 8},
        {ETMOPAppearancePartType::Hair, TEXT("Hår"), 54},
        {ETMOPAppearancePartType::Face, TEXT("Ansikte"), 100},
        {ETMOPAppearancePartType::Outerwear, TEXT("Jacka"), 235},
        {ETMOPAppearancePartType::Gloves, TEXT("Handskar"), 320},
        {ETMOPAppearancePartType::Trousers, TEXT("Byxor"), 415},
        {ETMOPAppearancePartType::Footwear, TEXT("Skor"), 532}
    };
    for (const auto& P : Positions)
    {
        Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(10, P.Y, 90, 40))[SNew(SButton).Text(FText::FromString(FString(TEXT("◀ ")) + P.Label))
            .OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, P.Type, -1)];
        Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(440, P.Y, 40, 40))[SNew(SButton).Text(FText::FromString(TEXT("▶")))
            .OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, P.Type, 1)];
        Canvas->AddSlot().Alignment(FVector2D::ZeroVector).Offset(FMargin(485, P.Y, 160, 44))[SNew(STextBlock).AutoWrapText(true)
            .Text_Lambda([this, Type=P.Type] { return PartLabel(Type); })];
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

TSharedRef<SWidget> STMOPPlayerAppearancePanel::PartRow(ETMOPAppearancePartType Type, const FString& Label)
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Label))]
        + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("◀"))).OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, Type, -1)]
            + SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this, Type] { return PartLabel(Type); })]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("▶"))).OnClicked(this, &STMOPPlayerAppearancePanel::Cycle, Type, 1)]];
}

FText STMOPPlayerAppearancePanel::PartLabel(ETMOPAppearancePartType Type) const
{
    auto Copy = Draft;
    const auto& Part = ATMOPPlayerAppearanceDirector::Choice(Copy, Type);
    return FText::FromString(Part.bHidden ? TEXT("Inget") : Part.CatalogId.IsNone() ? TEXT("Automatiskt") : Part.CatalogId.ToString());
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
    if (Status.IsEmpty()) Status = TEXT("Förhandsvisning uppdaterad — tryck Spara för att behålla efter omstart.");
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
    else Status = TEXT("Ingen synlig karaktärsmesh hittades. Kontrollera spelarens appearance-assets.");
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
