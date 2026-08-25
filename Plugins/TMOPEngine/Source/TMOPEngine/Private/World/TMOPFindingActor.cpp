#include "World/TMOPFindingActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
FString CompactFindingSource(FString Source)
{
    Source.TrimStartAndEndInline();
    int32 Cut = INDEX_NONE;
    const TCHAR Separators[] = { TCHAR(','), TCHAR(';'), TCHAR('\n'), TCHAR('\r') };
    for (const TCHAR Separator : Separators)
    {
        int32 Found = INDEX_NONE;
        if (Source.FindChar(Separator, Found) && (Cut == INDEX_NONE || Found < Cut))
            Cut = Found;
    }
    if (Cut != INDEX_NONE) Source.LeftInline(Cut);
    Source.TrimStartAndEndInline();
    return Source.Left(32);
}
}

ATMOPFindingActor::ATMOPFindingActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;

    FindingRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FindingRoot"));
    SetRootComponent(FindingRoot);

    FindingMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("FindingMesh"));
    FindingMesh->SetupAttachment(FindingRoot);
    FindingMesh->SetMobility(EComponentMobility::Movable);
    FindingMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    FindingMesh->SetCollisionResponseToAllChannels(ECR_Block);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (DefaultMesh.Succeeded())
        FindingMesh->SetStaticMesh(DefaultMesh.Object);

    FindingLabel = CreateDefaultSubobject<UTextRenderComponent>(
        TEXT("FindingLabel"));
    FindingLabel->SetupAttachment(FindingRoot);
    FindingLabel->SetRelativeLocation(FVector(0.0f, 0.0f, FindingLabelHeightCm));
    FindingLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    FindingLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    FindingLabel->SetWorldSize(FindingLabelWorldSize);
    FindingLabel->SetTextRenderColor(FColor(255, 205, 40));
    FindingLabel->SetCastShadow(false);
    FindingLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnlitTextMaterial(
        TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque.DefaultTextMaterialOpaque"));
    if (UnlitTextMaterial.Succeeded())
    {
        FindingLabelUnlitMaterial = UnlitTextMaterial.Object;
        FindingLabel->SetTextMaterial(FindingLabelUnlitMaterial);
    }
}

void ATMOPFindingActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsValid(FindingLabel) || GetWorld() == nullptr) return;
    const APlayerController* PC = GetWorld()->GetFirstPlayerController();
    const APlayerCameraManager* Camera = IsValid(PC) ? PC->PlayerCameraManager : nullptr;
    if (!IsValid(Camera)) return;
    const FVector ToCamera = Camera->GetCameraLocation() - FindingLabel->GetComponentLocation();
    if (!ToCamera.IsNearlyZero()) FindingLabel->SetWorldRotation(ToCamera.Rotation());
}

void ATMOPFindingActor::ConfigureFinding(
    const FText& InDisplayName,
    const FString& InEvidenceId,
    const FString& InSourceReference,
    const FString& InSourceTimeLabel,
    const double InLatitude,
    const double InLongitude,
    const bool bInLocationApproximate,
    UStaticMesh* InMesh,
    const FVector InScale,
    const FLinearColor InColor)
{
    DisplayName = InDisplayName;
    EvidenceId = InEvidenceId;
    SourceReference = InSourceReference;
    SourceTimeLabel = InSourceTimeLabel;
    SourceLatitude = InLatitude;
    SourceLongitude = InLongitude;
    bLocationApproximate = bInLocationApproximate;

    if (IsValid(InMesh)) FindingMesh->SetStaticMesh(InMesh);
    FindingMesh->SetRelativeScale3D(InScale);
    const FString CompactSource = CompactFindingSource(SourceReference);
    const FString Label = DocumentSymbol + TEXT("\n") +
        (CompactSource.IsEmpty() ? MissingSourceText : CompactSource) +
        TEXT("\n") + DisplayName.ToString();
    FindingLabel->SetText(FText::FromString(Label));
    FindingLabel->SetRelativeLocation(FVector(0.0f, 0.0f, FindingLabelHeightCm));
    FindingLabel->SetWorldSize(FindingLabelWorldSize);
    if (IsValid(FindingLabelUnlitMaterial))
        FindingLabel->SetTextMaterial(FindingLabelUnlitMaterial);

    if (UMaterialInstanceDynamic* Material =
        FindingMesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        Material->SetVectorParameterValue(TEXT("Color"), InColor);
        Material->SetVectorParameterValue(TEXT("BaseColor"), InColor);
    }
}
