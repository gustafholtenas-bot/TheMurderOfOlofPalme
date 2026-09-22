#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "People/TMOPCharacterAppearanceComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPSkeletalHairAssemblyTest,
    "TMOP.Appearance.Hair.SkeletalAssembly",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMOPSkeletalHairAssemblyTest::RunTest(const FString& Parameters)
{
    USkeletalMesh* Body = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    if (!TestNotNull(TEXT("Project's Manny reference mesh"), Body)) return false;
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world"), World)) return false;
    World->InitializeNewWorld(UWorld::InitializationValues().AllowAudioPlayback(false)
        .CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false)
        .ShouldSimulatePhysics(false));
    ATMOPHistoricalAgent* Agent = World->SpawnActor<ATMOPHistoricalAgent>();
    if (!TestNotNull(TEXT("Agent"), Agent))
    {
        World->DestroyWorld(false);
        return false;
    }
    Agent->BodyMesh->SetSkeletalMesh(Body);
    auto* Appearance = Agent->CharacterAppearance.Get();
    // Use the body as a real skinned fixture: this tests assembly, not hairstyle.
    FTMOPResolvedAppearancePart Part;
    Part.PartType = ETMOPAppearancePartType::Hair;
    Part.Mesh = Body;
    Part.StaticMesh = NewObject<UStaticMesh>();
    Part.AttachmentSocket = TEXT("head");
    Part.AttachmentTransform = FTransform(FRotator(0, 90, 0), FVector(0, 0, 200));
    Agent->HairMesh->AttachToComponent(Agent->BodyMesh,
        FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("head"));
    Agent->HairMesh->SetRelativeTransform(Part.AttachmentTransform);
    TestTrue(TEXT("Skinned hair wins when old static reference is also set"),
        Appearance->ApplySkeletalHair(Agent, Part));
    TestTrue(TEXT("Hair uses selected skeletal mesh"), Agent->HairMesh->GetSkeletalMeshAsset() == Body);
    TestTrue(TEXT("Hair is parented to body"), Agent->HairMesh->GetAttachParent() == Agent->BodyMesh);
    TestTrue(TEXT("Hair has no socket parent"), Agent->HairMesh->GetAttachSocketName().IsNone());
    TestTrue(TEXT("Old pivot/90-degree offset is cleared"),
        Agent->HairMesh->GetRelativeTransform().Equals(FTransform::Identity));
    TestTrue(TEXT("Hair follows body's bone pose"),
        Agent->HairMesh->LeaderPoseComponent.Get() == Agent->BodyMesh);
    TestTrue(TEXT("Static hair is not created"), Appearance->SocketHairMesh == nullptr);
    Appearance->ApplySkeletalHair(Agent, Part);
    TestTrue(TEXT("Reapplying appearance does not accumulate offsets"),
        Agent->HairMesh->GetRelativeTransform().Equals(FTransform::Identity));
    Part.Mesh.Reset();
    TestFalse(TEXT("Static-only hair is rejected"), Appearance->ApplySkeletalHair(Agent, Part));
    TestNull(TEXT("Missing hair clears old geometry"), Agent->HairMesh->GetSkeletalMeshAsset());
    TestFalse(TEXT("Missing hair is hidden"), Agent->HairMesh->IsVisible());
    Part.bIntentionallyEmpty = true;
    TestTrue(TEXT("Bald appearance succeeds without geometry"), Appearance->ApplySkeletalHair(Agent, Part));
    World->DestroyWorld(false);
    return true;
}
#endif
