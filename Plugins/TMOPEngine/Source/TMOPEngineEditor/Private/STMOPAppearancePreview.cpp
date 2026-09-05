#include "STMOPAppearancePreview.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Animation/AnimSequence.h"
#include "Components/AudioComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/TextureCube.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "People/TMOPCharacterAppearanceComponent.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "PropertyCustomizationHelpers.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "Vehicles/TMOPVehiclePresentation.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "TMOPAppearancePreview"

namespace TMOPAppearancePreview
{
UWorld* EditorWorld() { return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr; }
UDataTable* PeopleTable()
{
    if (UWorld* World = EditorWorld())
        for (TActorIterator<ATMOPPersonRegistryDirector> It(World); It; ++It)
            if (IsValid(It->PersonProfileTable)) return It->PersonProfileTable;
    return LoadObject<UDataTable>(nullptr, TEXT("/Game/TMOP/Agents/People/DT_TMOP_People.DT_TMOP_People"));
}
UDataTable* VehicleTable()
{
    if (UWorld* World = EditorWorld())
        for (TActorIterator<ATMOPHistoricalVehicleDirector> It(World); It; ++It)
            if (IsValid(It->HistoricalVehicleTable)) return It->HistoricalVehicleTable;
    return LoadObject<UDataTable>(nullptr, TEXT("/Game/TMOP/Vehicles/DT_TMOP_HistoricalVehicles.DT_TMOP_HistoricalVehicles"));
}
UDataTable* AppearanceTable()
{
    if (UWorld* World = EditorWorld())
        for (TActorIterator<ATMOPPersonRegistryDirector> It(World); It; ++It)
            if (IsValid(It->AppearanceAssetTable)) return It->AppearanceAssetTable;
    return LoadObject<UDataTable>(nullptr, TEXT("/Game/TMOP/Characters/Appearance/Data/DT_TMOP_AppearanceAssets.DT_TMOP_AppearanceAssets"));
}
UClass* DefaultPersonClass()
{
    if (UWorld* World = EditorWorld())
        for (TActorIterator<ATMOPPersonRegistryDirector> It(World); It; ++It)
            if (It->DefaultAgentClass) return It->DefaultAgentClass.Get();
    return nullptr;
}
UClass* DefaultVehicleClass()
{
    if (UWorld* World = EditorWorld())
        for (TActorIterator<ATMOPHistoricalVehicleDirector> It(World); It; ++It)
            if (It->DefaultVehicleClass) return It->DefaultVehicleClass.Get();
    return nullptr;
}
}

namespace
{
constexpr int32 AppearancePartCount = 12;

FTMOPAppearancePartChoice* FindAppearanceChoice(
    FTMOPAppearanceProfile& Profile, ETMOPAppearancePartType Type)
{
    switch (Type)
    {
    case ETMOPAppearancePartType::Body: return &Profile.Body;
    case ETMOPAppearancePartType::Face: return &Profile.Face;
    case ETMOPAppearancePartType::Hair: return &Profile.Hair;
    case ETMOPAppearancePartType::Outerwear: return &Profile.Outerwear;
    case ETMOPAppearancePartType::UpperBody: return &Profile.UpperBody;
    case ETMOPAppearancePartType::Trousers: return &Profile.Trousers;
    case ETMOPAppearancePartType::Footwear: return &Profile.Footwear;
    case ETMOPAppearancePartType::Gloves: return &Profile.Gloves;
    case ETMOPAppearancePartType::Headwear: return &Profile.Headwear;
    case ETMOPAppearancePartType::FacialHair: return &Profile.FacialHair;
    case ETMOPAppearancePartType::Scarf: return &Profile.Scarf;
    case ETMOPAppearancePartType::Glasses: return &Profile.Glasses;
    default: return nullptr;
    }
}

const FTMOPAppearancePartChoice* FindAppearanceChoice(
    const FTMOPAppearanceProfile& Profile, ETMOPAppearancePartType Type)
{
    switch (Type)
    {
    case ETMOPAppearancePartType::Body: return &Profile.Body;
    case ETMOPAppearancePartType::Face: return &Profile.Face;
    case ETMOPAppearancePartType::Hair: return &Profile.Hair;
    case ETMOPAppearancePartType::Outerwear: return &Profile.Outerwear;
    case ETMOPAppearancePartType::UpperBody: return &Profile.UpperBody;
    case ETMOPAppearancePartType::Trousers: return &Profile.Trousers;
    case ETMOPAppearancePartType::Footwear: return &Profile.Footwear;
    case ETMOPAppearancePartType::Gloves: return &Profile.Gloves;
    case ETMOPAppearancePartType::Headwear: return &Profile.Headwear;
    case ETMOPAppearancePartType::FacialHair: return &Profile.FacialHair;
    case ETMOPAppearancePartType::Scarf: return &Profile.Scarf;
    case ETMOPAppearancePartType::Glasses: return &Profile.Glasses;
    default: return nullptr;
    }
}

FText AppearancePartLabel(ETMOPAppearancePartType Type)
{
    switch (Type)
    {
    case ETMOPAppearancePartType::Body: return LOCTEXT("PartBody", "Body");
    case ETMOPAppearancePartType::Face: return LOCTEXT("PartFace", "Face");
    case ETMOPAppearancePartType::Hair: return LOCTEXT("PartHair", "Hair");
    case ETMOPAppearancePartType::Outerwear: return LOCTEXT("PartOuterwear", "Jacket / coat");
    case ETMOPAppearancePartType::UpperBody: return LOCTEXT("PartUpperBody", "Shirt / sweater");
    case ETMOPAppearancePartType::Trousers: return LOCTEXT("PartTrousers", "Trousers");
    case ETMOPAppearancePartType::Footwear: return LOCTEXT("PartFootwear", "Shoes");
    case ETMOPAppearancePartType::Gloves: return LOCTEXT("PartGloves", "Gloves");
    case ETMOPAppearancePartType::Headwear: return LOCTEXT("PartHeadwear", "Headwear");
    case ETMOPAppearancePartType::FacialHair: return LOCTEXT("PartFacialHair", "Facial hair");
    case ETMOPAppearancePartType::Scarf: return LOCTEXT("PartScarf", "Scarf");
    case ETMOPAppearancePartType::Glasses: return LOCTEXT("PartGlasses", "Glasses");
    default: return LOCTEXT("PartUnknown", "Part");
    }
}

const ETMOPAppearancePartType EditableAppearanceParts[] =
{
    ETMOPAppearancePartType::Body,
    ETMOPAppearancePartType::Face,
    ETMOPAppearancePartType::Hair,
    ETMOPAppearancePartType::Outerwear,
    ETMOPAppearancePartType::UpperBody,
    ETMOPAppearancePartType::Trousers,
    ETMOPAppearancePartType::Footwear,
    ETMOPAppearancePartType::Gloves,
    ETMOPAppearancePartType::Headwear,
    ETMOPAppearancePartType::FacialHair,
    ETMOPAppearancePartType::Scarf,
    ETMOPAppearancePartType::Glasses
};

TArray<AActor*> PreviewActors(AActor* Root)
{
    TArray<AActor*> Result;
    if (IsValid(Root))
    {
        Root->GetAllChildActors(Result, true);
        Result.Insert(Root, 0);
    }
    return Result;
}

FBox VisibleBounds(AActor* Actor)
{
    FBox Bounds(ForceInit);
    for (AActor* PartActor : PreviewActors(Actor))
    {
        TInlineComponentArray<UMeshComponent*> Parts(PartActor);
        for (UMeshComponent* Part : Parts)
            if (Part->IsVisible() && !Part->bHiddenInGame && !Cast<UWidgetComponent>(Part))
                Bounds += Part->Bounds.GetBox();
    }
    return Bounds;
}

void PreparePreviewActor(AActor* Actor)
{
    for (AActor* PartActor : PreviewActors(Actor))
    {
        PartActor->SetFlags(RF_Transient);
        PartActor->SetActorTickEnabled(false);
        PartActor->SetActorEnableCollision(false);
        TInlineComponentArray<UActorComponent*> Components(PartActor);
        for (UActorComponent* Component : Components)
        {
            Component->SetComponentTickEnabled(false);
            if (UAudioComponent* Audio = Cast<UAudioComponent>(Component)) Audio->Stop();
            if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
            {
                Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Primitive->SetGenerateOverlapEvents(false);
                Primitive->SetSimulatePhysics(false);
                if (!Cast<UMeshComponent>(Primitive) || Cast<UWidgetComponent>(Primitive))
                    Primitive->SetVisibility(false);
                else
                {
                    // Preserve runtime part visibility, but do not apply local-player hiding rules.
                    Primitive->SetVisibility(Primitive->IsVisible() && !Primitive->bHiddenInGame);
                    Primitive->SetOnlyOwnerSee(false);
                    Primitive->SetOwnerNoSee(false);
                    Primitive->SetCullDistance(0.0f);
                }
            }
        }
    }
}

template<typename T>
T* SpawnPreview(UWorld* World, UClass* Class)
{
    if (!World || !Class || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
        return nullptr;
    FActorSpawnParameters Params;
    Params.ObjectFlags = RF_Transient;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.bDeferConstruction = true;
    T* Actor = World->SpawnActor<T>(Class, FTransform::Identity, Params);
    if (APawn* Pawn = Cast<APawn>(Actor))
    {
        Pawn->AutoPossessAI = EAutoPossessAI::Disabled;
        Pawn->AutoPossessPlayer = EAutoReceiveInput::Disabled;
    }
    if (Actor) Actor->FinishSpawning(FTransform::Identity);
    return Actor;
}
}

class FTMOPAppearanceViewportClient final : public FEditorViewportClient
{
public:
    FTMOPAppearanceViewportClient(TSharedRef<FPreviewScene> InScene,
        const TWeakPtr<SEditorViewport>& Widget)
        : FEditorViewportClient(nullptr, &InScene.Get(), Widget), SceneOwner(InScene)
    {
        SetViewMode(VMI_Lit);
        SetViewportType(LVT_Perspective);
        SetRealtime(true);
        EngineShowFlags.SetGrid(false);
        EngineShowFlags.SetSelectionOutline(false);
        EngineShowFlags.SetCompositeEditorPrimitives(false);
        EngineShowFlags.SetMotionBlur(false);
        EngineShowFlags.SetTemporalAA(true);
        bSetListenerPosition = false;
        bUsingOrbitCamera = true;
        SetViewRotation(FRotator(-12.0f, -145.0f, 0.0f));
    }
    virtual FLinearColor GetBackgroundColor() const override
    { return FLinearColor(0.055f, 0.065f, 0.08f); }
    virtual bool GetPivotForOrbit(FVector& OutPivot) const override { OutPivot = Pivot; return true; }
    void FitActor(AActor* Actor)
    {
        FBox Bounds = VisibleBounds(Actor);
        if (!Bounds.IsValid) Bounds = FBox(FVector(-50,-50,0), FVector(50,50,180));
        Pivot = Bounds.GetCenter();
        FocusViewportOnBox(Bounds.ExpandBy(12.0), true);
        SetLookAtLocation(Pivot, false);
        Invalidate();
    }
    void Turn(float Degrees)
    {
        const FVector Offset = GetViewLocation() - Pivot;
        const FVector Location = Pivot + FRotator(0, Degrees, 0).RotateVector(Offset);
        SetViewLocation(Location);
        SetViewRotation((Pivot - Location).Rotation());
        SetLookAtLocation(Pivot);
        Invalidate();
    }
    void Zoom(float Factor)
    {
        const FVector Offset = GetViewLocation() - Pivot;
        SetViewLocation(Pivot + Offset.GetSafeNormal() * FMath::Clamp(Offset.Size() * Factor, 20.0, 20000.0));
        SetLookAtLocation(Pivot);
        Invalidate();
    }
private:
    TSharedRef<FPreviewScene> SceneOwner;
    FVector Pivot = FVector::ZeroVector;
};

void STMOPAppearanceViewport::Construct(const FArguments& Args)
{
    Scene = MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues()
        .SetEditor(true).SetTransactional(false).AllowAudioPlayback(false)
        .SetCreatePhysicsScene(true).ShouldSimulatePhysics(false)
        .SetLightRotation(FRotator(-35, -45, 0)).SetLightBrightness(3.0f).SetSkyBrightness(1.0f));
    // The editor preview sky supplies useful reflections for glass and metallic paint.
    if (UTextureCube* Sky = LoadObject<UTextureCube>(nullptr, TEXT("/Engine/MapTemplates/Sky/SunsetAmbientCubemap.SunsetAmbientCubemap")))
        Scene->SetSkyCubemap(Sky);
    Scene->UpdateCaptureContents();
    SEditorViewport::Construct(SEditorViewport::FArguments());
}
STMOPAppearanceViewport::~STMOPAppearanceViewport()
{
    ClearActor();
    if (PreviewClient) PreviewClient->Viewport = nullptr;
}
TSharedRef<FEditorViewportClient> STMOPAppearanceViewport::MakeEditorViewportClient()
{
    PreviewClient = MakeShared<FTMOPAppearanceViewportClient>(Scene.ToSharedRef(), SharedThis(this));
    return PreviewClient.ToSharedRef();
}
void STMOPAppearanceViewport::OnFocusViewportToSelection() { Fit(); }
UWorld* STMOPAppearanceViewport::GetPreviewWorld() const { return Scene ? Scene->GetWorld() : nullptr; }
AActor* STMOPAppearanceViewport::GetActor() const { return PreviewActor.Get(); }
void STMOPAppearanceViewport::ClearActor()
{
    if (PreviewActor.IsValid()) PreviewActor->Destroy();
    PreviewActor.Reset();
    if (PreviewClient) PreviewClient->Invalidate();
}
void STMOPAppearanceViewport::SetActor(AActor* Actor, bool bRefocus)
{
    PreviewActor = Actor;
    PreparePreviewActor(Actor);
    EvaluatePose(nullptr, 0.0f);
    if (bRefocus) Fit();
    if (PreviewClient) PreviewClient->Invalidate();
}
void STMOPAppearanceViewport::Fit() { if (PreviewClient) PreviewClient->FitActor(PreviewActor.Get()); }
void STMOPAppearanceViewport::Turn(float Degrees) { if (PreviewClient) PreviewClient->Turn(Degrees); }
void STMOPAppearanceViewport::Zoom(float Factor) { if (PreviewClient) PreviewClient->Zoom(Factor); }

void STMOPAppearanceViewport::EvaluatePose(UAnimSequence* Animation, float Seconds)
{
    for (AActor* PartActor : PreviewActors(PreviewActor.Get()))
    {
        TInlineComponentArray<USkeletalMeshComponent*> Parts(PartActor);
        for (USkeletalMeshComponent* Part : Parts)
        {
            if (!Part->GetSkeletalMeshAsset()) continue;
            Part->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            if (!Animation && !Part->LeaderPoseComponent.IsValid())
            {
                Part->SetAnimationMode(EAnimationMode::AnimationSingleNode);
                Part->SetAnimation(nullptr);
            }
            if (Animation && !Part->LeaderPoseComponent.IsValid() &&
                Animation->GetSkeleton() == Part->GetSkeletalMeshAsset()->GetSkeleton())
            {
                if (Part->GetAnimationMode() != EAnimationMode::AnimationSingleNode)
                    Part->SetAnimationMode(EAnimationMode::AnimationSingleNode);
                Part->SetAnimation(Animation);
                Part->SetPosition(Seconds, false);
                Part->TickAnimation(0.0f, false);
            }
            Part->RefreshBoneTransforms();
            Part->UpdateComponentToWorld();
            Part->MarkRenderTransformDirty();
            Part->MarkRenderDynamicDataDirty();
        }
    }
    if (PreviewClient) PreviewClient->Invalidate();
}

TArray<FName> STMOPAppearanceViewport::GetSockets() const
{
    TArray<FName> Result;
    if (ATMOPVehicleBase* Vehicle = Cast<ATMOPVehicleBase>(PreviewActor.Get()))
    {
        if (Vehicle->RoofAccessorySocket) Result.Add(TEXT("RoofAccessorySocket"));
        if (UMeshComponent* Body = TMOPVehiclePresentation::ResolveBodyMesh(Vehicle))
            for (FName Socket : Body->GetAllSocketNames()) Result.AddUnique(Socket);
    }
    if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(PreviewActor.Get()))
        if (Agent->BodyMesh) Result.Append(Agent->BodyMesh->GetAllSocketNames());
    Result.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });
    return Result;
}

void STMOPAppearancePreview::Construct(const FArguments& Args)
{
    OnPersonAppearanceChanged = Args._OnPersonAppearanceChanged;
    AppearanceOptions.SetNum(AppearancePartCount);
    AppearanceCombos.SetNum(AppearancePartCount);

    TSharedRef<SVerticalBox> SelectorRows = SNew(SVerticalBox);
    AppearanceSelectorBox = SelectorRows;
    for (const ETMOPAppearancePartType PartType : EditableAppearanceParts)
    {
        const int32 PartIndex = static_cast<int32>(PartType);
        SelectorRows->AddSlot().AutoHeight().Padding(0, 1)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(0.34f).VAlign(VAlign_Center)
            [ SNew(STextBlock).Text(AppearancePartLabel(PartType)) ]
            + SHorizontalBox::Slot().FillWidth(0.66f)
            [
                SAssignNew(AppearanceCombos[PartIndex],
                    SComboBox<TSharedPtr<FName>>)
                .OptionsSource(&AppearanceOptions[PartIndex])
                .OnGenerateWidget(this,
                    &STMOPAppearancePreview::GenerateAppearanceOption)
                .OnSelectionChanged(this,
                    &STMOPAppearancePreview::SelectAppearanceAsset, PartType)
                [ SNew(STextBlock).Text(this,
                    &STMOPAppearancePreview::GetAppearanceChoiceText,
                    PartType) ]
            ]
        ];
    }

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)
            [ SNew(STextBlock).Text(LOCTEXT("PreviewTitle", "3D APPEARANCE")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("Fit", "Fit")).OnClicked_Lambda([this]{ViewportWidget->Fit();return FReply::Handled();}) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("ReloadPreview", "Reload assets + preview"))
                .ToolTipText(LOCTEXT("ReloadPreviewTip", "Reload the appearance catalog choices and rebuild the preview from the current profile."))
                .OnClicked(this, &STMOPAppearancePreview::ReloadAppearanceAssets) ]
        ]
        + SVerticalBox::Slot().FillHeight(1).Padding(0,3)
        [ SAssignNew(ViewportWidget, STMOPAppearanceViewport) ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(FText::FromString(TEXT("<"))).ToolTipText(LOCTEXT("RotateLeft", "Rotate view left"))
                .OnClicked_Lambda([this]{ViewportWidget->Turn(-30);return FReply::Handled();}) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(FText::FromString(TEXT(">"))).ToolTipText(LOCTEXT("RotateRight", "Rotate view right"))
                .OnClicked_Lambda([this]{ViewportWidget->Turn(30);return FReply::Handled();}) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(FText::FromString(TEXT("-")))
                .OnClicked_Lambda([this]{ViewportWidget->Zoom(1.2f);return FReply::Handled();}) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(FText::FromString(TEXT("+")))
                .OnClicked_Lambda([this]{ViewportWidget->Zoom(1.0f/1.2f);return FReply::Handled();}) ]
            + SHorizontalBox::Slot().FillWidth(1).Padding(4,0)
            [ SNew(SEditableTextBox).Text(this,&STMOPAppearancePreview::TimeText)
                .Visibility_Lambda([this]{return PersonProfile.IsSet()?EVisibility::Visible:EVisibility::Collapsed;})
                .ToolTipText(LOCTEXT("PropTime","Preview time for time-limited held objects. Does not change the timeline."))
                .OnTextCommitted(this,&STMOPAppearancePreview::SetTime) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0,3)
        [ SNew(SHorizontalBox)
            .Visibility_Lambda([this]{return PersonProfile.IsSet()?EVisibility::Visible:EVisibility::Collapsed;})
            + SHorizontalBox::Slot().FillWidth(1)
            [ SNew(SObjectPropertyEntryBox).AllowedClass(UAnimSequence::StaticClass())
                .ObjectPath_Lambda([this]{return PreviewAnimation.IsValid()?PreviewAnimation->GetPathName():FString();})
                .OnObjectChanged_Lambda([this](const FAssetData& Asset)
                {
                    PreviewAnimation=Cast<UAnimSequence>(Asset.GetAsset()); PoseSeconds=0;
                    bPosePlaying=false;
                    Rebuild(false);
                })
                .ToolTipText(LOCTEXT("PoseHelp","Optional pose animation; uses the body's skeleton. Empty shows the reference pose.")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text_Lambda([this]{return bPosePlaying?LOCTEXT("PausePose","Pause"):LOCTEXT("PlayPose","Play");})
                .IsEnabled_Lambda([this]{return PreviewAnimation.IsValid();})
                .OnClicked_Lambda([this]
                {
                    bPosePlaying=!bPosePlaying;
                    if(bPosePlaying&&!bPoseTimerRegistered)
                    {bPoseTimerRegistered=true;RegisterActiveTimer(1.0f/30.0f,FWidgetActiveTimerDelegate::CreateSP(this,&STMOPAppearancePreview::TickPose));}
                    return FReply::Handled();
                }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
        [
            SNew(SExpandableArea)
            .InitiallyCollapsed(true)
            .Visibility(this,
                &STMOPAppearancePreview::GetAppearanceSelectorVisibility)
            .HeaderContent()
            [ SNew(STextBlock).Text(LOCTEXT("AppearanceAssets", "APPEARANCE ASSETS")) ]
            .BodyContent()
            [
                SNew(SBox).MaxDesiredHeight(260.0f)
                [ SNew(SScrollBox) + SScrollBox::Slot() [ SelectorRows ] ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SBox).MaxDesiredHeight(64)
            [ SNew(SScrollBox) + SScrollBox::Slot()
                [ SAssignNew(Status,STextBlock).AutoWrapText(true)
                    .Text(LOCTEXT("Empty","Select a person or vehicle.")) ] ] ]
    ];
    SetToolTipText(LOCTEXT("Controls","Alt + left drag: orbit. Mouse wheel: zoom. F / Fit: frame model. Preview lighting is separate from the level."));
}

void STMOPAppearancePreview::AddReferencedObjects(FReferenceCollector& Collector)
{
    if (VehicleProfile.IsSet())
        Collector.AddPropertyReferencesWithStructARO(FTMOPHistoricalVehicleRow::StaticStruct(), &VehicleProfile.GetValue(), nullptr);
    if (PersonProfile.IsSet())
        Collector.AddPropertyReferencesWithStructARO(FTMOPPersonProfileRow::StaticStruct(), &PersonProfile.GetValue(), nullptr);
    Collector.AddReferencedObject(AppearanceCatalog);
    Collector.AddReferencedObject(PreviewAnimation);
}
void STMOPAppearancePreview::ShowVehicle(const FTMOPHistoricalVehicleRow& Profile)
{
    if(VehicleProfile.IsSet() && ViewportWidget->GetActor() &&
        FTMOPHistoricalVehicleRow::StaticStruct()->CompareScriptStruct(&VehicleProfile.GetValue(),&Profile,0)) return;
    const bool bRefocus=!VehicleProfile.IsSet() || VehicleProfile->VehicleId!=Profile.VehicleId ||
        VehicleProfile->ModelData!=Profile.ModelData || VehicleProfile->VehicleClass!=Profile.VehicleClass;
    VehicleProfile=Profile;PersonProfile.Reset();PreviewAnimation.Reset();bPosePlaying=false;
    RefreshAppearanceOptions();
    Rebuild(bRefocus);
}
void STMOPAppearancePreview::ShowPerson(const FTMOPPersonProfileRow& Profile, UDataTable* Catalog, int32 AtSecond)
{
    if(PersonProfile.IsSet() && AppearanceCatalog.Get()==Catalog && ViewportWidget->GetActor() &&
        FTMOPPersonProfileRow::StaticStruct()->CompareScriptStruct(&PersonProfile.GetValue(),&Profile,0))
    {
        if(PreviewSecond!=AtSecond)
        {
            PreviewSecond=FMath::Clamp(AtSecond,0,86399);
            if(ATMOPHistoricalAgent* Agent=Cast<ATMOPHistoricalAgent>(ViewportWidget->GetActor()))
            {
                Agent->AppearancePreviewSecond=PreviewSecond;
                Agent->ApplyHeldItems(Profile.LeftHandItem,Profile.RightHandItem,Profile.AdditionalCarriedItems);
                PreparePreviewActor(Agent);
                ViewportWidget->EvaluatePose(PreviewAnimation.Get(),PoseSeconds);
            }
        }
        return;
    }
    const bool bRefocus=!PersonProfile.IsSet() || PersonProfile->EntityId!=Profile.EntityId;
    if(bRefocus){PreviewAnimation.Reset();bPosePlaying=false;PoseSeconds=0;}
    PersonProfile=Profile;VehicleProfile.Reset();AppearanceCatalog=Catalog;
    RefreshAppearanceOptions();
    PreviewSecond=FMath::Clamp(AtSecond,0,86399);
    Rebuild(bRefocus);
}
void STMOPAppearancePreview::Clear(const FString& Reason)
{
    VehicleProfile.Reset();PersonProfile.Reset();PreviewAnimation.Reset();bPosePlaying=false;
    ViewportWidget->ClearActor();Description=Reason;
    Status->SetText(FText::FromString(Reason));
    RefreshAppearanceOptions();
}
void STMOPAppearancePreview::Refresh() { Rebuild(false); }
AActor* STMOPAppearancePreview::GetPreviewActor() const { return ViewportWidget ? ViewportWidget->GetActor() : nullptr; }
TArray<FName> STMOPAppearancePreview::GetSockets() const { return ViewportWidget->GetSockets(); }

void STMOPAppearancePreview::RefreshAppearanceOptions()
{
    if (AppearanceOptions.Num() != AppearancePartCount)
        AppearanceOptions.SetNum(AppearancePartCount);
    for (TArray<TSharedPtr<FName>>& Options : AppearanceOptions)
    {
        Options.Reset();
        Options.Add(MakeShared<FName>(NAME_None));
    }

    UDataTable* Catalog = AppearanceCatalog.Get();
    if (!IsValid(Catalog) && PersonProfile.IsSet())
    {
        Catalog = TMOPAppearancePreview::AppearanceTable();
        AppearanceCatalog = Catalog;
    }
    if (IsValid(Catalog) &&
        Catalog->GetRowStruct() == FTMOPAppearanceAssetRow::StaticStruct())
    {
        for (const FName RowName : Catalog->GetRowNames())
        {
            const FTMOPAppearanceAssetRow* Row =
                Catalog->FindRow<FTMOPAppearanceAssetRow>(RowName,
                    TEXT("Appearance preview selector"), false);
            if (!Row) continue;
            const int32 PartIndex = static_cast<int32>(Row->PartType);
            if (AppearanceOptions.IsValidIndex(PartIndex))
                AppearanceOptions[PartIndex].Add(MakeShared<FName>(RowName));
        }
    }

    for (int32 PartIndex = 0; PartIndex < AppearanceOptions.Num(); ++PartIndex)
    {
        AppearanceOptions[PartIndex].Sort(
            [](const TSharedPtr<FName>& A, const TSharedPtr<FName>& B)
            {
                if (!A.IsValid()) return true;
                if (!B.IsValid()) return false;
                if (A->IsNone()) return true;
                if (B->IsNone()) return false;
                return A->LexicalLess(*B);
            });
        if (!AppearanceCombos.IsValidIndex(PartIndex) ||
            !AppearanceCombos[PartIndex].IsValid()) continue;
        AppearanceCombos[PartIndex]->RefreshOptions();
        FName Current = NAME_None;
        if (PersonProfile.IsSet())
            if (const FTMOPAppearancePartChoice* Choice = FindAppearanceChoice(
                PersonProfile->AppearanceProfile,
                static_cast<ETMOPAppearancePartType>(PartIndex)))
                Current = Choice->CatalogId;
        const TSharedPtr<FName>* Match = AppearanceOptions[PartIndex].FindByPredicate(
            [Current](const TSharedPtr<FName>& Item)
            { return Item.IsValid() && *Item == Current; });
        AppearanceCombos[PartIndex]->SetSelectedItem(
            Match ? *Match : AppearanceOptions[PartIndex][0]);
    }
}

void STMOPAppearancePreview::SelectAppearanceAsset(
    TSharedPtr<FName> Item, const ESelectInfo::Type SelectInfo,
    const ETMOPAppearancePartType PartType)
{
    if (SelectInfo == ESelectInfo::Direct || !Item.IsValid() ||
        !PersonProfile.IsSet()) return;
    FTMOPAppearanceProfile& Profile = PersonProfile->AppearanceProfile;
    FTMOPAppearancePartChoice* Choice = FindAppearanceChoice(Profile, PartType);
    if (!Choice) return;
    Choice->CatalogId = *Item;
    Choice->MeshOverride.Reset();
    Choice->StaticMeshOverride.Reset();
    Choice->MaterialOverride.Reset();
    if (!Item->IsNone() && Profile.GenerationMode !=
        ETMOPAppearanceGenerationMode::MetaHuman)
        Profile.GenerationMode = ETMOPAppearanceGenerationMode::Manual;
    OnPersonAppearanceChanged.ExecuteIfBound(Profile);
    Rebuild(false);
}

TSharedRef<SWidget> STMOPAppearancePreview::GenerateAppearanceOption(
    TSharedPtr<FName> Item) const
{
    return SNew(STextBlock).Text(!Item.IsValid() || Item->IsNone()
        ? LOCTEXT("AutomaticAppearance", "Automatic (from evidence)")
        : FText::FromName(*Item));
}

FText STMOPAppearancePreview::GetAppearanceChoiceText(
    const ETMOPAppearancePartType PartType) const
{
    if (PersonProfile.IsSet())
        if (const FTMOPAppearancePartChoice* Choice = FindAppearanceChoice(
            PersonProfile->AppearanceProfile, PartType))
            if (!Choice->CatalogId.IsNone())
                return FText::FromName(Choice->CatalogId);
    return LOCTEXT("AutomaticAppearance", "Automatic (from evidence)");
}

EVisibility STMOPAppearancePreview::GetAppearanceSelectorVisibility() const
{
    return PersonProfile.IsSet() ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply STMOPAppearancePreview::ReloadAppearanceAssets()
{
    if (PersonProfile.IsSet())
        AppearanceCatalog = TMOPAppearancePreview::AppearanceTable();
    RefreshAppearanceOptions();
    Rebuild(false);
    return FReply::Handled();
}

void STMOPAppearancePreview::Rebuild(bool bRefocus)
{
    ViewportWidget->ClearActor();
    UWorld* World=ViewportWidget->GetPreviewWorld();
    if (!World) return;
    TArray<FString> Warnings;
    AActor* Created=nullptr;
    if(VehicleProfile.IsSet())
    {
        const auto& Profile=VehicleProfile.GetValue();
        UClass* Class=TMOPVehiclePresentation::ResolveClass(Profile,TMOPAppearancePreview::DefaultVehicleClass());
        ATMOPVehicleBase* Vehicle=SpawnPreview<ATMOPVehicleBase>(World,Class);
        Created=Vehicle;
        if(Vehicle)
        {
            Vehicle->VehicleId=Profile.VehicleId;
            if(!TMOPVehiclePresentation::ApplyProfile(Vehicle,Profile,&Warnings))
                Warnings.Add(TEXT("The selected vehicle has no usable model."));

        }
        else Warnings.Add(TEXT("Set Model Data / Vehicle Class, or the level director's Default Vehicle Class."));
        Description=Profile.DisplayName.IsEmpty()?Profile.VehicleId.ToString():Profile.DisplayName.ToString();
    }
    else if(PersonProfile.IsSet())
    {
        const auto& Profile=PersonProfile.GetValue();
        UClass* Class=Profile.AgentClass?Profile.AgentClass.Get():TMOPAppearancePreview::DefaultPersonClass();
        ATMOPHistoricalAgent* Agent=SpawnPreview<ATMOPHistoricalAgent>(World,Class);
        Created=Agent;
        if(Agent)
        {
            Agent->AppearancePreviewSecond=PreviewSecond;
            Agent->EntityIdentity->EntityId=Profile.EntityId;
            Agent->DisplayName=Profile.FullName;
            UTMOPPersonProfileComponent* Person=Agent->FindComponentByClass<UTMOPPersonProfileComponent>();
            if(!Person)
            {
                Person=NewObject<UTMOPPersonProfileComponent>(Agent,NAME_None,RF_Transient);
                Agent->AddInstanceComponent(Person);Person->RegisterComponent();
            }
            Person->Profile=Profile;Person->ResolvedEntityId=Profile.EntityId;Person->bHasLoadedProfile=true;
            UTMOPCharacterAppearanceComponent* Appearance=Agent->CharacterAppearance;
            if(Appearance)
            {
                if(!Appearance->AssetCatalogOverride)
                    Appearance->AssetCatalogOverride=AppearanceCatalog.IsValid()?AppearanceCatalog.Get():TMOPAppearancePreview::AppearanceTable();
                if(!Appearance->ApplyAppearance())Warnings.Add(TEXT("Appearance could not be fully assembled."));
                TArray<FString> AppearanceWarnings;
                Appearance->ValidateAppearance(AppearanceWarnings);
                Warnings.Append(AppearanceWarnings);
            }
            // Assembly must finish before props are attached to the final body's sockets.
            Agent->ApplyHeldItems(Profile.LeftHandItem,Profile.RightHandItem,Profile.AdditionalCarriedItems);
            auto CheckHand = [&](const FTMOPHeldItemDefinition& Item,UStaticMeshComponent* Part,const TCHAR* Label)
            {
                if(!Item.bVisible || (Item.ItemId.IsNone() && Item.Mesh.IsNull())) return;
                if(!Part || !Part->GetStaticMesh())
                    Warnings.Add(FString::Printf(TEXT("%s: mesh missing for '%s'; check Item ID / Mesh."),Label,*Item.ItemId.ToString()));
                if(Agent->BodyMesh && !Item.SocketName.IsNone() && !Agent->BodyMesh->DoesSocketExist(Item.SocketName))
                    Warnings.Add(FString::Printf(TEXT("%s: socket '%s' does not exist."),Label,*Item.SocketName.ToString()));
            };
            CheckHand(Profile.LeftHandItem,Agent->LeftHandHeldItem,TEXT("Left hand"));
            CheckHand(Profile.RightHandItem,Agent->RightHandHeldItem,TEXT("Right hand"));
            if(!Agent->BodyMesh || !Agent->BodyMesh->GetSkeletalMeshAsset())
                Warnings.Add(TEXT("Missing body mesh; check Agent Class and the appearance catalog."));
            if(PreviewAnimation.IsValid() && Agent->BodyMesh && Agent->BodyMesh->GetSkeletalMeshAsset() &&
                PreviewAnimation->GetSkeleton()!=Agent->BodyMesh->GetSkeletalMeshAsset()->GetSkeleton())
            {Warnings.Add(TEXT("Preview animation uses another skeleton; reference pose shown."));bPosePlaying=false;}
        }
        else Warnings.Add(TEXT("Set Agent Class, or the level Person Registry Director's Default Agent Class."));
        Description=Profile.FullName.IsEmpty()?Profile.EntityId.ToString():Profile.FullName.ToString();
    }
    else return;
    ViewportWidget->SetActor(Created,bRefocus);
    ViewportWidget->EvaluatePose(PreviewAnimation.Get(),PoseSeconds);
    if(Created && !VisibleBounds(Created).IsValid)Warnings.Add(TEXT("No visible mesh."));
    const FString Full=Description+(Warnings.IsEmpty()?FString():TEXT("\n")+FString::Join(Warnings,TEXT("\n")));
    Status->SetText(FText::FromString(Full));
    Status->SetToolTipText(FText::FromString(Full));
    Status->SetColorAndOpacity(Warnings.IsEmpty()?FSlateColor::UseForeground():FSlateColor(FLinearColor(1.0f,0.7f,0.25f)));
}
FText STMOPAppearancePreview::TimeText() const
{
    return FText::FromString(FString::Printf(TEXT("%02d:%02d:%02d"),PreviewSecond/3600,(PreviewSecond/60)%60,PreviewSecond%60));
}
void STMOPAppearancePreview::SetTime(const FText& Text,ETextCommit::Type Commit)
{
    TArray<FString> Parts;Text.ToString().ParseIntoArray(Parts,TEXT(":"),false);
    if(Parts.Num()!=3 || !Parts[0].IsNumeric() || !Parts[1].IsNumeric() || !Parts[2].IsNumeric())return;
    const int32 H=FCString::Atoi(*Parts[0]),M=FCString::Atoi(*Parts[1]),S=FCString::Atoi(*Parts[2]);
    if(H<0||H>23||M<0||M>59||S<0||S>59)return;
    PreviewSecond=H*3600+M*60+S;Rebuild(false);
}
EActiveTimerReturnType STMOPAppearancePreview::TickPose(double Now,float DeltaSeconds)
{
    if(!bPosePlaying||!PreviewAnimation.IsValid())
    {bPoseTimerRegistered=false;return EActiveTimerReturnType::Stop;}
    if(GetVisibility()!=EVisibility::Visible)return EActiveTimerReturnType::Continue;
    PoseSeconds=FMath::Fmod(PoseSeconds+DeltaSeconds,FMath::Max(0.001f,PreviewAnimation->GetPlayLength()));
    ViewportWidget->EvaluatePose(PreviewAnimation.Get(),PoseSeconds);
    return EActiveTimerReturnType::Continue;
}
#undef LOCTEXT_NAMESPACE
