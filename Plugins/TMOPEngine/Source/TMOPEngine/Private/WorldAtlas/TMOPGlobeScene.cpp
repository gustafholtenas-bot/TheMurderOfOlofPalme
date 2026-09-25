#include "WorldAtlas/TMOPGlobeScene.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

FTMOPGlobeScene::FTMOPGlobeScene(UStaticMesh* Mesh, UMaterialInterface* Material, const FRotator& Alignment)
    : FPreviewScene(FPreviewScene::ConstructionValues().SetEditor(false)
        .SetCreatePhysicsScene(false).ShouldSimulatePhysics(false).AllowAudioPlayback(false)
        .SetTransactional(false).SetLightBrightness(2.5f).SetSkyBrightness(0.8f)), MeshAlignment(Alignment.Quaternion())
{
    if (!Mesh || !GetWorld()) return;
    // A sphere's radius is one axis extent, not the diagonal BoundingSphere radius.
    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    MeshCenter = Bounds.Origin;
    MeshScale = 100.0 / FMath::Max(0.01, double(Bounds.BoxExtent.GetMax()));
    Sphere = NewObject<UStaticMeshComponent>(GetTransientPackage());
    Sphere->SetMobility(EComponentMobility::Movable);
    Sphere->SetStaticMesh(Mesh);
    Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Sphere->SetCastShadow(false);
    if (Material) Sphere->SetMaterial(0, Material);
    AddComponent(Sphere, FTransform::Identity, false);
    SetLightDirection(FRotator(-30, 160, 0));

    Target = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
    Target->ClearColor = FLinearColor::Black;
    Target->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
    Target->InitAutoFormat(1024, 1024);
    Target->UpdateResourceImmediate(true);
    Capture = NewObject<USceneCaptureComponent2D>(GetTransientPackage());
    Capture->TextureTarget = Target;
    Capture->ProjectionType = ECameraProjectionMode::Orthographic;
    Capture->OrthoWidth = 240.0f;
    Capture->bCaptureEveryFrame = false;
    Capture->bCaptureOnMovement = false;
    Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    Capture->ShowOnlyComponents.Add(Sphere);
    Capture->ShowFlags.SetAtmosphere(false);
    Capture->ShowFlags.SetFog(false);
    Capture->ShowFlags.SetMotionBlur(false);
    Capture->ShowFlags.SetEyeAdaptation(false);
    Capture->ShowFlags.SetAntiAliasing(false); // no temporal history in an on-demand capture
    Capture->PostProcessSettings.bOverride_AutoExposureMethod = true;
    Capture->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    Capture->PostProcessSettings.bOverride_AutoExposureBias = true;
    Capture->PostProcessSettings.AutoExposureBias = 0;
    AddComponent(Capture, FTransform(FRotator(0, 180, 0), FVector(400, 0, 0)), false);
    Brush.SetResourceObject(Target);
    Brush.ImageSize = FVector2D(1024, 1024);
    Brush.DrawAs = ESlateBrushDrawType::Image;
    Render(FQuat::Identity);
}

void FTMOPGlobeScene::Render(const FQuat& Rotation)
{
    if (!Sphere || !Capture) return;
    const FQuat Q = Rotation * MeshAlignment;
    Sphere->SetWorldTransform(FTransform(Q, -Q.RotateVector(MeshCenter * MeshScale), FVector(MeshScale)));
    GetWorld()->SendAllEndOfFrameUpdates();
    Capture->CaptureScene(); // explicit capture continues to work while the game is paused
}

void FTMOPGlobeScene::AddReferencedObjects(FReferenceCollector& Collector)
{
    FPreviewScene::AddReferencedObjects(Collector);
    Collector.AddReferencedObject(Target);
}

FTMOPGlobeScene::~FTMOPGlobeScene()
{
    Brush.SetResourceObject(nullptr);
    if (Capture) Capture->TextureTarget = nullptr;
    // FPreviewScene unregisters components and destroys its world; no rooted objects.
}
