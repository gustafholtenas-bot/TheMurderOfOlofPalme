"""Data and packaging contracts, runnable without Unreal: python -m unittest discover -s Tools/WorldAtlas."""
import datetime
import json
import math
import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
CONTENT = ROOT / 'Plugins/TMOPEngine/Content/WorldAtlas'
PRIVATE = ROOT / 'Plugins/TMOPEngine/Source/TMOPEngine/Private'

class AtlasDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.world = json.loads((CONTENT / 'world.json').read_text(encoding='utf-8'))
        cls.entries = cls.world['entries']
        cls.by_id = {e['id']: e for e in cls.entries}

    def test_ids_kinds_and_coordinates(self):
        self.assertEqual(self.world['schema'], 1)
        self.assertEqual(self.world['reference_date'], '1986-02-28')
        self.assertEqual(len(self.entries), len(self.by_id))
        for e in self.entries:
            self.assertRegex(e['id'], r'^[a-z][a-z0-9-]*$')
            self.assertIn(e['kind'], {'country', 'actor', 'group', 'conflict', 'arms', 'funds', 'event'})
            self.assertTrue(-90 <= e['lat'] <= 90)
            self.assertTrue(-180 <= e['lon'] <= 180)
            for date in ('from', 'to'):
                if e[date]: datetime.date.fromisoformat(e[date])
            if e['from'] and e['to']: self.assertLessEqual(e['from'], e['to'])

    def test_references_and_route_endpoints(self):
        for e in self.entries:
            for key in ('related', 'route'):
                for target in e[key]: self.assertIn(target, self.by_id, (e['id'], target))
            if e['route']:
                self.assertIn(e['kind'], {'arms', 'funds'})
                self.assertGreaterEqual(len(e['route']), 2)
                self.assertTrue(e['sources'])
                for target in e['route']: self.assertIn(self.by_id[target]['kind'], {'country', 'actor'})
            if e['kind'] == 'group': self.assertFalse(e['marker'], 'Networks have no invented geographic HQ')

    def test_translations_are_complete_and_current(self):
        for e in self.entries:
            self.assertIn('title', e['text'])
            for key, value in e['text'].items():
                self.assertTrue(value['sv'], (e['id'], key))
                self.assertTrue(value['en'], (e['id'], key))
                self.assertEqual(value['sv'], value['en_source'], (e['id'], key, 'stale translation'))
        code = (PRIVATE / 'WorldAtlas/STMOPWorldAtlas.cpp').read_text(encoding='utf-8')
        builtins = (PRIVATE / 'Localization/TMOPMenuTranslations.inl').read_text(encoding='utf-8')
        keys = dict(re.findall(r'NSLOCTEXT\("TMOP",\s*"(Atlas\w+)",\s*"([^"]*)"\)', code))
        for key, source in keys.items():
            self.assertIn(f'Add(TEXT("{key}"), TEXT("{source}"), TEXT("', builtins)

    def test_historical_boundaries_and_labels(self):
        nato = self.by_id['nato']['related']
        self.assertEqual(len(nato), 16)
        self.assertNotIn('se', nato)
        self.assertEqual(self.by_id['de']['text']['title']['en'], 'West Germany')
        self.assertGreater(self.by_id['bilderberg-1986']['from'], '1986-02-28')
        self.assertTrue(self.by_id['iran-money']['later_only'])
        self.assertLess(self.by_id['contra-portugal']['to'], '1986-02-28')
        self.assertGreaterEqual(sum(e['kind'] == 'conflict' for e in self.entries), 53)
        self.assertEqual(self.by_id['iraq-supply']['route'], [])
        self.assertEqual(self.by_id['sweden-arms']['route'], [])

    def test_sources_are_explicit_and_not_scripts(self):
        for e in self.entries:
            if e['kind'] in {'country', 'actor', 'group', 'conflict', 'event'}: self.assertTrue(e['sources'])
            for s in e['sources']:
                self.assertTrue(s['url'].startswith('https://'))
                self.assertTrue(s['title'])
                self.assertIn('published', s)

    def test_country_profile_coverage_and_source_scopes(self):
        countries = [e for e in self.entries if e['kind'] == 'country']
        self.assertEqual(len(countries), 33)
        for e in countries:
            self.assertEqual(e['profile_as_of'], self.world['reference_date'])
            for field in ('government', 'leaders', 'ministers', 'intelligence', 'personnel', 'research'):
                self.assertIn(field, e['text'], (e['id'], field))
            self.assertIn(e['intelligence_coverage'], {'named_selection', 'date_verification_pending'})
            covered = {f for s in e['sources'] for f in s.get('fields', [])}
            self.assertTrue({'government', 'ministers', 'personnel'} <= covered, e['id'])
        for e in self.entries:
            for source in e['sources']:
                for field in source.get('fields', []):
                    self.assertIn(field, e['text'], (e['id'], field))
        # These records must retain their explicit uncertainty until new evidence is supplied.
        for id in ('eg', 'hn', 'sv', 'ao', 'gt', 'is'):
            self.assertEqual(self.by_id[id]['intelligence_coverage'], 'date_verification_pending')

    def test_nicaragua_actor_separation_and_beneficiary(self):
        country = self.by_id['ni']
        actors = [self.by_id[id] for id in ('ni-fsln', 'ni-contras')]
        for e in actors:
            self.assertEqual(e['kind'], 'actor')
            self.assertEqual(e['country'], 'ni')
            self.assertTrue(e['marker'])
            self.assertEqual((e['lat'], e['lon']), (country['lat'], country['lon']))
            self.assertIn(e['id'], country['related'])
            self.assertIn('map_note', e['text'])
            self.assertIn('conflict-nicaragua', e['related'])
            self.assertEqual(len(e['marker_offset']), 2)
            self.assertTrue(all(math.isfinite(v) and abs(v) <= 80 for v in e['marker_offset']))
        offsets = [(0, 0)] + [e['marker_offset'] for e in actors]
        for i, a in enumerate(offsets):
            for b in offsets[i+1:]:
                self.assertGreater(math.dist(a, b), 28, '14-pixel hit circles must not overlap')
        self.assertIn('ni-contras', actors[0]['related'])
        self.assertIn('ni-fsln', actors[1]['related'])
        self.assertEqual(self.by_id['iran-money']['route'][-1], 'ni-contras')
        self.assertEqual(self.by_id['contra-portugal']['route'][-1], 'hn')

    def test_reference_date_officeholders(self):
        expectations = {
            'se': ('ministers', 'Roine Carlsson'),
            'gb': ('ministers', 'George Younger'),
            'fr': ('ministers', 'Michel Crépeau'),
            'is': ('ministers', 'Matthías Árni Mathiesen'),
            'lu': ('ministers', 'Marc Fischbach'),
            'nl': ('personnel', 'Aart Blom'),
            'lb': ('personnel', 'Jamil Nehme'),
            'dk': ('personnel', 'Henning Fode'),
            'no': ('personnel', 'Jostein Erstad'),
        }
        for id, (field, name) in expectations.items():
            self.assertIn(name, self.by_id[id]['text'][field]['en'])
        self.assertIn('28 February 1986', self.by_id['eg']['text']['ministers']['en'])
        self.assertIn('John N. McMahon', self.by_id['us']['text']['personnel']['en'])
        self.assertIn('Babrak Karmal', self.by_id['af']['text']['government']['en'])
        self.assertIn('Samora Machel', self.by_id['mz']['text']['leaders']['en'])

    def test_conflict_graphs_and_date_precision(self):
        for e in self.entries:
            if e['kind'] != 'conflict': continue
            self.assertTrue(e['from'] and e['to'])
            self.assertIn(e['date_precision'], {'year', 'month', 'day'})
            nodes = {p['id']: p for p in e['participants']}
            self.assertGreaterEqual(len(nodes), 2)
            self.assertEqual(len(nodes), len(e['participants']))
            used = set()
            self.assertTrue(e['links'])
            for link in e['links']:
                self.assertIn(link['from'], nodes)
                self.assertIn(link['to'], nodes)
                self.assertNotEqual(link['from'], link['to'])
                self.assertIn(link['kind'], {'opposition', 'support', 'violence'})
                key = (link['from'], link['to'], link['kind'])
                self.assertNotIn(key, used)
                used.add(key)
            for node in nodes.values():
                self.assertIn(node['label'], e['text'])
                self.assertTrue(-90 <= node['lat'] <= 90 and -180 <= node['lon'] <= 180)
                self.assertEqual(len(node['offset']), 2)
                self.assertTrue(all(math.isfinite(v) and abs(v) <= 80 for v in node['offset']))
            if 'parent_conflict' in e:
                parent = self.by_id[e['parent_conflict']]
                self.assertEqual(parent['kind'], 'conflict')
                self.assertIn(parent['id'], e['related'])
                self.assertIn(e['id'], parent['related'])

    def test_conflict_reference_snapshot_and_nearby(self):
        date = datetime.date(1986, 2, 28)
        begin, end = date - datetime.timedelta(days=90), date + datetime.timedelta(days=90)
        self.assertEqual(str(begin), '1985-11-30')
        self.assertEqual(str(end), '1986-05-29')
        conflicts = [e for e in self.entries if e['kind'] == 'conflict']
        active = {e['id'] for e in conflicts if e['from'] <= str(date) <= e['to']}
        for name in ('egypt-mutiny','palestine','turkey-pkk','iraq-kurds','iran-kurds','nicaragua','dawn-nine','al-faw'):
            self.assertIn('conflict-'+name, active)
        nearby = {'conflict-'+n for n in ('south-yemen','agacher','lesotho-coup','lf-coup','people-power','kampala','sidra','libya-airstrike')}
        self.assertEqual({e['id'] for e in conflicts} - active, nearby)
        for id in nearby:
            e = self.by_id[id]
            self.assertTrue(e['from'] <= str(end) and e['to'] >= str(begin))

    def test_civilians_are_not_given_combat_arrows(self):
        e = self.by_id['conflict-gukurahundi']
        self.assertEqual([link['kind'] for link in e['links']], ['violence'])
        self.assertIn('Civilians', e['text']['participant-p1']['en'])
        for id in ('conflict-turkey-pkk', 'conflict-iraq-kurds', 'conflict-iran-kurds'):
            labels = ' '.join(self.by_id[id]['text'][p['label']]['en'] for p in self.by_id[id]['participants'])
            for future_actor in ('YPG', 'PJAK', 'SDF'): self.assertNotIn(future_actor, labels)
        labels = ' '.join(self.by_id['conflict-palestine']['text'][p['label']]['en'] for p in self.by_id['conflict-palestine']['participants'])
        self.assertNotIn('Hamas', labels)

    def test_coastline_geometry_and_staging(self):
        coast = json.loads((CONTENT / 'coastlines.json').read_text(encoding='utf-8'))
        self.assertGreater(len(coast['lines']), 50)
        for line in coast['lines']:
            self.assertGreaterEqual(len(line), 2)
            for lon, lat in line:
                self.assertTrue(-180.001 <= lon <= 180.001)
                self.assertTrue(-90 <= lat <= 90)
        build = (ROOT / 'Plugins/TMOPEngine/Source/TMOPEngine/TMOPEngine.Build.cs').read_text()
        for file in ('world.json', 'coastlines.json'):
            self.assertIn(f'RuntimeDependencies.Add("$(PluginDir)/Content/WorldAtlas/{file}", StagedFileType.UFS)', build)

if __name__ == '__main__': unittest.main()
