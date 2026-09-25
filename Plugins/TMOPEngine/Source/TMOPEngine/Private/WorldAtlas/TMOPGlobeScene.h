#pragma once

#include "CoreMinimal.h"
#include "PreviewScene.h"
#include "Styling/SlateBrush.h"

class UStaticMesh;
class UMaterialInterface;
class UStaticMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

/** One isolated, non-ticking runtime world per open atlas. No actors in the game map. */
class FTMOPGlobeScene final : public FPreviewScene
{
public:
    FTMOPGlobeScene(UStaticMesh* Mesh, UMaterialInterface* Material, const FRotator& Alignment);
    virtual ~FTMOPGlobeScene() override;
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override { return TEXT("FTMOPGlobeScene"); }
    void Render(const FQuat& Rotation);
    const FSlateBrush* GetBrush() const { return Target ? &Brush : nullptr; }
private:
    UStaticMeshComponent* Sphere = nullptr;
    USceneCaptureComponent2D* Capture = nullptr;
    UTextureRenderTarget2D* Target = nullptr;
    FSlateBrush Brush;
    FQuat MeshAlignment;
    FVector MeshCenter = FVector::ZeroVector;
    double MeshScale = 1;
};
