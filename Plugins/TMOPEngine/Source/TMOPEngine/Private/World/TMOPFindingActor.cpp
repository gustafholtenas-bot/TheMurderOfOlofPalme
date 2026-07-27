#include "World/TMOPFindingActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ATMOPFindingActor::ATMOPFindingActor()
{
    PrimaryActorTick.bCanEverTick = false;

    FindingMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("FindingMesh"));
    SetRootComponent(FindingMesh);
    FindingMesh->SetMobility(EComponentMobility::Movable);
    FindingMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    FindingMesh->SetCollisionResponseToAllChannels(ECR_Block);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (DefaultMesh.Succeeded())
        FindingMesh->SetStaticMesh(DefaultMesh.Object);

    FindingLabel = CreateDefaultSubobject<UTextRenderComponent>(
        TEXT("FindingLabel"));
    FindingLabel->SetupAttachment(FindingMesh);
    FindingLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 75.0f));
    FindingLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    FindingLabel->SetWorldSize(18.0f);
    FindingLabel->SetTextRenderColor(FColor(255, 205, 40));
    FindingLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
    FindingLabel->SetText(DisplayName);

    if (UMaterialInstanceDynamic* Material =
        FindingMesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        Material->SetVectorParameterValue(TEXT("Color"), InColor);
        Material->SetVectorParameterValue(TEXT("BaseColor"), InColor);
    }
}

