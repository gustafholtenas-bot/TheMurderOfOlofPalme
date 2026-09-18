import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/UI/TMOPPauseMenuWidget.cpp"
HEADER = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/UI/TMOPPauseMenuWidget.h"


class SwedishNavigationTests(unittest.TestCase):
    def test_navigation_order(self):
        source = SOURCE.read_text(encoding="utf-8")
        entries = re.findall(r"AddNavigationEntry\(ETMOPPauseHubSection::(\w+)\);", source)
        self.assertEqual(entries, [
            "Inventory", "Publications", "Map", "MyObservations", "Theories", "TheoryBuilder",
            "MurderDayMysteries", "MurderKnowledge", "AfterMurderEvents",
            "WorldGroups", "SwedenGroups", "Sources", "Settings", "Controls", "SaveLoad", "Quit",
        ])
        self.assertIn("FORTSÄTT SPELA", source)
        self.assertIn("FÖRFLYTTA I TID", source)
        self.assertIn("SScrollBox::Slot()[NavigationPanel]", source)
        self.assertEqual(source.count("AddNavigationGap("), 6)

    def test_new_sections_have_titles_and_pages(self):
        source = SOURCE.read_text(encoding="utf-8")
        header = HEADER.read_text(encoding="utf-8")
        for section in ["MyObservations", "MurderKnowledge", "Theories", "MurderDayMysteries", "AfterMurderEvents",
                        "WorldGroups", "SwedenGroups", "TheoryBuilder"]:
            self.assertIn(section, header)
            self.assertEqual(source.count("case ETMOPPauseHubSection::" + section + ":"), 2)
        self.assertIn("case ETMOPPauseHubSection::Sources: BuildSourcesPage();", source)
        self.assertIn("case ETMOPPauseHubSection::Evidence: BuildEvidencePage();", source)
        new_pages = source.split("case ETMOPPauseHubSection::MyObservations:")[-1].split("break;")[0]
        self.assertNotIn("BuildEvidencePage", new_pages)
        self.assertNotIn("BuildSourcesPage", new_pages)
        self.assertIn("BuildNotebookPage", new_pages)


if __name__ == "__main__":
    unittest.main()
