#pragma once

#include "CoreMinimal.h"

/** East-positive longitude, north-positive latitude, degrees. Camera is at +X,
 * looking towards the origin: screen right is -Y and screen up is +Z. */
namespace TMOPGlobe
{
    inline FVector Unit(double Latitude, double Longitude)
    {
        const double Lat = FMath::DegreesToRadians(Latitude);
        const double Lon = FMath::DegreesToRadians(Longitude);
        return FVector(FMath::Cos(Lat) * FMath::Cos(Lon),
            -FMath::Cos(Lat) * FMath::Sin(Lon), FMath::Sin(Lat));
    }

    inline FQuat View(double Latitude, double Longitude)
    {
        // Longitude increases eastwards; camera-right is -Y. Mirror-free globe.
        return FQuat(FVector::YAxisVector, FMath::DegreesToRadians(Latitude)) *
            FQuat(FVector::ZAxisVector, FMath::DegreesToRadians(Longitude));
    }

    inline bool Project(const FVector& UnitPoint, const FQuat& Rotation,
        const FVector2D& Center, double Radius, FVector2D& Out)
    {
        const FVector P = Rotation.RotateVector(UnitPoint);
        Out = Center + FVector2D(-P.Y, -P.Z) * Radius;
        return P.X > 0.002; // Hide the far hemisphere and the ambiguous limb.
    }

    inline FVector Arc(const FVector& A, const FVector& B, double T)
    {
        const double Dot = FMath::Clamp(FVector::DotProduct(A, B), -1.0, 1.0);
        if (Dot > 0.9999) return FMath::Lerp(A, B, T).GetSafeNormal();
        if (Dot < -0.9999)
        {
            // Deterministic great circle for antipodal endpoints.
            const FVector Axis = FVector::CrossProduct(A,
                FMath::Abs(A.Z) < 0.9 ? FVector::ZAxisVector : FVector::YAxisVector).GetSafeNormal();
            return FQuat(Axis, PI * T).RotateVector(A);
        }
        const double Angle = FMath::Acos(Dot);
        return (A * FMath::Sin((1.0 - T) * Angle) + B * FMath::Sin(T * Angle)) / FMath::Sin(Angle);
    }
}
