#pragma once
#include "CoreMinimal.h"
#include "Layout/Geometry.h"
#include "WorldAtlas/Flights/TMOPFlightData.h"
class SWidget;
class FSlateWindowElementList;
TSharedRef<SWidget> MakeTMOPFlightBrowser(TSharedRef<FTMOPFlightState> State, TFunction<void(int32)> Focus);
TSharedRef<SWidget> MakeTMOPFlightDetails(TSharedRef<FTMOPFlightState> State);
TSharedRef<SWidget> MakeTMOPFlightControls(TSharedRef<FTMOPFlightState> State);
void PaintTMOPFlights(const FTMOPFlightState& State, const FGeometry& G, FSlateWindowElementList& Out,
    int32 Layer, const FQuat& Rotation, const FVector2D& Center, double Radius);
int32 HitTMOPFlight(const FTMOPFlightState& State, const FQuat& Rotation, const FVector2D& Center,
    double Radius, const FVector2D& Mouse);
