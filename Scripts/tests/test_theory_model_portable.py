"""Execute the actual model .cpp with a tiny type shim when Unreal is unavailable.

This verifies graph/discovery/template behavior, not UHT, Slate or UE serialization.
Run TMOP.TheoryBuilder in Unreal Automation for engine integration/save tests.
"""
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine"

SHIM = r'''
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>
#include <cstdint>
#define USTRUCT(...)
#define UENUM(...)
#define UPROPERTY(...)
#define UMETA(...)
#define GENERATED_BODY()
#define TMOPENGINE_API
#define TEXT(x) L##x
using int32 = int;
using uint8 = unsigned char;
using TCHAR = wchar_t;
struct FString : std::wstring {
    using std::wstring::wstring;
    FString() = default;
    FString(const std::wstring& S) : std::wstring(S) {}
    bool IsEmpty() const { return empty(); }
    void Empty() { clear(); }
    FString ToUpper() const { auto S = *this; for (auto& C : S) C = std::towupper(C); return S; }
    bool StartsWith(const FString& S) const { return find(S) == 0; }
    bool EndsWith(const FString& S) const { return size() >= S.size() && compare(size()-S.size(), S.size(), S) == 0; }
};
struct FName {
    FString Value;
    FName() = default;
    FName(const wchar_t* S) : Value(FString(S).ToUpper()) {}
    bool IsNone() const { return Value.empty(); }
    FString ToString() const { return Value; }
    bool operator==(const FName& O) const { return Value == O.Value; }
    bool operator!=(const FName& O) const { return !(*this == O); }
};
inline FName NAME_None;
struct FText { FString Value; static FText FromString(const FString& S) { return {S}; } FString ToString() const { return Value; } };
struct FVector2D { double X=0, Y=0; FVector2D() = default; FVector2D(double A, double B):X(A),Y(B){} static const FVector2D ZeroVector; };
inline const FVector2D FVector2D::ZeroVector;
struct FGuid {
    uint64_t Value=0;
    static FGuid NewGuid() { static uint64_t Counter=0; return {++Counter}; }
    bool operator==(const FGuid& O) const { return Value == O.Value; }
    bool operator!=(const FGuid& O) const { return !(*this == O); }
};
template<class T> struct TArray : std::vector<T> {
    using std::vector<T>::vector;
    void Add(const T& V) { this->push_back(V); }
    int32 Num() const { return static_cast<int32>(this->size()); }
    bool IsEmpty() const { return this->empty(); }
    T& Last() { return this->back(); }
    template<class F> bool ContainsByPredicate(F P) const { return std::any_of(this->begin(), this->end(), P); }
    template<class F> T* FindByPredicate(F P) { auto I=std::find_if(this->begin(), this->end(), P); return I==this->end()?nullptr:&*I; }
    template<class F> const T* FindByPredicate(F P) const { auto I=std::find_if(this->begin(), this->end(), P); return I==this->end()?nullptr:&*I; }
    template<class F> void RemoveAll(F P) { this->erase(std::remove_if(this->begin(), this->end(), P), this->end()); }
};
struct FMath { template<class T> static T Clamp(T V, T A, T B) { return std::clamp(V,A,B); } };
'''

DRIVER = r'''
#include "Research/TMOPTheoryTypes.h"
#include <cassert>
#include <iostream>
int main() {
    const int People[] = {1, 5, 14, 19}, Cars[] = {0, 2, 5, 7};
    FTMOPNotebookObservation Shooter;
    Shooter.EntityId=TEXT("THE_KILLER"); Shooter.Category=ETMOPNotebookCategory::Shooter;
    Shooter.DisplayName=FText::FromString(TEXT("Mannen med revolvern"));
    for (int I=0; I<4; ++I) {
        auto T=TMOPTheory::CreateTemplate(I);
        int P=0,V=0,S=0;
        for (const auto& N:T.Nodes) {
            P += N.Kind==ETMOPTheoryNodeKind::Person;
            V += N.Kind==ETMOPTheoryNodeKind::Vehicle;
            S += N.bShooter;
            assert(N.Size.X>0 && N.Size.Y>0);
        }
        assert(P==People[I] && V==Cars[I] && S==1 && T.Links.IsEmpty());
        auto* Root=T.Nodes.FindByPredicate([](const auto& N){return N.bShooter;});
        assert(Root->EntityId.IsNone());
        TMOPTheory::SynchronizeShooter(T,{Shooter});
        assert(Root->EntityId==Shooter.EntityId);
        assert(!TMOPTheory::RemoveNode(T,Root->Id));
        assert(!TMOPTheory::AssignObservation(T,Root->Id, TEXT("OTHER"),{Shooter}));
        TMOPTheory::SynchronizeShooter(T,{});
        assert(Root->EntityId.IsNone());
    }
    FTMOPNotebookObservation Person, Car;
    Person.EntityId=TEXT("OBSERVED_SHARED");
    Car.EntityId=Person.EntityId; Car.Kind=ETMOPNotebookEntityKind::Vehicle;
    TArray<FTMOPNotebookObservation> O;
    assert(TMOPNotebook::AddUnique(O,Person)); assert(TMOPNotebook::AddUnique(O,Car));
    assert(!TMOPNotebook::AddUnique(O,Car));
    assert(TMOPNotebook::IsVehicleEligible(TEXT("CAR"),TEXT("OBSERVED_CAR")));
    assert(!TMOPNotebook::IsVehicleEligible(TEXT("CAR"),TEXT("CIVILIAN")));
    auto T=TMOPTheory::CreateTemplate(0);
    auto A=TMOPTheory::AddNode(T,ETMOPTheoryNodeKind::Person,{},TEXT("Person"));
    auto B=TMOPTheory::AddNode(T,ETMOPTheoryNodeKind::Vehicle,{},TEXT("Car"));
    auto C=TMOPTheory::AddNode(T,ETMOPTheoryNodeKind::Person,{},TEXT("Person 2"));
    assert(!TMOPTheory::AssignObservation(T,A,Person.EntityId,{}));
    assert(!TMOPTheory::AssignObservation(T,A,Car.EntityId,{Car}));
    assert(TMOPTheory::AssignObservation(T,A,Person.EntityId,O));
    assert(TMOPTheory::AssignObservation(T,B,Car.EntityId,O));
    assert(!TMOPTheory::AssignObservation(T,C,Person.EntityId,O));
    assert(TMOPTheory::AddLink(T,A,B)); assert(!TMOPTheory::AddLink(T,B,A));
    assert(!TMOPTheory::AddLink(T,A,A)); assert(!TMOPTheory::AddLink(T,A,FGuid::NewGuid()));
    const auto Undo=T;
    assert(TMOPTheory::RemoveNode(T,A)); assert(T.Links.IsEmpty());
    T=Undo; assert(T.Links.Num()==1);
    assert(T.Nodes.FindByPredicate([A](const auto& N){return N.Id==A;})->EntityId==Person.EntityId);
    assert(TMOPTheory::DefaultInformation().Num()==9);
    std::cout << "4 reference layouts, discovery, type restrictions, deduplication, links, deletion and undo snapshots passed\n";
}
'''


class TheoryModelTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("g++"), "g++ required for portable model checks")
    def test_actual_cpp_model(self):
        with tempfile.TemporaryDirectory() as temp:
            d = pathlib.Path(temp)
            (d / "Engine").mkdir()
            (d / "UObject").mkdir()
            (d / "UObject/SoftObjectPath.h").write_text("#pragma once\nstruct FSoftObjectPath {};\n")
            (d / "CoreMinimal.h").write_text(SHIM, encoding="utf-8")
            (d / "Engine/DataTable.h").write_text("#pragma once\nstruct FTableRowBase {};\n")
            for name in ["TMOPNotebookTypes.generated.h", "TMOPTheoryTypes.generated.h"]:
                (d / name).write_text("")
            (d / "main.cpp").write_text(DRIVER, encoding="utf-8")
            subprocess.run(["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-I", str(d),
                            "-I", str(MODULE / "Public"), str(d / "main.cpp"),
                            str(MODULE / "Private/Research/TMOPTheoryTypes.cpp"), "-o", str(d / "test")],
                           check=True, capture_output=True, text=True)
            result = subprocess.run([str(d / "test")], check=True, capture_output=True, text=True)
            self.assertIn("passed", result.stdout)


if __name__ == "__main__":
    unittest.main()
