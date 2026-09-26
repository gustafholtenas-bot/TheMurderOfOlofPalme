"""Offline contracts for the shipped 1986 political globe (stdlib only)."""
import json
import math
import pathlib
import struct
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
CONTENT = ROOT / 'Plugins/TMOPEngine/Content/WorldAtlas'


class AtlasAppearanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.land = json.loads((CONTENT / 'land_1986.json').read_text())
        cls.alignment = json.loads((CONTENT / 'alignments_1986.json').read_text())
        cls.flags = json.loads((CONTENT / 'flags_1986.json').read_text())
        cls.world = json.loads((CONTENT / 'world.json').read_text())
        cls.by_id = {x['id']: x for x in cls.land['countries']}
        cls.blocs = {x['id']: x for x in cls.alignment['countries']}

    def test_snapshot_and_historical_successions(self):
        for data in (self.land, self.alignment, self.flags):
            self.assertEqual(data['schema'], 1)
            self.assertEqual(data['reference_date'], '1986-02-28')
        self.assertEqual(len(self.by_id), len(self.land['countries']))
        self.assertEqual(set(self.by_id), set(self.blocs))
        self.assertTrue({'su', 'yu', 'cs', 'dd', 'de', 'yd', 'ye', 'sd', 'et'} <= self.by_id.keys())
        self.assertFalse({'ru', 'ua', 'ee', 'lv', 'lt', 'hr', 'si', 'rs', 'ba', 'cz', 'sk', 'ss', 'er'} & self.by_id.keys())

    def test_alliance_membership_and_non_alignment(self):
        nato = {k for k, r in self.blocs.items() if r['basis'] == 'nato_member'}
        self.assertEqual(nato, set('us ca gb fr be nl lu dk no is it pt gr tr de es'.split()))
        warsaw = {k for k, r in self.blocs.items() if r['basis'] == 'warsaw_member'}
        self.assertEqual(warsaw, set('su dd pl cs hu ro bg'.split()))
        for id in ('se', 'fi', 'ch', 'at', 'ie', 'yu', 'cn', 'al', 'ir', 'iq'):
            self.assertEqual(self.blocs[id]['bloc'], 'other', id)
        self.assertEqual(self.blocs['ni']['bloc'], 'east')
        self.assertIn('Contras', self.blocs['ni']['en'])
        for r in self.blocs.values():
            self.assertIn(r['bloc'], {'west', 'east', 'other'})
            self.assertTrue(r['sv'] and r['en'])
            self.assertEqual(r['sv'], r['en_source'])

    def test_mesh_indices_short_edges_and_finite_coordinates(self):
        total_vertices = total_indices = 0
        for id, land in self.by_id.items():
            vertices, indices = land['vertices'], land['indices']
            self.assertTrue(vertices and indices, id)
            self.assertEqual(len(indices) % 3, 0, id)
            total_vertices += len(vertices) + sum(map(len, land['borders']))
            total_indices += len(indices)
            for x, y in vertices:
                self.assertTrue(math.isfinite(x) and math.isfinite(y))
                self.assertTrue(-180 <= x <= 180 and -90 <= y <= 90, id)
            for index in indices:
                self.assertIsInstance(index, int)
                self.assertTrue(0 <= index < len(vertices), id)
            for i in range(0, len(indices), 3):
                triangle = [vertices[j] for j in indices[i:i+3]]
                # A long edge could span the back of the globe and leak a fill.
                self.assertLessEqual(max(p[0] for p in triangle)-min(p[0] for p in triangle), 3.00001, id)
                self.assertLessEqual(max(p[1] for p in triangle)-min(p[1] for p in triangle), 3.00001, id)
        self.assertLess(total_vertices, 400000)
        self.assertLess(total_indices, 1200000)

    def test_representative_historical_locations(self):
        # Test actual generated triangles, including the split German/Yemeni
        # meshes; state labels alone would not detect the wrong country fill.
        cases = [('su', 37.62, 55.75), ('su', 24.75, 59.44), ('de', 7.10, 50.73),
                 ('dd', 13.74, 51.05), ('cs', 14.42, 50.08), ('yu', 20.46, 44.81),
                 ('ye', 44.21, 15.37), ('yd', 44.95, 13.10), ('se', 18.05, 59.32),
                 ('fi', 25.0, 61.0), ('et', 38.93, 15.32), ('sd', 31.58, 4.85)]
        def contains(a,b,c,p):
            cross = lambda u,v,w: (v[0]-u[0])*(w[1]-u[1])-(v[1]-u[1])*(w[0]-u[0])
            signs = [cross(a,b,p), cross(b,c,p), cross(c,a,p)]
            return min(signs) >= -1e-8 or max(signs) <= 1e-8
        for id,x,y in cases:
            land = self.by_id[id]; vertices = land['vertices']; indices = land['indices']
            self.assertTrue(any(contains(*(vertices[j] for j in indices[i:i+3]), (x,y))
                for i in range(0,len(indices),3)), (id,x,y))

    def test_flag_coverage_rectangles_and_historical_variants(self):
        png = (CONTENT / 'flags_1986.png').read_bytes()
        self.assertEqual(png[:8], b'\x89PNG\r\n\x1a\n')
        width,height = struct.unpack('>II', png[16:24])
        flags = {r['id']: r for r in self.flags['flags']}
        self.assertEqual(len(flags), len(self.flags['flags']))
        countries = {r['id'] for r in self.world['entries'] if r['kind']=='country'}
        countries.update(p['flag'] for e in self.world['entries'] for p in e.get('participants', []) if p.get('flag'))
        self.assertEqual(countries, set(flags))
        for r in flags.values():
            x,y,w,h = r['rect']
            self.assertTrue(w > 0 and h > 0 and x >= 0 and y >= 0 and x+w <= width and y+h <= height)
            self.assertTrue(r['source'].startswith('https://'))
            self.assertTrue(r['license'])
        for id, period in [('af','1980'), ('iq','1963'), ('za','1994'), ('sy','1980')]:
            self.assertIn(period, flags[id]['historical_variant'])
        self.assertIn('Soviet', flags['su']['historical_variant'])

    def test_packaged_visual_assets_are_staged(self):
        build = (ROOT / 'Plugins/TMOPEngine/Source/TMOPEngine/TMOPEngine.Build.cs').read_text()
        for name in ('land_1986.json','alignments_1986.json','flags_1986.json','flags_1986.png'):
            self.assertIn(f'RuntimeDependencies.Add("$(PluginDir)/Content/WorldAtlas/{name}", StagedFileType.UFS)', build)


if __name__ == '__main__': unittest.main()
