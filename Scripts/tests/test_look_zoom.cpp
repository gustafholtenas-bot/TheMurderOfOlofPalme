#include "../../Plugins/TMOPEngine/Source/TMOPEngine/Private/Player/TMOPLookZoomMath.h"
#include <iostream>
#include <limits>

int main()
{
    using namespace TMOPLookZoomMath;
    const float NaN = std::numeric_limits<float>::quiet_NaN();
    const float Inf = std::numeric_limits<float>::infinity();
    struct Case { const char* Name; float Actual; float Expected; };
    const Case Cases[] = {
        {"released preserves third-person FOV", FieldOfView(85, 40, 0), 85},
        {"held zoom reaches configured FOV", FieldOfView(90, 40, 1), 40},
        {"analog half zoom", FieldOfView(90, 40, 0.5f), 65},
        {"a tighter base camera never zooms out", FieldOfView(35, 40, 1), 35},
        {"out-of-range request clamps", FieldOfView(90, 40, 2), 40},
        {"negative request releases", FieldOfView(90, 40, -1), 90},
        {"invalid request releases", FieldOfView(90, 40, NaN), 90},
        {"invalid zoom FOV keeps base", FieldOfView(85, NaN, 1), 85},
        {"nonfinite device input releases", Amount(Inf), 0},
        {"trigger resting below zero", AnalogAmount(-1, 0.04f), 0},
        {"trigger drift ignored", AnalogAmount(0.03f, 0.04f), 0},
        {"trigger dead-zone boundary", AnalogAmount(0.04f, 0.04f), 0},
        {"trigger normalized half", AnalogAmount(0.52f, 0.04f), 0.5f},
        {"trigger full travel", AnalogAmount(1, 0.04f), 1},
        {"bad dead zone cannot divide by zero", AnalogAmount(1, 1), 1},
        {"pinch starts without a jump", PinchAmount(100, 100, 2), 0},
        {"pinch spreads halfway", PinchAmount(150, 100, 2), 0.5f},
        {"pinch full spread", PinchAmount(200, 100, 2), 1},
        {"pinch narrows back to neutral", PinchAmount(50, 100, 2), 0},
        {"coincident touches cannot divide by zero", PinchAmount(100, 0, 2), 0},
        {"bad pinch ratio releases", PinchAmount(100, 100, 1), 0},
        {"invalid touch position releases", PinchAmount(NaN, 100, 2), 0},
        {"unzoomed look sensitivity", LookSensitivity(90, 90), 1},
        {"40 degree zoom sensitivity", LookSensitivity(40, 90), 0.3639702f},
        {"wide FOV never accelerates look", LookSensitivity(120, 90), 1},
    };
    for (const auto& Test : Cases)
        if (!std::isfinite(Test.Actual) || std::fabs(Test.Actual - Test.Expected) > 0.0001f)
        {
            std::cerr << "FAIL: " << Test.Name << ": " << Test.Actual << '\n';
            return 1;
        }
    std::cout << sizeof(Cases) / sizeof(Case) << " look-zoom cases passed\n";
    return 0;
}
