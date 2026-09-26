"""Guard against presenting orders, seizures or allegations as delivered arms."""
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ExportResearchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        world = json.loads((ROOT / 'Plugins/TMOPEngine/Content/WorldAtlas/world.json').read_text())
        cls.entries = {e['id']: e for e in world['entries']}
        cls.manifest = json.loads((ROOT / 'Tools/WorldAtlas/SOURCES/BOFORS_RESEARCH_2026-09-26.json').read_text())

    def test_cases_and_branches_have_reciprocal_navigation(self):
        for id in self.manifest['new_entry_ids']:
            e = self.entries[id]
            for target in e['related']:
                self.assertIn(id, self.entries[target]['related'], (id, target))
        for id in self.manifest['export_route_branch_ids']:
            e = self.entries[id]
            self.assertFalse(e['shipment']['quantities'])
            self.assertTrue(e['shipment']['quantity_included_in_parent'])
            self.assertIn(e['shipment']['parent_case'], e['related'])

    def test_unfulfilled_and_unverified_cases_have_no_delivery_arrows(self):
        for id in self.manifest['export_case_ids']:
            e = self.entries[id]
            if e['shipment']['status'] in {'cancelled', 'denied', 'unverified_claim', 'contract_after_reference_date', 'retrospective_overview'}:
                self.assertEqual(e['route'], [], id)
        india = self.entries['bofors-india-contract']
        self.assertGreater(india['from'], '1986-02-28')
        self.assertTrue(india['later_only'])
        self.assertEqual(india['shipment']['quantities'][0]['basis'], 'contract_quantity')

    def test_partial_exports_do_not_become_full_order_quantities(self):
        def quantities(id):
            return {q['basis']: q['value'] for q in self.entries[id]['shipment']['quantities']}
        rp = quantities('bofors-rp13')
        self.assertEqual((rp['order'], rp['reported_export'], rp['seized_not_exported']), (155, 115, 20))
        tirrena = quantities('bofors-tirrena-propellant')
        self.assertEqual(tirrena['exported_to_italy'], tirrena['reported_onward_to_iran'] + tirrena['returned_to_sweden'])
        self.assertEqual(tirrena['reported_onward_to_iran'], 50)
        fdsp = quantities('bofors-fdsp')
        self.assertAlmostEqual(fdsp['exported_to_yugoslavia'] + fdsp['stopped_not_exported'], 350)
        self.assertEqual(self.entries['bofors-fdsp']['route'][-1], 'export-place-yu')
        self.assertEqual(self.entries['bofors-oman-ammunition-1985']['route'][-1], 'export-place-sg')
        self.assertNotIn('ir', self.entries['bofors-dnw']['route'])

    def test_document_dates_and_access_limitations_are_preserved(self):
        for id in self.manifest['security_document_ids']:
            e = self.entries[id]
            self.assertEqual((e['from'], e['to']), ('1986-02-28', '1986-02-28'))
            self.assertTrue(e['sources'])
            self.assertFalse(e['route'])
        third_force = self.entries['sadf-third-force-19860228']
        self.assertEqual(third_force['evidence_status'], 'later_submission_citing_document')
        self.assertEqual(third_force['sources'][0]['published'], '1997-08-04')
        cia = self.entries['bofors-cia-review-1988']
        self.assertEqual(cia['sources'][0]['published'], '1988-03-04')
        self.assertNotEqual(cia.get('record_type'), 'security_document')


if __name__ == '__main__':
    unittest.main()
