#include "TMOPLaneNetworkActor.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

ATMOPLaneNetworkActor::ATMOPLaneNetworkActor()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;
}

void ATMOPLaneNetworkActor::BeginPlay()
{
    Super::BeginPlay();
    if (bLoadOnBeginPlay)
    {
        LoadLaneNetwork();
    }
}

void ATMOPLaneNetworkActor::ClearGeneratedSplines()
{
    TArray<USplineComponent*> Existing;
    GetComponents(Existing);
    for (USplineComponent* Spline : Existing)
    {
        if (Spline && Spline->ComponentHasTag(TEXT("TMOPGeneratedLane")))
        {
            Spline->DestroyComponent();
        }
    }
    Lanes.Reset();
    LaneIndexByID.Reset();
    NextLanesByID.Reset();
}

static bool TMOPReadPoints(const TSharedPtr<FJsonObject>& Object, TArray<FVector>& OutPoints)
{
    const TArray<TSharedPtr<FJsonValue>>* JsonPoints = nullptr;
    if (!Object->TryGetArrayField(TEXT("pointsCm"), JsonPoints)) return false;
    for (const TSharedPtr<FJsonValue>& PointValue : *JsonPoints)
    {
        const TArray<TSharedPtr<FJsonValue>>* XYZ = nullptr;
        if (!PointValue->TryGetArray(XYZ) || XYZ->Num() != 3) continue;
        OutPoints.Emplace((*XYZ)[0]->AsNumber(), (*XYZ)[1]->AsNumber(), (*XYZ)[2]->AsNumber());
    }
    return OutPoints.Num() >= 2;
}

bool ATMOPLaneNetworkActor::LoadLaneNetwork()
{
    ClearGeneratedSplines();
    const FString FullPath = FPaths::IsRelative(JsonPath)
        ? FPaths::Combine(FPaths::ProjectContentDir(), JsonPath) : JsonPath;
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP: Could not read lane JSON: %s"), *FullPath);
        return false;
    }
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP: Invalid lane JSON: %s"), *FullPath);
        return false;
    }

    auto AddItems = [this](const TArray<TSharedPtr<FJsonValue>>& Items, bool bCrossing)
    {
        for (const TSharedPtr<FJsonValue>& Value : Items)
        {
            const TSharedPtr<FJsonObject> Obj = Value->AsObject();
            if (!Obj.IsValid()) continue;
            FString ID; if (!Obj->TryGetStringField(TEXT("laneId"), ID) || ID.IsEmpty()) continue;
            TArray<FVector> Points; if (!TMOPReadPoints(Obj, Points)) continue;

            FTMOPLaneRuntimeData Data;
            Data.LaneID = FName(*ID);
            Data.bIsCrossing = bCrossing;
            FString RoadID;
            if (Obj->TryGetStringField(TEXT("roadId"), RoadID)) Data.RoadID = FName(*RoadID);
            Obj->TryGetStringField(TEXT("direction"), Data.Direction);
            double Index = 1.0; Obj->TryGetNumberField(TEXT("laneIndexFromRight"), Index);
            Data.LaneIndexFromRight = FMath::RoundToInt(Index);
            Obj->TryGetBoolField(TEXT("legalForNormalTraffic"), Data.bLegalForNormalTraffic);
            FString Temp;
            if (Obj->TryGetStringField(TEXT("fromLaneId"), Temp)) Data.FromLaneID = FName(*Temp);
            if (Obj->TryGetStringField(TEXT("toLaneId"), Temp)) Data.ToLaneID = FName(*Temp);
            Obj->TryGetStringField(TEXT("turnType"), Data.TurnType);

            USplineComponent* Spline = NewObject<USplineComponent>(this, *FString::Printf(TEXT("Lane_%s"), *ID));
            Spline->ComponentTags.Add(TEXT("TMOPGeneratedLane"));
            Spline->SetupAttachment(RootComponent);
            Spline->RegisterComponent();
            Spline->ClearSplinePoints(false);
            for (const FVector& Point : Points)
            {
                // JSON locations are absolute world coordinates. Convert them
                // to this actor's local space so the actor may live at origin.
                Spline->AddSplinePoint(GetActorTransform().InverseTransformPosition(Point), ESplineCoordinateSpace::Local, false);
            }
            Spline->SetClosedLoop(false, false);
            Spline->UpdateSpline();
            Spline->SetDrawDebug(bDrawSplines);
            Data.Spline = Spline;
            LaneIndexByID.Add(Data.LaneID, Lanes.Num());
            if (bCrossing && !Data.FromLaneID.IsNone() && !Data.ToLaneID.IsNone())
            {
                NextLanesByID.Add(Data.FromLaneID, Data.ToLaneID);
            }
            Lanes.Add(MoveTemp(Data));
        }
    };

    const TArray<TSharedPtr<FJsonValue>>* LaneItems = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* CrossingItems = nullptr;
    if (Root->TryGetArrayField(TEXT("lanes"), LaneItems)) AddItems(*LaneItems, false);
    if (Root->TryGetArrayField(TEXT("crossings"), CrossingItems)) AddItems(*CrossingItems, true);
    UE_LOG(LogTemp, Display, TEXT("TMOP: Loaded %d lane/crossing splines from %s"), Lanes.Num(), *FullPath);
    return Lanes.Num() > 0;
}

bool ATMOPLaneNetworkActor::FindLane(FName LaneID, FTMOPLaneRuntimeData& OutLane) const
{
    const int32* Index = LaneIndexByID.Find(LaneID);
    if (!Index || !Lanes.IsValidIndex(*Index)) return false;
    OutLane = Lanes[*Index]; return true;
}

TArray<FName> ATMOPLaneNetworkActor::GetNextLaneIDs(FName LaneID, bool bAllowIllegalTraffic) const
{
    TArray<FName> Candidates; NextLanesByID.MultiFind(LaneID, Candidates);
    if (bAllowIllegalTraffic) return Candidates;
    return Candidates.FilterByPredicate([this](FName Candidate)
    {
        const int32* Index = LaneIndexByID.Find(Candidate);
        return Index && Lanes.IsValidIndex(*Index) && Lanes[*Index].bLegalForNormalTraffic;
    });
}
