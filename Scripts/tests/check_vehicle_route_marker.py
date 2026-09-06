"""Run the actual vehicle-map Marker lambda with a checked-array C++ shim.

The shim enforces the self-element Add constraint shown by UE's assertion.
This does not compile Unreal/Slate. Pass --source to test an older source file.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile


SHIM = r'''
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
struct FVector2D {
    double X = 0, Y = 0;
    FVector2D() = default;
    FVector2D(double x, double y) : X(x), Y(y) {}
    FVector2D operator+(FVector2D b) const { return {X+b.X, Y+b.Y}; }
    FVector2D operator-(FVector2D b) const { return {X-b.X, Y-b.Y}; }
    FVector2D operator*(double n) const { return {X*n, Y*n}; }
    FVector2D GetSafeNormal() const {
        double n = std::hypot(X,Y); return n > 0 ? FVector2D(X/n,Y/n) : FVector2D();
    }
    bool operator==(FVector2D b) const { return X==b.X && Y==b.Y; }
};
struct Rotation {
    FVector2D Forward;
    FVector2D GetForwardVector() const { return Forward; }
};
struct FTransform {
    FVector2D Location, Forward;
    FVector2D GetLocation() const { return Location; }
    Rotation GetRotation() const { return {Forward}; }
};
struct FString : std::string {
    using std::string::string;
    bool IsEmpty() const { return empty(); }
};
struct FLinearColor {};
template<class T> struct TArray : std::vector<T> {
    TArray(std::initializer_list<T> items) : std::vector<T>(items) {}
    void Add(const T& item) {
        auto address = reinterpret_cast<std::uintptr_t>(&item);
        auto begin = reinterpret_cast<std::uintptr_t>(this->data());
        auto end = begin + this->capacity()*sizeof(T);
        if (address >= begin && address < end)
            throw std::runtime_error("self-element Add: same container assertion as reported");
        this->push_back(item);
    }
};
int main() {
    const FVector2D Size(800,600);
    auto Project = [](FVector2D p, FVector2D) { return p; };
    std::vector<FVector2D> Painted;
    float PaintedWidth = 0;
    int Labels = 0;
    auto Line = [&](const TArray<FVector2D>& points, FLinearColor, float width) {
        Painted.assign(points.begin(),points.end()); PaintedWidth = width;
    };
    auto Label = [&](FVector2D, const FString&, FLinearColor) { ++Labels; };
'''

CASES = r'''
    int failures = 0;
    for (bool car : {false,true}) {
        for (FVector2D direction : {FVector2D(1,0),FVector2D(0,1),FVector2D(0,0)}) {
            try {
                Labels = 0;
                Marker(FTransform{{100,200},direction}, car ? FString() : FString("FROM"), {}, car);
                const bool ok = Painted.size()==(car ? 4u : 3u)
                    && (!car || Painted.front()==Painted.back())
                    && PaintedWidth==(car ? 3.f : 2.f) && Labels==(car ? 0 : 1);
                if (!ok) ++failures;
                std::cout << (ok ? "PASS: " : "FAIL: ") << (car ? "vehicle" : "endpoint")
                          << " direction=" << direction.X << ',' << direction.Y << '\n';
            } catch (const std::exception& e) {
                ++failures; std::cout << "FAIL: " << e.what() << '\n';
            }
        }
    }
    return failures ? 1 : 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[2] /
                        "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/STMOPVehicleEditor.cpp")
    args = parser.parse_args()
    source = args.source.read_text(encoding="utf-8")
    start = source.index("auto Marker = [&]")
    end = source.index("\n        };", start) + len("\n        };")
    with tempfile.TemporaryDirectory(prefix="tmop_marker_") as tmp:
        cpp = Path(tmp) / "marker.cpp"
        binary = Path(tmp) / "marker"
        cpp.write_text(SHIM + source[start:end] + CASES, encoding="utf-8")
        subprocess.run(["c++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(binary)], check=True)
        return subprocess.run([str(binary)], check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
