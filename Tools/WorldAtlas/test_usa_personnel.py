"""Validate dated affiliations and separation of current and historical records."""
import hashlib,json,unittest
from validate_hierarchy import CONTENT,validate_entry
class USAPersonnelTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.entry=next(e for e in json.loads((CONTENT/'world.json').read_text())['entries'] if e['id']=='us');rd=CONTENT.parents[3]/'Tools/WorldAtlas/Research';cls.r=json.loads((rd/'usa_personnel_1986.json').read_text());cls.images=json.loads((rd/'usa_portraits_1986.json').read_text());cls.people={p['id']:p for p in cls.r['people']};cls.nodes={n['id']:n for n in cls.entry['hierarchy']['nodes']}
 def test_dated_sourced_cards(self):
  self.assertEqual(len(self.people),25)
  for p in self.people.values():
   n=self.nodes[p['node_id']];self.assertIn(p['reference_date'],['1986-02-28','1986']);self.assertEqual(p['inclusion_period'],'1985–1986');self.assertTrue(p['evidence_status'] and p['sources'])
   if not p['existing_office_card']:self.assertEqual(n['parent'],p['organization'])
   for k in ['label','note']:
    t=self.entry['text'][n[k]];self.assertTrue(t['sv'] and t['en']);self.assertEqual(t['en_source'],t['sv'])
 def test_staff_and_external_networks_separate(self):
  for id in ['oliver-north','john-poindexter','vincent-cannistraro']:self.assertEqual(self.nodes[id]['parent'],'nsc-staff')
  for id in ['richard-secord','albert-hakim','thomas-clines','richard-gadd','rafael-quintero']:self.assertEqual(self.nodes[id]['parent'],'enterprise')
  self.assertEqual(self.nodes['covert-networks']['parent'],'');self.assertEqual(self.nodes['enterprise']['parent'],'covert-networks');self.assertEqual(self.nodes['duane-clarridge']['parent'],'cia-ctc');self.assertEqual(self.nodes['joseph-fernandez']['parent'],'cia-costa-rica');self.assertEqual(self.nodes['donald-gregg']['parent'],'vp-security-staff');self.assertEqual(self.nodes['john-singlaub']['parent'],'external-contra-support');self.assertIn('Administration',self.people['richard-kerr']['role']['en'])
 def test_later_roles_and_historical_programs_not_backdated(self):
  self.assertIn('Intelligence',self.people['robert-gates']['role']['en']);self.assertNotIn('Central',self.people['robert-gates']['role']['en']);self.assertIn('McMahon',self.entry['text'][self.nodes['cia-deputy']['label']]['en']);self.assertIn('Senior',self.people['thomas-twetten']['role']['en']);self.assertIn('NSC',self.people['vincent-cannistraro']['role']['en'])
  self.assertEqual(len(self.r['historical_context']),2)
  for h in self.r['historical_context']:self.assertNotIn(h['id'],self.nodes);self.assertEqual(h['status'],'historical_context_only_not_1986_staff')
  self.assertNotIn('william-harvey',self.nodes);self.assertEqual(self.r['year_only_people'],[]);self.assertIn('john-singlaub',self.nodes);self.assertEqual(self.r['reference_date'],'1985–1986');self.assertEqual(len(self.r['changes_from_US01']['added_people']),8)
 def test_existing_office_cards_not_duplicated(self):
  self.assertEqual(len(self.r['changes_from_SA02']['added_people']),12);self.assertEqual(len(self.r['changes_from_SA02']['existing_office_cards_enriched']),5)
  for p in self.people.values():
   if p['existing_office_card']:self.assertNotIn(p['id'],self.nodes)
  self.assertNotIn('richard-gadd',set(p['person_id'] for p in self.images))
 def test_portrait_files_and_dates(self):
  self.assertEqual(len(self.images),7)
  for p in self.images:
   self.assertEqual(hashlib.sha256((CONTENT/'Portraits'/p['file']).read_bytes()).hexdigest(),p['sha256']);self.assertTrue(p['license_url'].startswith('https://'));self.assertTrue(p['date_note']['sv']);n=self.nodes[self.people[p['person_id']]['node_id']];self.assertEqual(n['portrait'],p['file']);self.assertIn(p['date_note']['sv'],self.entry['text'][n['portrait_caption']]['sv'])
  self.assertEqual(self.people['robert-gates']['portrait']['source_date'],'2006-12-12');self.assertEqual(self.people['john-poindexter']['portrait']['source_date'],'1985-11-04')
if __name__=='__main__':unittest.main()
