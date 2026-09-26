import copy
import hashlib
import json
from pathlib import Path
import unittest

from validate_hierarchy import CONTENT, validate_entry


class HierarchyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.world = json.loads((CONTENT / 'world.json').read_text())
        cls.by_id = {e['id']: e for e in cls.world['entries']}
        cls.flags = {f['id'] for f in json.loads((CONTENT / 'flags_1986.json').read_text())['flags']}

    def test_all_countries_have_valid_localized_trees(self):
        for entry in self.world['entries']:
            if entry['kind'] == 'country':
                self.assertIn('hierarchy', entry, entry['id'])
            validate_entry(entry, self.flags)

    def test_china_vietnam_and_non_state_participants(self):
        self.assertEqual([p['flag'] for p in self.by_id['conflict-china-vietnam']['participants']], ['cn', 'vn'])
        for id in ['conflict-turkey-pkk', 'conflict-nicaragua', 'conflict-gukurahundi']:
            self.assertNotIn('flag', self.by_id[id]['participants'][1])
        # This combined Vietnamese/Kampuchean participant cannot truthfully be
        # represented by either state's flag alone.
        self.assertNotIn('flag', self.by_id['conflict-cambodia']['participants'][0])

    def test_sourced_us_branches_and_constitutional_distinctions(self):
        nodes = {n['id']: n for n in self.by_id['us']['hierarchy']['nodes']}
        self.assertEqual(nodes['fbi']['parent'], 'minister-3')
        self.assertEqual(nodes['nsa']['parent'], 'minister-1')
        self.assertEqual(nodes['cia-catf']['parent'], 'cia-la')
        self.assertEqual(nodes['cia-la']['parent'], 'cia-operations')
        self.assertEqual(nodes['cia-operations']['parent'], 'cia-deputy')
        self.assertEqual(nodes['cia-analysis']['parent'], 'cia-deputy')
        self.assertNotEqual(nodes['cia']['parent'], 'leader-1')  # vice-president
        self.assertIn('John N. McMahon', self.by_id['us']['text'][nodes['cia-deputy']['label']]['en'])
        self.assertIn('Director for Intelligence', self.by_id['us']['text'][nodes['cia-analysis']['label']]['en'])
        swiss = {n['id']: n for n in self.by_id['ch']['hierarchy']['nodes']}
        self.assertEqual(swiss['cabinet']['parent'], 'council')
        self.assertEqual(swiss['leader-0']['parent'], 'council')
        self.assertEqual(swiss['leader-0']['relation'], 'group')

    def test_invalid_graphs_and_sources_are_rejected(self):
        for mutation in ('cycle', 'orphan', 'duplicate', 'no-source', 'no-label', 'wrong-date'):
            with self.subTest(mutation=mutation):
                e = copy.deepcopy(self.by_id['us']); ns = e['hierarchy']['nodes']
                cia = next(n for n in ns if n['id'] == 'cia')
                if mutation == 'cycle': cia['parent'] = 'cia-catf'
                if mutation == 'orphan': cia['parent'] = 'no-such-office'
                if mutation == 'duplicate': ns.append(copy.deepcopy(cia))
                if mutation == 'no-source': cia['sources'] = []
                if mutation == 'no-label': cia['label'] = 'no-such-text'
                if mutation == 'wrong-date': e['hierarchy']['as_of'] = '2003-06-13'
                with self.assertRaises(ValueError): validate_entry(e, self.flags)

    def test_unshipped_flag_and_unbounded_depth_are_rejected(self):
        e = copy.deepcopy(self.by_id['conflict-china-vietnam'])
        e['participants'][0]['flag'] = 'xx'
        with self.assertRaises(ValueError): validate_entry(e, self.flags)
        e = copy.deepcopy(self.by_id['us']); ns = e['hierarchy']['nodes']; base = copy.deepcopy(ns[0])
        for i in range(33):
            n = copy.deepcopy(base); n.update(id=f'level-{i}', parent=f'level-{i-1}' if i else '', relation='group'); ns.append(n)
        with self.assertRaises(ValueError): validate_entry(e, self.flags)

    def test_flag_source_bytes_match_manifest(self):
        source = Path(__file__).parent / 'SourceAssets/flags'
        manifest = json.loads((source / 'sources.json').read_text())
        self.assertEqual(set(manifest), self.flags)
        for id, meta in manifest.items():
            self.assertEqual(hashlib.sha256((source / (id+'.svg')).read_bytes()).hexdigest(), meta['sha256'], id)


if __name__ == '__main__': unittest.main()
