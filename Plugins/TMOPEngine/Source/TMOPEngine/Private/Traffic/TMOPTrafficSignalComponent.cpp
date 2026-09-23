#include "Traffic/TMOPTrafficSignalComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

UTMOPTrafficSignalComponent::UTMOPTrafficSignalComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}
void UTMOPTrafficSignalComponent::BeginPlay()
{
    Super::BeginPlay();
    ApplySignalState(CurrentState);
}
UMeshComponent* UTMOPTrafficSignalComponent::ResolveMesh() const
{
    if (IsValid(TargetMesh)) return TargetMesh;
    if (auto* ParentMesh = Cast<UMeshComponent>(GetAttachParent())) return ParentMesh;
    TArray<UMeshComponent*> Meshes;
    if (GetOwner()) GetOwner()->GetComponents(Meshes);
    return Meshes.Num() == 1 ? Meshes[0] : nullptr;
}
FName UTMOPTrafficSignalComponent::GetLampSlot(int32 Lamp) const
{
    if (Lamp == 0) return bPedestrian && RedSlot == TEXT("Vehicle_Red") ? FName(TEXT("Pedestrian_Red")) : RedSlot;
    if (Lamp == 1) return YellowSlot;
    return bPedestrian && GreenSlot == TEXT("Vehicle_Green") ? FName(TEXT("Pedestrian_Green")) : GreenSlot;
}
void UTMOPTrafficSignalComponent::ApplySignalState(const ETMOPTrafficSignalState NewState)
{
    const bool bChanged = CurrentState != NewState;
    CurrentState = NewState;
    if (UMeshComponent* Mesh = ResolveMesh())
    {
        LampMaterials.SetNum(3);
        const bool On[] = {
            NewState == ETMOPTrafficSignalState::Red || NewState == ETMOPTrafficSignalState::RedYellow,
            !bPedestrian && (NewState == ETMOPTrafficSignalState::Yellow || NewState == ETMOPTrafficSignalState::RedYellow || NewState == ETMOPTrafficSignalState::GreenYellow),
            NewState == ETMOPTrafficSignalState::Green || (!bPedestrian && NewState == ETMOPTrafficSignalState::GreenYellow)};
        for (int32 Lamp = 0; Lamp < 3; ++Lamp)
        {
            if (bPedestrian && Lamp == 1) continue;
            const int32 Slot = Mesh->GetMaterialIndex(GetLampSlot(Lamp));
            if (Slot == INDEX_NONE) continue;
            if (!IsValid(LampMaterials[Lamp]) || Mesh->GetMaterial(Slot) != LampMaterials[Lamp])
                LampMaterials[Lamp] = Mesh->CreateDynamicMaterialInstance(Slot);
            if (IsValid(LampMaterials[Lamp]))
                LampMaterials[Lamp]->SetScalarParameterValue(LampParameter, On[Lamp] ? 1.0f : 0.0f);
        }
    }
    if (bChanged) OnSignalVisualChanged(NewState);
}
bool UTMOPTrafficSignalComponent::ValidateMaterials(TArray<FString>& Errors) const
{
    Errors.Reset();
    const UMeshComponent* Mesh = ResolveMesh();
    if (!Mesh) { Errors.Add(TEXT("Missing or ambiguous TargetMesh.")); return false; }
    TSet<int32> Slots;
    for (int32 Lamp = 0; Lamp < 3; ++Lamp)
    {
        if (bPedestrian && Lamp == 1) continue;
        const FName Name = GetLampSlot(Lamp);
        const int32 Slot = Mesh->GetMaterialIndex(Name);
        if (Slot == INDEX_NONE) { Errors.Add(TEXT("Missing material slot: ") + Name.ToString()); continue; }
        if (Slots.Contains(Slot)) Errors.Add(TEXT("Two lamps use the same material slot."));
        Slots.Add(Slot);
        float Value = 0;
        UMaterialInterface* Material = Mesh->GetMaterial(Slot);
        if (!Material || !Material->GetScalarParameterValue(FMaterialParameterInfo(LampParameter), Value))
            Errors.Add(TEXT("Material lacks scalar parameter ") + LampParameter.ToString() + TEXT(": ") + Name.ToString());
    }
    return Errors.IsEmpty();
}
