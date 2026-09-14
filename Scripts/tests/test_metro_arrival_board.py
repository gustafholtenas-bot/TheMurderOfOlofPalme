import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
TABLE = ROOT / "DataTables/09_13/DT_TMOP_MetroArrivals_Northbound.json"
SOURCE = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine"


class MetroArrivalBoardTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows = json.loads(TABLE.read_text(encoding="utf-8"))

    def test_two_station_timetable_is_complete(self):
        self.assertEqual(len(self.rows), 14)
        by_station = {}
        for row in self.rows:
            by_station.setdefault(row["StationId"], []).append(row)
            self.assertEqual(row["Direction"], "Northbound")
        self.assertEqual(set(by_station), {"HOTORGET", "RADMANS_GATAN"})
        self.assertEqual(len(by_station["HOTORGET"]), 7)
        self.assertEqual(len(by_station["RADMANS_GATAN"]), 7)

    def test_exact_arrival_times(self):
        actual = {
            (row["StationId"], row["Line"], row["ArrivalTime"]["Minute"],
             row["ArrivalTime"]["Second"])
            for row in self.rows
        }
        expected = {
            ("HOTORGET", 17, 4, 0), ("HOTORGET", 19, 9, 0),
            ("HOTORGET", 18, 14, 0), ("HOTORGET", 17, 24, 0),
            ("HOTORGET", 19, 29, 0), ("HOTORGET", 18, 34, 0),
            ("HOTORGET", 17, 44, 0),
            ("RADMANS_GATAN", 17, 5, 40), ("RADMANS_GATAN", 19, 10, 40),
            ("RADMANS_GATAN", 18, 15, 40), ("RADMANS_GATAN", 17, 25, 40),
            ("RADMANS_GATAN", 19, 30, 40), ("RADMANS_GATAN", 18, 35, 40),
            ("RADMANS_GATAN", 17, 45, 40),
        }
        self.assertEqual(actual, expected)
        outside = [row for row in self.rows if not row["bWithinSimulationWindow"]]
        self.assertEqual([row["Name"] for row in outside],
                         ["RADMANS_GATAN_N_234540_L17"])

    def test_runtime_auto_installs_and_disclaims_missing_direction(self):
        anchor = (SOURCE / "Private/Anchors/TMOPHistoricalAnchor.cpp").read_text(encoding="utf-8")
        board = (SOURCE / "Private/Transit/TMOPMetroEntranceBoardComponent.cpp").read_text(encoding="utf-8")
        self.assertIn('StartsWith(TEXT("MetroHotorget")', anchor)
        self.assertIn('StartsWith(TEXT("MetroRadmansgatan")', anchor)
        self.assertIn('Contains(TEXT("inside")', anchor)
        self.assertIn("Södergående tidtabell saknas", board)
        self.assertIn("GetCurrentTimeSecondsExact", board)
        self.assertIn("UTMOPMetroBoardWidget::StaticClass()", board)


if __name__ == "__main__":
    unittest.main()

