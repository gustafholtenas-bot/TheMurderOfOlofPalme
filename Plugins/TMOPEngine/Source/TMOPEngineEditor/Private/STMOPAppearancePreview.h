#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "UObject/GCObject.h"
#include "Widgets/SCompoundWidget.h"
#include "People/TMOPPersonProfileTypes.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"

class STextBlock;
class SVerticalBox;
class FTMOPAppearanceViewportClient;
class UAnimSequence;
enum class ETMOPAppearancePartType : uint8;
template<typename OptionType> class SComboBox;

DECLARE_DELEGATE_OneParam(FTMOPOnPersonAppearanceChanged,
    const FTMOPAppearanceProfile&);

/** Isolated preview world: never runs BeginPlay or the simulation directors. */
class STMOPAppearanceViewport final : public SEditorViewport
{
public:
    SLATE_BEGIN_ARGS(STMOPAppearanceViewport) {}
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    virtual ~STMOPAppearanceViewport() override;
    UWorld* GetPreviewWorld() const;
    void ClearActor();
    void SetActor(AActor* Actor, bool bRefocus);
    void Fit();
    void Turn(float Degrees);
    void Zoom(float Factor);
    void EvaluatePose(UAnimSequence* Animation, float Seconds);
    TArray<FName> GetSockets() const;
    AActor* GetActor() const;
protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual void OnFocusViewportToSelection() override;
private:
    TSharedPtr<FPreviewScene> Scene;
    TSharedPtr<FTMOPAppearanceViewportClient> PreviewClient;
    TWeakObjectPtr<AActor> PreviewActor;
};

/** Common compact panel for vehicle, person and observation editors. */
class STMOPAppearancePreview final : public SCompoundWidget, public FGCObject
{
public:
    SLATE_BEGIN_ARGS(STMOPAppearancePreview) {}
        SLATE_EVENT(FTMOPOnPersonAppearanceChanged, OnPersonAppearanceChanged)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    void ShowVehicle(const FTMOPHistoricalVehicleRow& Profile);
    void ShowPerson(const FTMOPPersonProfileRow& Profile, UDataTable* Catalog = nullptr,
        int32 AtSecond = 23 * 3600);
    void Clear(const FString& Reason);
    void Refresh();
    AActor* GetPreviewActor() const;
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override { return TEXT("STMOPAppearancePreview"); }
    TArray<FName> GetSockets() const;
private:
    void Rebuild(bool bRefocus);
    void RefreshAppearanceOptions();
    void SelectAppearanceAsset(TSharedPtr<FName> Item,
        ESelectInfo::Type SelectInfo, ETMOPAppearancePartType PartType);
    TSharedRef<SWidget> GenerateAppearanceOption(TSharedPtr<FName> Item) const;
    FText GetAppearanceChoiceText(ETMOPAppearancePartType PartType) const;
    EVisibility GetAppearanceSelectorVisibility() const;
    FReply ReloadAppearanceAssets();
    void SetTime(const FText& Text, ETextCommit::Type Commit);
    FText TimeText() const;
    TSharedPtr<STMOPAppearanceViewport> ViewportWidget;
    TSharedPtr<SVerticalBox> AppearanceSelectorBox;
    TSharedPtr<STextBlock> Status;
    TOptional<FTMOPHistoricalVehicleRow> VehicleProfile;
    TOptional<FTMOPPersonProfileRow> PersonProfile;
    TWeakObjectPtr<UDataTable> AppearanceCatalog;
    TWeakObjectPtr<UAnimSequence> PreviewAnimation;
    FTMOPOnPersonAppearanceChanged OnPersonAppearanceChanged;
    TArray<TArray<TSharedPtr<FName>>> AppearanceOptions;
    TArray<TSharedPtr<SComboBox<TSharedPtr<FName>>>> AppearanceCombos;
    FString Description;
    bool bPosePlaying = false;
    bool bPoseTimerRegistered = false;
    float PoseSeconds = 0.0f;
    int32 PreviewSecond = 23 * 3600;
    EActiveTimerReturnType TickPose(double Now, float DeltaSeconds);
};

/** Current level's configured tables/classes, without creating a game instance. */
namespace TMOPAppearancePreview
{
    UWorld* EditorWorld();
    UDataTable* VehicleTable();
    UDataTable* PeopleTable();
    UDataTable* AppearanceTable();
    UClass* DefaultPersonClass();
    UClass* DefaultVehicleClass();
}
