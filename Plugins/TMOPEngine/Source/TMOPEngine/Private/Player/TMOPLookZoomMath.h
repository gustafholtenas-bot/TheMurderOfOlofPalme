#pragma once

#include <algorithm>
#include <cmath>

namespace TMOPLookZoomMath
{
inline float Amount(float Value)
{
    return std::isfinite(Value) ? std::clamp(Value, 0.0f, 1.0f) : 0.0f;
}
inline float FieldOfView(float Base, float FullZoom, float Alpha)
{
    Base = std::isfinite(Base) ? std::clamp(Base, 5.0f, 170.0f) : 90.0f;
    FullZoom = std::isfinite(FullZoom) ? std::clamp(FullZoom, 5.0f, Base) : Base;
    return Base + (FullZoom - Base) * Amount(Alpha);
}
inline float AnalogAmount(float Value, float DeadZone)
{
    DeadZone = std::isfinite(DeadZone) ? std::clamp(DeadZone, 0.0f, 0.95f) : 0.0f;
    return Amount((Amount(Value) - DeadZone) / (1.0f - DeadZone));
}
inline float PinchAmount(float Distance, float InitialDistance, float FullZoomRatio)
{
    if (!std::isfinite(Distance) || !std::isfinite(InitialDistance) ||
        !std::isfinite(FullZoomRatio) || InitialDistance < 1.0f || FullZoomRatio <= 1.0f)
        return 0.0f;
    return Amount((Distance / InitialDistance - 1.0f) / (FullZoomRatio - 1.0f));
}
inline float LookSensitivity(float CurrentFOV, float BaseFOV)
{
    constexpr float HalfDegreesToRadians = 0.00872664626f;
    const float Base = FieldOfView(BaseFOV, BaseFOV, 0.0f);
    const float Current = FieldOfView(CurrentFOV, CurrentFOV, 0.0f);
    return std::clamp(std::tan(Current * HalfDegreesToRadians) /
        std::tan(Base * HalfDegreesToRadians), 0.1f, 1.0f);
}
}
