#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "People/TMOPPersonProfileTypes.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "UObject/StrongObjectPtr.h"
#include "Styling/SlateBrush.h"
#include "Engine/TextureRenderTarget2D.h"

class ATMOPPlayerCharacter;
class ATMOPPlayerAppearanceDirector;
class USceneCaptureComponent2D;
class SVerticalBox;

class STMOPPlayerAppearancePanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPPlayerAppearancePanel) : _Player(nullptr), _Startup(false), _Ready(false) {}
        SLATE_ARGUMENT(ATMOPPlayerCharacter*, Player)
        SLATE_ARGUMENT(bool, Startup)
        SLATE_ATTRIBUTE(bool, Ready)
        SLATE_EVENT(FSimpleDelegate, OnEdited)
        SLATE_EVENT(FSimpleDelegate, OnReady)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    virtual ~STMOPPlayerAppearancePanel() override;
private:
    void RebuildOptions(bool bRepair);
    void RefreshPreview();
    void DestroyPreview();
    bool Apply();
    FReply Cycle(ETMOPAppearancePartType Type, int32 Direction);
    FReply ChangeGender();
    FReply ChangeBuild();
    FReply ChangeColor();
    FText PartLabel(ETMOPAppearancePartType Type) const;
    TSharedRef<SWidget> PartRow(ETMOPAppearancePartType Type, const FString& Label);
    TWeakObjectPtr<ATMOPPlayerCharacter> Player;
    TWeakObjectPtr<ATMOPPlayerAppearanceDirector> Director;
    FTMOPPersonProfileRow Draft;
    FTMOPPersonProfileRow Accepted;
    FTMOPPersonProfileRow Opening;
    TMap<ETMOPAppearancePartType, TArray<FName>> Options;
    FString Status;
    TStrongObjectPtr<AActor> PreviewActor;
    TStrongObjectPtr<UTextureRenderTarget2D> Target;
    FSlateBrush PreviewBrush;
    float PreviewYaw = 0;
    bool bStartup = false;
    TAttribute<bool> Ready;
    FSimpleDelegate OnEdited;
    FSimpleDelegate OnReady;
};
