#include "Vehicles/TMOPVehiclePresentation.h"

#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"

namespace TMOPVehiclePresentation
{
UClass* ResolveClass(const FTMOPHistoricalVehicleRow& Profile, UClass* DefaultClass)
{
    UClass* Result = IsValid(Profile.ModelData) ? ATMOPConfiguredVehicle::StaticClass() : DefaultClass;
    if (UClass* RowClass = Profile.VehicleClass.Get())
    {
        if (!RowClass->IsChildOf(ATMOPVehicleBase::StaticClass())) return nullptr;
        Result = IsValid(Profile.ModelData) && RowClass->IsChildOf(ATMOPConfiguredVehicle::StaticClass())
            ? ATMOPConfiguredVehicle::StaticClass() : RowClass;
    }
    return Result && Result->IsChildOf(ATMOPVehicleBase::StaticClass()) ? Result : nullptr;
}

bool Attach(USceneComponent* Part, UMeshComponent* Body, USceneComponent* RoofMount,
    FName Socket, const FTransform& Offset, FString& OutWarning)
{
    if (!IsValid(Part)) return false;
    if (Socket.IsNone() || Socket == TEXT("RoofAccessorySocket"))
    {
        if (!IsValid(RoofMount)) { OutWarning = TEXT("Missing RoofAccessorySocket mount."); return false; }
        Part->AttachToComponent(RoofMount, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        if (!IsValid(Body) || !Body->DoesSocketExist(Socket))
        {
            OutWarning = FString::Printf(TEXT("Socket '%s' is missing on the body mesh. Accessory hidden."), *Socket.ToString());
            return false;
        }
        Part->AttachToComponent(Body, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
    }
    Part->SetRelativeTransform(Offset);
    return true;
}

void BuildAccessories(ATMOPVehicleBase* Vehicle, UMeshComponent* Body,
    const TArray<FTMOPVehicleAccessoryVisual>& Entries, TArray<FString>& OutWarnings)
{
    if (!IsValid(Vehicle)) return;
    static const FName GeneratedTag(TEXT("TMOP.GeneratedVehicleAccessory"));
    TInlineComponentArray<UStaticMeshComponent*> Existing(Vehicle);
    for (UStaticMeshComponent* Part : Existing)
        if (Part->ComponentHasTag(GeneratedTag))
        {
            Vehicle->RemoveInstanceComponent(Part);
            Part->DestroyComponent();
        }
    for (const FTMOPVehicleAccessoryVisual& Entry : Entries)
    {
        if (!Entry.bEnabled || Entry.Type == ETMOPRoofAccessoryType::None) continue;
        if (!IsValid(Entry.Mesh))
        {
            OutWarnings.Add(FString::Printf(TEXT("%s: choose a mesh."), *Entry.AccessoryId.ToString()));
            continue;
        }
        UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Vehicle, NAME_None, RF_Transient);
        Vehicle->AddInstanceComponent(Part);
        Part->ComponentTags.Add(GeneratedTag);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetStaticMesh(Entry.Mesh);
        FString Warning;
        const bool bAttached = Attach(Part, Body, Vehicle->RoofAccessorySocket,
            Entry.SocketName, Entry.LocalTransform, Warning);
        if (!bAttached)
        {
            OutWarnings.Add(Entry.AccessoryId.ToString() + TEXT(": ") + Warning);
            Vehicle->RemoveInstanceComponent(Part);
            Part->DestroyComponent();
            continue;
        }
        if (IsValid(Entry.Material)) Part->SetMaterial(0, Entry.Material);
        Part->RegisterComponent();
    }
}

UMeshComponent* ResolveBodyMesh(ATMOPVehicleBase* Vehicle)
{
    if (!IsValid(Vehicle)) return nullptr;
    if (ATMOPConfiguredVehicle* Configured = Cast<ATMOPConfiguredVehicle>(Vehicle)) return Configured->BodyMesh;
    UMeshComponent* Body = nullptr;
    TInlineComponentArray<UMeshComponent*> Meshes(Vehicle);
    for (UMeshComponent* Part : Meshes)
        if (!Cast<UWidgetComponent>(Part) && !Part->ComponentHasTag(TEXT("TMOP.GeneratedVehicleAccessory")) &&
            (!Body || Part->Bounds.SphereRadius > Body->Bounds.SphereRadius)) Body = Part;
    return Body;
}

bool ApplyProfile(ATMOPVehicleBase* Vehicle, const FTMOPHistoricalVehicleRow& Profile, TArray<FString>* OutWarnings)
{
    if (!IsValid(Vehicle)) return false;
    if (ATMOPConfiguredVehicle* Configured = Cast<ATMOPConfiguredVehicle>(Vehicle))
    {
        Configured->VehicleModel = Profile.ModelData;
        Configured->bOverrideBodyColor = Profile.bOverrideBodyColor;
        Configured->BodyColor = Profile.BodyColor;
        Configured->RoofAccessory = Profile.RoofAccessory;
        Configured->AdditionalAccessories = Profile.AdditionalAccessories;
        const bool bApplied = Configured->ApplyConfiguration();
        if (OutWarnings) OutWarnings->Append(Configured->AccessoryWarnings);
        return bApplied;
    }
    // Bespoke vehicles retain their Blueprint appearance. Equipment can still be added.
    UMeshComponent* Body = ResolveBodyMesh(Vehicle);
    TArray<FTMOPVehicleAccessoryVisual> Equipment = Profile.AdditionalAccessories;
    if (Profile.RoofAccessory.Type != ETMOPRoofAccessoryType::None)
    {
        FTMOPVehicleAccessoryVisual Legacy;
        Legacy.AccessoryId = TEXT("RoofAccessory");
        Legacy.Type = Profile.RoofAccessory.Type;
        Legacy.Mesh = Profile.RoofAccessory.Mesh;
        Legacy.Material = Profile.RoofAccessory.Material;
        Legacy.SocketName = Profile.RoofAccessory.SocketName;
        Legacy.LocalTransform = Profile.RoofAccessory.LocalTransform;
        Equipment.Insert(Legacy, 0);
    }
    TArray<FString> Warnings;
    BuildAccessories(Vehicle, Body, Equipment, Warnings);
    if (OutWarnings) OutWarnings->Append(Warnings);
    return IsValid(Body);
}
}
