"""Check the dated South African personnel cards, branch placement and portraits."""
import copy
import hashlib
import json
import unittest
from validate_hierarchy import CONTENT, validate_entry

class SouthAfricaPersonnelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entry=next(x for x in json.loads((CONTENT/'world.json').read_text())['entries'] if x['id']=='za')
        research=CONTENT.parents[3]/'Tools/WorldAtlas/Research'
        cls.research=json.loads((research/'south_africa_personnel_1986.json').read_text())
        cls.portraits=json.loads((research/'south_africa_portraits_1986.json').read_text())
        cls.people={x['id']:x for x in cls.research['people']}
        cls.nodes={x['id']:x for x in cls.entry['hierarchy']['nodes']}

    def test_people_have_sourced_localized_dated_cards(self):
        self.assertEqual(len(self.people),23)
        for id,p in self.people.items():
            n=self.nodes[id]
            self.assertEqual(n['parent'],p['organization'])
            self.assertEqual(n['relation'],'group')
            self.assertEqual(p['reference_date'],'1986-02-28')
            self.assertTrue(p['sources'] and p['evidence_status'])
            for field in ['label','note']:
                value=self.entry['text'][n[field]]
                self.assertTrue(value['sv'] and value['en'])
                self.assertEqual(value['en_source'],value['sv'])

    def test_transitions_and_later_titles_are_not_backdated(self):
        self.assertEqual(self.nodes['craig-williamson']['parent'],'dcc')
        self.assertEqual(self.nodes['jack-cronje']['parent'],'northern-transvaal')
        self.assertEqual(self.nodes['roelf-venter']['parent'],'c2')
        self.assertEqual(self.nodes['pieter-van-der-westhuizen']['parent'],'ssc-secretariat')
        self.assertEqual(self.nodes['andries-putter']['parent'],'military-intelligence')
        self.assertEqual(self.nodes['joep-joubert']['parent'],'special-forces')
        self.assertNotIn('ccb',self.nodes)
        self.assertNotIn('dirk-coetzee',self.nodes)
        self.assertEqual(self.nodes['nis']['label'],'office-nis')
        self.assertNotIn('Barnard',self.entry['text']['office-nis']['en'])
        self.assertEqual(self.nodes['niel-barnard']['parent'],'nis')

    def test_organizations_are_distinct_and_candidates_are_not_staff(self):
        expected={'nis':'services','sap':'services','security-branch':'sap','c-section':'security-branch',
                  'vlakplaas-c1':'c-section','c2':'c-section','northern-transvaal':'security-branch',
                  'sadf':'services','military-intelligence':'sadf','dcc':'military-intelligence',
                  'special-forces':'sadf','ssc-secretariat':'services'}
        for id,parent in expected.items():self.assertEqual(self.nodes[id]['parent'],parent)
        self.assertEqual(self.research['year_only_people'],[])
        self.assertEqual(self.research['review_people'],[])
        self.assertEqual(len(self.research['changes_from_IL06']['added_people']),17)
        self.assertNotIn('niel-barnard',self.research['changes_from_IL06']['added_people'])

    def test_portrait_credit_date_and_file_hash(self):
        self.assertEqual(len(self.portraits),1)
        p=self.portraits[0]
        self.assertEqual(p['person_id'],'eugene-de-kock')
        self.assertEqual(p['source_date'],'1997')
        self.assertEqual(p['author'],'George Hallett')
        self.assertEqual(p['license'],'CC BY-SA 3.0')
        self.assertTrue(p['license_url'].startswith('https://creativecommons.org/'))
        self.assertEqual(hashlib.sha256((CONTENT/'Portraits'/p['file']).read_bytes()).hexdigest(),p['sha256'])
        n=self.nodes[p['person_id']]
        self.assertEqual(n['portrait'],p['file'])
        self.assertIn('1997',self.entry['text'][n['portrait_caption']]['sv'])

    def test_invalid_graph_and_unsafe_portrait_are_rejected(self):
        for value in ['../other.jpg','x\\other.jpg','portrait.svg']:
            e=copy.deepcopy(self.entry)
            next(n for n in e['hierarchy']['nodes'] if n['id']=='eugene-de-kock')['portrait']=value
            with self.assertRaises(ValueError):validate_entry(e,set())
        e=copy.deepcopy(self.entry)
        next(n for n in e['hierarchy']['nodes'] if n['id']=='nis')['parent']='not-a-node'
        with self.assertRaises(ValueError):validate_entry(e,set())

if __name__=='__main__':unittest.main()
