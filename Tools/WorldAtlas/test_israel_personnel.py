"""Data and asset checks for the Israel personnel expansion."""
import copy
import hashlib
import json
from pathlib import Path
import unittest
from validate_hierarchy import CONTENT, validate_entry

class IsraelPersonnelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entry = next(e for e in json.loads((CONTENT/'world.json').read_text())['entries'] if e['id']=='il')
        cls.research = Path(__file__).parent/'Research'
        cls.people = json.loads((cls.research/'israel_personnel_1986.json').read_text())['people']
        cls.portraits = json.loads((cls.research/'israel_portraits_1986.json').read_text())

    def test_research_people_have_sourced_localized_cards(self):
        nodes={n['id']:n for n in self.entry['hierarchy']['nodes']}
        self.assertEqual(len(self.people),55)
        for person in self.people:
            n=nodes[person['id']]
            self.assertEqual(n['parent'],person['organization'])
            self.assertTrue(n['sources'])
            self.assertEqual(n['relation'],'group')
            for key in ['label','note']:
                text=self.entry['text'][n[key]]
                self.assertTrue(text['sv'] and text['en'])
                self.assertEqual(text['sv'],text['en_source'])

    def test_later_titles_are_not_backdated(self):
        people={p['id']:p for p in self.people}
        for id in ['shabtai-shavit','carmi-gillon','avi-dichter','nadav-argaman','yossi-cohen']:
            self.assertNotIn('director',people[id]['role']['en'].lower())
        self.assertEqual(people['omer-bar-lev']['organization'],'sayeret-matkal')
        self.assertEqual(people['alik-ron']['organization'],'shaldag')
        self.assertEqual(people['ronen-bar']['organization'],'sayeret-matkal')
        self.assertEqual(people['david-tzur']['organization'],'yamam')

    def test_portrait_assets_have_licenses_and_matching_hashes(self):
        self.assertEqual(len(self.portraits),37)
        for p in self.portraits:
            self.assertTrue(p['author'] and p['source_url'])
            self.assertTrue(p['license_url'].startswith('https://creativecommons.org/'))
            self.assertEqual(hashlib.sha256((CONTENT/'Portraits'/p['file']).read_bytes()).hexdigest(),p['sha256'])
        for n in self.entry['hierarchy']['nodes']:
            if 'portrait' in n:
                self.assertTrue((CONTENT/'Portraits'/n['portrait']).is_file())
                self.assertIn(n['portrait_caption'],self.entry['text'])

    def test_annual_and_review_people_stay_out_of_february(self):
        data=json.loads((self.research/'israel_personnel_1986.json').read_text())
        ids={n['id'] for n in self.entry['hierarchy']['nodes']}
        for p in data['year_only_people']+data['review_people']:
            self.assertNotIn(p['id'],ids)
        nodes={n['id']:n for n in self.entry['hierarchy']['nodes']}
        self.assertEqual(nodes['yamam']['parent'],'police-special-forces')
        self.assertEqual(nodes['aman-special-operations']['parent'],'aman')
        self.assertEqual(nodes['lebanon-coordination']['parent'],'services')
        self.assertEqual(nodes['nativ']['parent'],'services')
        self.assertEqual(nodes['naval-intelligence']['parent'],'military')
        self.assertEqual(nodes['tal-russo']['parent'],'military')
        self.assertEqual(nodes['yaakov-kedmi']['parent'],'nativ')
        self.assertEqual(nodes['reuven-erlich']['parent'],'lebanon-coordination')
        self.assertEqual(next(p for p in self.portraits if p['person_id']=='dror-weinberg')['source_date'],'')

    def test_portrait_path_traversal_and_missing_credit_rejected(self):
        for value in ['../outside.jpg','/outside.jpg','x\\outside.jpg','C:outside.jpg','',5,'portrait.svg']:
            e=copy.deepcopy(self.entry)
            next(n for n in e['hierarchy']['nodes'] if 'portrait' in n)['portrait']=value
            with self.subTest(value=value),self.assertRaises(ValueError):validate_entry(e,set())
        e=copy.deepcopy(self.entry)
        del next(n for n in e['hierarchy']['nodes'] if 'portrait' in n)['portrait_caption']
        with self.assertRaises(ValueError):validate_entry(e,set())

if __name__=='__main__':unittest.main()
