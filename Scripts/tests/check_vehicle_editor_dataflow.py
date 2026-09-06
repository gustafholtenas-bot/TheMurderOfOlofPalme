"""Execute actual editor dataflow methods with a small C++ dependency shim.

This does NOT compile Unreal/Slate or reproduce the UI crash. It checks that
refresh/validation cannot copy stale command buffers over a newly selected row.
Pass --source to check an older STMOPVehicleEditor.cpp against the same cases.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile


def method(source, name):
    start = source.index("void STMOPVehicleEditor::" + name + "(")
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


SHIM = r'''
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
using int32 = int;
using FString = std::string;
#define TEXT(x) x
constexpr int32 INDEX_NONE = -1;
struct FName : std::string {
    using std::string::string;
    using std::string::operator=;
    bool IsNone() const { return empty(); }
    FString ToString() const { return *this; }
};
template<class T> struct TArray : std::vector<T> {
    void Reset() { this->clear(); }
    int32 Num() const { return static_cast<int32>(this->size()); }
    void Add(const T& item) { this->push_back(item); }
    bool IsValidIndex(int32 i) const { return i >= 0 && i < Num(); }
};
template<class T> struct TSharedPtr : std::shared_ptr<T> {
    TSharedPtr() = default;
    TSharedPtr(std::shared_ptr<T> p) : std::shared_ptr<T>(std::move(p)) {}
    bool IsValid() const { return bool(*this); }
    T* Get() const { return this->get(); }
};
template<class T, class... Args> TSharedPtr<T> MakeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template<class T> T&& MoveTemp(T& value) { return std::move(value); }
struct FTMOPHistoricalVehicleTimelineEntry { int32 Marker = 0; std::string Notes; };
struct FTMOPHistoricalVehicleRow {
    std::string VehicleId;
    std::string Notes;
    TArray<FTMOPHistoricalVehicleTimelineEntry> Timeline;
};
struct UObject { virtual ~UObject() = default; };
struct UTMOPVehicleDetailsObject : UObject { FTMOPHistoricalVehicleRow Data; };
struct UTMOPVehicleEntryDetailsObject : UObject { FTMOPHistoricalVehicleTimelineEntry Data; };
struct WeakObject {
    UObject* Pointer = nullptr;
    UObject* Get() const { return Pointer; }
    void Reset() { Pointer = nullptr; }
    WeakObject& operator=(UObject* p) { Pointer = p; return *this; }
};
namespace EPropertyChangeType { enum Type { ValueSet = 1, Interactive = 2 }; }
struct FPropertyChangedEvent {
    EPropertyChangeType::Type ChangeType = EPropertyChangeType::ValueSet;
    const UObject* Object = nullptr;
    const UObject* GetObjectBeingEdited(int32) const { return Object; }
};
struct FWidgetActiveTimerDelegate {
    template<class... Args> static int CreateSP(Args...) { return 0; }
};
template<class T> struct TGuardValue {
    T& Ref; T Old;
    TGuardValue(T& ref, T next) : Ref(ref), Old(ref) { Ref = next; }
    ~TGuardValue() { Ref = Old; }
};
struct Preview {
    FString VehicleId;
    void ShowVehicle(const FTMOPHistoricalVehicleRow& row) { VehicleId = row.VehicleId; }
    void Clear(const FString&) { VehicleId.clear(); }
    TArray<FName> GetSockets() { return {}; }
};
struct Combo { void RefreshOptions() {} };
struct FStructOnScope {
    void* Memory = nullptr;
    void* GetStructMemory() { return Memory; }
};
struct List { void RequestListRefresh() {} };
struct Cache { void Reset() {} };
class STMOPVehicleEditor {
public:
    FTMOPHistoricalVehicleRow WorkingRow, VehicleBuffer;
    FTMOPHistoricalVehicleTimelineEntry EntryBuffer;
    TSharedPtr<FStructOnScope> VehicleStruct, EntryStruct;
    TSharedPtr<UTMOPVehicleDetailsObject> VehicleDetailsObject;
    TSharedPtr<UTMOPVehicleEntryDetailsObject> EntryDetailsObject;
    WeakObject PendingEditedObject;
    TSharedPtr<Preview> AppearancePreview = MakeShared<Preview>();
    TSharedPtr<Combo> AccessorySocketCombo;
    TArray<TSharedPtr<FString>> AccessorySockets;
    bool bRefreshingAccessories = false;
    FName SelectedRowName = "SELECTED";
    bool bSynchronizingDetails = false, bPendingDetailsRefresh = false, bPreviewPlaying = false;
    int32 SelectedTimelineIndex = INDEX_NONE;
    TArray<TSharedPtr<int32>> TimelineItems;
    TSharedPtr<List> TimelineList;
    Cache CachedFingerprints;
    std::string CurrentErrors, CachedDrivingSummary, CachedRouteEndpoints, CachedOccupants;
    void CommitEntry();
    void CommitVehicle();
    void RebuildValidation();
    void RefreshTimeline();
    void RefreshAppearancePreview();
    void OnDetailsChanged(const FPropertyChangedEvent&, bool);
    void QueueDetailsRefresh();
    int ApplyPendingDetailsRefresh(double, float) { return 0; }
    void RegisterActiveTimer(float, int) {}
    std::string ValidateRow(const FTMOPHistoricalVehicleRow&) { return {}; }
    std::string BuildDrivingSummary() { return {}; }
    std::string BuildRouteEndpointsText() { return {}; }
    std::string BuildOccupantsText(int32) { return {}; }
    void RefreshValidationItems() {}
    void RebuildRoutePreview() {}
    void SyncDetailsFromWorking() {
        VehicleBuffer = WorkingRow;
        if (WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
            EntryBuffer = WorkingRow.Timeline[SelectedTimelineIndex];
        QueueDetailsRefresh();
    }
    STMOPVehicleEditor() {
        VehicleStruct = MakeShared<FStructOnScope>();
        EntryStruct = MakeShared<FStructOnScope>();
        VehicleStruct->Memory = &VehicleBuffer;
        EntryStruct->Memory = &EntryBuffer;
    }
};
'''

CASES = r'''
int main() {
    int failures = 0;
    auto check = [&](bool ok, const char* text) {
        std::cout << (ok ? "PASS: " : "FAIL: ") << text << '\n';
        if (!ok) ++failures;
    };
    STMOPVehicleEditor editor;
    // State immediately after SelectVehicle assigns WorkingRow and before
    // its RefreshTimeline call. The previous vehicle's buffer still exists.
    editor.VehicleBuffer.VehicleId = "PREVIOUS";
    editor.VehicleBuffer.Notes = "Previous car source";
    editor.WorkingRow.VehicleId = "SELECTED";
    editor.WorkingRow.Notes = "Selected car source";
    editor.WorkingRow.Timeline.resize(256);
    editor.WorkingRow.Timeline[0].Marker = 22;
    editor.RefreshTimeline();
    editor.RefreshAppearancePreview();
    check(editor.AppearancePreview->VehicleId == "SELECTED", "3D preview receives selected vehicle, not previous vehicle");
    check(editor.WorkingRow.VehicleId == "SELECTED", "row selection retains selected vehicle ID");
    check(editor.WorkingRow.Notes == "Selected car source", "row selection retains selected vehicle source");
    check(editor.WorkingRow.Timeline.Num() == 256, "large timeline retained");

    editor.SelectedTimelineIndex = 0;
    editor.EntryBuffer.Marker = 11;
    editor.WorkingRow.Timeline[0].Marker = 33;
    editor.RebuildValidation();
    check(editor.WorkingRow.Timeline[0].Marker == 33, "validation does not overwrite edited entry");

    // Explicit command commits must still work, independently of validation.
    editor.EntryBuffer.Marker = 44;
    editor.CommitEntry();
    check(editor.WorkingRow.Timeline[0].Marker == 44, "explicit entry commit works");
    editor.VehicleBuffer.Notes = "Authored metadata";
    editor.CommitVehicle();
    check(editor.WorkingRow.Notes == "Authored metadata", "explicit metadata commit works");
    check(editor.WorkingRow.Timeline.Num() == 256, "metadata commit preserves full timeline");

    editor.bPendingDetailsRefresh = false;
    editor.EntryDetailsObject = MakeShared<UTMOPVehicleEntryDetailsObject>();
    editor.EntryDetailsObject->Data = editor.WorkingRow.Timeline[0];
    FPropertyChangedEvent event;
    event.Object = editor.EntryDetailsObject.Get();
    editor.EntryDetailsObject->Data.Notes = "First notification";
    editor.OnDetailsChanged(event, false);
    editor.EntryDetailsObject->Data.Notes = "Final notification before next tick";
    editor.OnDetailsChanged(event, false);
    check(editor.WorkingRow.Timeline[0].Notes == "Final notification before next tick",
          "final edit from same object is not dropped while refresh is pending");

    // After rebinding, delayed notifications from the old object are invalid.
    const auto old_object = editor.EntryDetailsObject;
    editor.EntryDetailsObject = MakeShared<UTMOPVehicleEntryDetailsObject>();
    editor.EntryDetailsObject->Data.Notes = "Uncommitted new panel value";
    editor.bPendingDetailsRefresh = false;
    editor.WorkingRow.Timeline[0].Notes = "Keep working data";
    editor.SyncDetailsFromWorking();
    editor.bPendingDetailsRefresh = false;
    editor.OnDetailsChanged(event, false);
    check(editor.WorkingRow.Timeline[0].Notes == "Keep working data",
          "notification from old object cannot commit the replacement object's data");
    return failures ? 1 : 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[2] /
                        "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/STMOPVehicleEditor.cpp")
    args = parser.parse_args()
    source = args.source.read_text(encoding="utf-8")
    functions = "\n".join(method(source, n) for n in
                          ("CommitEntry", "CommitVehicle", "RebuildValidation", "RefreshTimeline",
                           "OnDetailsChanged", "QueueDetailsRefresh", "RefreshAppearancePreview"))
    with tempfile.TemporaryDirectory(prefix="tmop_dataflow_") as tmp:
        cpp = Path(tmp) / "dataflow.cpp"
        binary = Path(tmp) / "dataflow"
        cpp.write_text(SHIM + functions + CASES, encoding="utf-8")
        subprocess.run(["c++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
                        # The shim's FName is a string; UE's FName is a cheap value type.
                        "-Wno-range-loop-construct",
                        str(cpp), "-o", str(binary)], check=True)
        return subprocess.run([str(binary)], check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
