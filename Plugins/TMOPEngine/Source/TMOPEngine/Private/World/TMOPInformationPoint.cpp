#include "World/TMOPInformationPoint.h"

#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "World/TMOPInformationComponent.h"

ATMOPInformationPoint::ATMOPInformationPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("PointRoot")));
    Information = CreateDefaultSubobject<UTMOPInformationComponent>(TEXT("Information"));
    // Drag the actor onto the ground; its reading point starts at standing reading height.
    Information->InteractionOffset = FVector(0.0f, 0.0f, 140.0f);
#if WITH_EDITORONLY_DATA
    EditorLabel = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("EditorLabel"));
    if (EditorLabel)
    {
        EditorLabel->SetupAttachment(GetRootComponent());
        EditorLabel->SetHorizontalAlignment(EHTA_Center);
        EditorLabel->SetWorldSize(22.0f);
        EditorLabel->SetTextRenderColor(FColor(100, 210, 245));
        EditorLabel->SetText(NSLOCTEXT("TMOP", "InformationPointEditorLabel", "Informationspunkt"));
        EditorLabel->SetHiddenInGame(true);
        EditorLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        EditorLabel->SetCanEverAffectNavigation(false);
    }
#endif
}

void ATMOPInformationPoint::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
#if WITH_EDITORONLY_DATA
    if (IsValid(EditorLabel.Get()) && IsValid(Information.Get()))
    {
        EditorLabel->SetText(Information->Title.IsEmpty()
            ? NSLOCTEXT("TMOP", "InformationPointEditorLabel", "Informationspunkt") : Information->Title);
        EditorLabel->SetRelativeLocation(Information->InteractionOffset + FVector(0.0f, 0.0f, 25.0f));
    }
#endif
}
