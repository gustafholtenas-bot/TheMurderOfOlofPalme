"""Check period inclusion, identity separation and explicit research gaps."""
import json,hashlib,unittest
from validate_hierarchy import CONTENT
class NATOPersonnelTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.world={e['id']:e for e in json.loads((CONTENT/'world.json').read_text())['entries']};cls.rd=CONTENT.parents[3]/'Tools/WorldAtlas/Research';cls.r=json.loads((cls.rd/'nato_personnel_1985_1986.json').read_text());cls.p={(p['country'],p['id']):p for p in cls.r['people']}
 def test_historical_members_and_research_gap(self):
  self.assertEqual(set(c['id'] for c in self.r['countries']),{'be','ca','dk','fr','de','gr','is','it','lu','nl','no','pt','es','tr','gb','us'});self.assertEqual(len(self.r['countries']),16);self.assertEqual(next(c for c in self.r['countries'] if c['id']=='is')['person_records'],0)
 def test_all_person_records_have_runtime_card_and_sources(self):
  self.assertEqual(len(self.p),411)
  for p in self.p.values():
   e=self.world[p['country']];nodes={n['id']:n for n in e['hierarchy']['nodes']};n=nodes[p['node_id']];self.assertTrue(p['sources']);self.assertTrue(all(s.startswith('https://') for s in p['sources']));self.assertEqual(p['inclusion_period'],'1985–1986')
   if not p['existing_office_card']:self.assertEqual(n['parent'],p['organization'])
   self.assertTrue(e['text'][n['label']]['sv']);self.assertTrue(e['text'][n['note']]['en'])
 def test_successive_roles_and_partial_periods(self):
  self.assertEqual(len(self.r['changes_from_US02']['added_people']),20);self.assertIn('1985-03',self.p['gb','john-jones']['service_period']);self.assertIn('1986-09-05',self.p['tr','hayri-undul']['service_period']);self.assertEqual(self.p['us','charles-lord']['organization'],'nsa-deputies');self.assertEqual(self.p['nl','aart-blom']['service_period'],'1986-02-01–1989-02-01')
 def test_network_membership_not_assigned_to_every_chief(self):
  self.assertEqual(self.p['lu','charles-hoffmann']['organization'],'sre');self.assertEqual(self.p['be','albert-raes']['organization'],'security');self.assertEqual(self.p['it','paolo-inzerilli']['organization'],'gladio');self.assertEqual(self.p['es','jose-amedo']['organization'],'gal');self.assertEqual(self.p['es','juan-alberto-perote']['organization'],'cesid-aome');self.assertEqual(len([n for n in self.r['network_nodes'] if n['category']=='stay_behind']),7)
 def test_scope_does_not_make_appointments_simultaneous(self):
  for c in self.r['countries']:
   h=self.world[c['id']]['hierarchy'];self.assertEqual(h['as_of'],'1986-02-28');self.assertEqual(h['personnel_period']['start'],'1985-01-01');self.assertIn('not simultaneous',h['personnel_period']['semantics'])
  self.assertEqual(len([p for p in self.p.values() if p['evidence_status']=='inherited_office_record_further_period_recheck_pending']),3)
 def test_nato02_roles_and_identity_notes(self):
  self.assertEqual(len(self.r['changes_from_NATO01']['added_people']),10)
  self.assertEqual(self.p['gb','stella-rimington']['service_period'],'Från 1986')
  self.assertEqual(self.p['gb','stella-rimington']['portrait']['source_date'],'2016-10-11')
  self.assertEqual(self.p['fr','christine-cabon']['service_period'],'April–maj 1985')
  self.assertIn('Barcelo',self.p['fr','jean-michel-bartelo']['note']['sv'])
  self.assertIn('Ingen individuell fällande dom',self.p['fr','jean-luc-kister']['note']['sv'])
 def test_nato03_dates_and_agency_separation(self):
  self.assertEqual(len(self.r['changes_from_NATO02']['added_people']),11)
  self.assertEqual(self.p['nl','aj-romijn']['service_period'],'1980-04-01–1986-09-26')
  self.assertIn('1986-09-26',self.p['nl','km-meulmeester']['service_period'])
  self.assertEqual(self.p['tr','hasan-kundakci']['organization'],'ozel-harp-dairesi')
  self.assertEqual(self.p['tr','hiram-abas']['organization'],'mit')
  self.assertEqual(self.p['de','ludwig-holger-pfahls']['evidence_status'],'parliamentary_appointment_date_verified')
  self.assertEqual(len(self.r['period_verification_pending_ids']),3)
 def test_nato04_periods_and_court_distinction(self):
  self.assertEqual(len(self.r['changes_from_NATO03']['added_people']),11)
  self.assertEqual(self.p['ca','ray-kobzey']['organization'],'csis-bc')
  self.assertEqual(self.p['ca','blair-seaborn']['organization'],'pco-intelligence')
  self.assertEqual(self.p['es','antonio-rosino-blanco']['service_period'],'1984-03-01–1985-10')
  self.assertEqual(self.p['es','julio-hierro-moset']['service_period'],'1985-10–1986-07-12')
  self.assertIn('Frikänd 2011',self.p['es','miguel-planchuelo']['note']['sv'])
  self.assertNotEqual(self.p['es','miguel-planchuelo']['organization'],'gal')
 def test_nato05_dated_roles_and_career_inferences(self):
  self.assertEqual(len(self.r['changes_from_NATO04']['added_people']),8)
  self.assertEqual(self.p['it','federigo-mannucci-benincasa']['service_period'],'1971-01-29–1991-02-28')
  self.assertEqual(self.p['gb','jonathan-evans']['service_period'],'Från 1985')
  for id in ['eliza-manningham-buller','andrew-parker','stephen-lander']:
   self.assertEqual(self.p['gb',id]['evidence_status'],'official_career_span_period_overlap_inferred')
   self.assertIn('öppen',self.p['gb',id]['role']['sv'])
  self.assertIn('independent_recheck_pending',self.p['it','bruno-contrada']['evidence_status'])
  self.assertEqual(self.p['gb','eliza-manningham-buller']['portrait']['source_date'],'2025-07-23')
 def test_nato06_service_and_oversight_are_distinct(self):
  self.assertEqual(len(self.r['changes_from_NATO05']['added_people']),11)
  self.assertEqual(self.p['dk','christian-rene-dehn']['service_period'],'1983-10-01–1986-12-01 (PET)')
  self.assertEqual(self.p['dk','frank-jan-jensen']['organization'],'pet-department-2')
  self.assertEqual(self.p['dk','niels-madsen']['organization'],'justice-pet-oversight')
  self.assertEqual(self.p['pt','pedro-alexandre-gomes-cardoso']['organization'],'sirp-technical-commission')
  self.assertEqual(self.p['gr','nikolaos-gryllakis']['organization'],'nd-security-information')
  self.assertNotEqual(self.p['gr','nikolaos-gryllakis']['organization'],'kyp')
 def test_nato07_roles_dates_and_inference(self):
  self.assertEqual(len(self.r['changes_from_NATO06']['added_people']),10)
  self.assertEqual(self.p['fr','bernard-gerard']['service_period'],'Utnämnd 1986-04-09')
  self.assertEqual(self.p['de','paul-munstermann']['service_period'],'1986–1994')
  self.assertEqual(self.p['fr','louis-caprioli']['service_period'],'1983–1994')
  self.assertEqual(self.p['fr','gilles-menage']['organization'],'elysee-security-coordination')
  self.assertEqual(self.p['fr','christian-prouteau']['organization'],'elysee-antiterror')
  self.assertIn('overlap_inferred',self.p['fr','alain-chouet']['evidence_status'])
  self.assertIn('original_unavailable',self.p['be','jacques-de-vlieghere']['evidence_status'])
 def test_nato08_assessment_roles_are_distinct(self):
  self.assertEqual(len(self.r['changes_from_NATO07']['added_people']),8)
  self.assertEqual(self.p['gb','percy-cradock']['organization'],'joint-intelligence-committee')
  self.assertEqual(self.p['us','douglas-maceachin']['organization'],'cia-sova')
  self.assertEqual(self.p['us','graham-fuller']['organization'],'national-intelligence-council')
  self.assertEqual(self.p['us','graham-fuller']['service_period'],'Belagt 1985-05-07')
  self.assertIn('Från 1981',self.p['us','lawrence-gershwin']['service_period'])
 def test_nato09_analysts_and_period_titles(self):
  self.assertEqual(len(self.r['changes_from_NATO08']['added_people']),8)
  self.assertEqual(self.p['us','beth-seeger']['organization'],'cia-global-issues')
  self.assertEqual(self.p['us','mary-desjeans']['organization'],'cia-sova')
  self.assertEqual(self.p['us','david-cohen']['service_period'],'1981–1985')
  self.assertEqual(self.p['us','john-hibbits']['service_period'],'Belagt 1985-05')
  self.assertIn('öppen',self.p['us','wayne-limberg']['role']['sv'])
 def test_nato10_oversight_and_dated_operational_roles(self):
  self.assertEqual(len(self.r['changes_from_NATO09']['added_people']),8)
  self.assertEqual(self.p['fr','jean-claude-lesquer']['organization'],'dgse-service-action')
  for id in ['mario-julio-montalvao-machado','antonio-alves-marques-junior','jose-anselmo-dias-rodrigues']:
   self.assertEqual(self.p['pt',id]['organization'],'sirp-oversight-council')
  self.assertEqual(self.p['us','grey-hodnett']['service_period'],'Belagt 1986-04-29')
  self.assertEqual(self.p['us','ross-cowey']['organization'],'cia-assessment-review-1985')
 def test_nato11_contract_and_analytical_offices(self):
  self.assertEqual(len(self.r['changes_from_NATO10']['added_people']),5)
  self.assertEqual(self.p['us','thomas-barksdale']['organization'],'cia-nesa-analysis')
  self.assertEqual(self.p['us','george-cave']['service_period'],'Från 1986-03; uppdrag under 1986')
  self.assertEqual(self.p['gb','derek-boorman']['service_period'],'Belagt i slutet av 1986')
 def test_nato12_analytical_staff_and_acting_roles(self):
  self.assertEqual(len(self.r['changes_from_NATO11']['added_people']),15)
  self.assertEqual(self.p['us','frank-mcneil']['organization'],'state-inr')
  self.assertEqual(self.p['us','douglas-george']['organization'],'cia-acis')
  self.assertEqual(self.p['us','martha-mautner']['evidence_status'],'contemporary_official_staff_magazine')
 def test_nato13_geographical_roles_and_unspecified_titles(self):
  self.assertEqual(len(self.r['changes_from_NATO12']['added_people']),5)
  self.assertEqual(self.p['us','george-kolt']['service_period'],'1984–1986')
  self.assertEqual(self.p['us','robert-w-smith']['organization'],'state-inr-geographer')
  self.assertEqual(self.p['us','tim-hudson']['evidence_status'],'contemporary_official_staff_notice_no_formal_title')
 def test_nato14_career_inference_and_conflicting_station_dates(self):
  self.assertEqual(len(self.r['changes_from_NATO13']['added_people']),3)
  self.assertFalse(self.r['changes_from_NATO13']['added_groups'])
  for id in ['richard-dearlove','john-scarlett']:
   self.assertEqual(self.p['gb',id]['evidence_status'],'affiliated_biography_career_span_period_overlap_inferred')
   self.assertIn('chefstitlar',self.p['gb',id]['note']['sv'])
  self.assertIn('skiljer',self.p['gb','meta-ramsay']['service_period'])
 def test_nato15_distinct_florence_services_and_later_assignments(self):
  self.assertEqual(len(self.r['changes_from_NATO14']['added_people']),4)
  self.assertEqual(len(self.r['changes_from_NATO14']['added_groups']),2)
  self.assertEqual(self.p['it','paolo-fornaro']['service_period'],'1980-11-12–1986-05-31')
  self.assertIn('1987',self.p['it','paolo-fornaro']['note']['sv'])
  self.assertEqual(self.p['it','vito-sebastiano-luongo']['organization'],'sisde-firenze')
  self.assertEqual(self.p['it','federigo-mannucci-benincasa']['organization'],'sismi-firenze')
  self.assertTrue(self.p['it','marco-mancini']['evidence_status'].endswith('_inferred'))
 def test_nato16_dated_succession_and_state_affiliation(self):
  self.assertEqual(len(self.r['changes_from_NATO15']['added_people']),7)
  self.assertEqual(len(self.r['changes_from_NATO15']['added_groups']),3)
  self.assertEqual(self.p['ca','james-warren']['service_period'],'Från mars 1986')
  self.assertIn('augusti',self.p['ca','chris-scowen']['service_period'])
  self.assertEqual(self.p['de','peter-frisch']['organization'],'verfassungsschutz-niedersachsen')
  self.assertEqual(self.p['de','peter-frisch']['service_period'],'1984–1987')
  self.assertEqual(self.p['it','luciano-piacentini']['organization'],'col-moschin')
  self.assertEqual(self.p['it','simone-baschiera']['organization'],'col-moschin')
 def test_portrait_attribution_and_integrity(self):
  photos=json.loads((self.rd/'nato_portraits_1985_1986.json').read_text());self.assertEqual(len(photos),11)
  for im in photos:
   self.assertEqual(hashlib.sha256((CONTENT/'Portraits'/im['file']).read_bytes()).hexdigest(),im['sha256']);self.assertTrue(im['author'] and im['license_url'] and im['date_note']['sv']);p=self.p[im['country'],im['person_id']];n=next(n for n in self.world[p['country']]['hierarchy']['nodes'] if n['id']==p['node_id']);self.assertEqual(n['portrait'],im['file'])
if __name__=='__main__':unittest.main()
