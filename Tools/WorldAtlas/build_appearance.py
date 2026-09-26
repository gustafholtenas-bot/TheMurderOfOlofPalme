"""Build offline globe assets. Requires shapely>=2, numpy, mapbox-earcut, cairosvg, Pillow.

python build_appearance.py --countries ne_50m_admin_0_countries.geojson \
    --states ne_10m_admin_1_states_provinces.geojson

Input: Natural Earth public-domain data. Sources and cartographic limitations are
documented in APPEARANCE_1986.md. No downloads or Python are required by the game.
"""
import argparse
import io
import json
import math
from pathlib import Path

import cairosvg
import mapbox_earcut
import numpy as np
from PIL import Image
from shapely import make_valid
from shapely.geometry import shape, box, Polygon, MultiPolygon
from shapely.ops import unary_union

ROOT = Path(__file__).resolve().parents[2]
CONTENT = ROOT / 'Plugins/TMOPEngine/Content/WorldAtlas'
SOURCES = Path(__file__).parent / 'SourceAssets/flags'


def polygons(g):
    if isinstance(g, Polygon):
        if not g.is_empty and g.area > 1e-9:
            yield g
    elif hasattr(g, 'geoms'):
        for child in g.geoms:
            yield from polygons(child)


def dump(path, value, compact=False):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=None if compact else 2,
        separators=(',', ':') if compact else None) + '\n', encoding='utf-8')


def historical_shapes(countries, states):
    shapes, names, parents = {}, {}, {}
    for f in countries['features']:
        p = f['properties']
        id = p['ISO_A2_EH'].lower()
        if id == '-99':
            id = {'SOL': 'so', 'CYN': 'cy', 'KAS': 'kashmir'}[p['ADM0_A3']]
        shapes.setdefault(id, []).append(make_valid(shape(f['geometry'])))
        names[id] = p['NAME']
        parents[id] = p['SOV_A3']
    shapes = {id: unary_union(geoms) for id, geoms in shapes.items()}
    # State succession is separate from ideology; China and Yugoslavia are NOT
    # automatically classified as Soviet-bloc because their governments were socialist.
    merges = {
        'su': ('Soviet Union', 'ru ua by md ee lv lt ge am az kz uz tm kg tj'),
        'yu': ('Yugoslavia', 'rs me ba hr si mk xk'),
        'cs': ('Czechoslovakia', 'cz sk'),
        'sd': ('Sudan', 'sd ss'),
        'et': ('Ethiopia', 'et er'),
    }
    for target, (name, ids) in merges.items():
        shapes[target] = unary_union([shapes.pop(id) for id in ids.split()])
        names[target] = name
    germany = shapes.pop('de')
    east_names = {'Sachsen', 'Thüringen', 'Sachsen-Anhalt', 'Brandenburg', 'Mecklenburg-Vorpommern', 'Berlin'}
    east = unary_union([shape(f['geometry']) for f in states['features']
        if f['properties'].get('adm0_a3') == 'DEU' and f['properties']['name'] in east_names])
    # Extend only the outer national edge to match the 1:50m coast, then divide
    # the original polygon: no overlapping fills or coastline slivers.
    east = east.union(east.buffer(.03).difference(germany.buffer(-.04)))
    shapes['dd'] = germany.intersection(east)
    shapes['de'] = germany.difference(east)
    names['de'], names['dd'] = 'West Germany', 'East Germany'
    # Generalised former South Yemen, reconstructed from coastal governorates.
    # Al Dali was created later from both sides; only its southern part is used.
    south_names = {'Hadramawt', 'Al Mahrah', 'Lahij', '`Adan', 'Abyan', 'Shabwah'}
    south_parts = []
    for f in states['features']:
        p = f['properties']
        if p.get('adm0_a3') != 'YEM':
            continue
        if p['name'] in south_names:
            south_parts.append(shape(f['geometry']))
        elif p['name'] == "Al Dali'":
            south_parts.append(shape(f['geometry']).intersection(box(40, 10, 50, 13.85)))
    yemen = shapes.pop('ye')
    south = unary_union(south_parts)
    south = south.union(south.buffer(.03).difference(yemen.buffer(-.04)))
    shapes['yd'] = yemen.intersection(south)
    shapes['ye'] = yemen.difference(south)
    names['ye'], names['yd'] = 'North Yemen', 'South Yemen'
    names.update({'mm': 'Burma', 'cd': 'Zaire', 'na': 'South West Africa (Namibia)',
        'ps': 'Palestinian territories', 'tl': 'East Timor (occupied)', 'aq': 'Antarctica'})
    return shapes, names, parents


def mesh(g):
    vertices, indices, borders, lookup = [], [], [], {}
    def add_vertex(xy):
        key = tuple(round(float(v), 6) for v in xy)
        if key not in lookup:
            lookup[key] = len(vertices)
            vertices.append(list(key))
        return lookup[key]
    for p in polygons(g):
        p = p.simplify(.018, preserve_topology=True)
        for ring in [p.exterior, *p.interiors]:
            # Borders also need short edges near the horizon and at high zoom.
            points = []
            xy = list(ring.coords)
            for a, b in zip(xy, xy[1:]):
                count = max(1, math.ceil(max(abs(a[0]-b[0]), abs(a[1]-b[1])) / 1.5))
                points.extend([[round(a[0]+(b[0]-a[0])*i/count, 6),
                    round(a[1]+(b[1]-a[1])*i/count, 6)] for i in range(count)])
            points.append([round(v, 6) for v in xy[-1]])
            borders.append(points)
        # Grid clipping bounds triangle extent and prevents a large country's
        # triangles cutting through the globe. Earcut preserves lakes and holes.
        lo, la, hi, ha = p.bounds
        for x in range(math.floor(lo / 3)*3, math.ceil(hi / 3)*3, 3):
            for y in range(math.floor(la / 3)*3, math.ceil(ha / 3)*3, 3):
                for piece in polygons(p.intersection(box(x, y, x+3, y+3))):
                    rings = [list(piece.exterior.coords)[:-1]] + [list(r.coords)[:-1] for r in piece.interiors]
                    coords = np.array([v for ring in rings for v in ring], dtype=np.float64)
                    ends = np.cumsum([len(r) for r in rings], dtype=np.uint32)
                    triangles = mapbox_earcut.triangulate_float64(coords, ends)
                    ids = [add_vertex(v) for v in coords]
                    indices.extend(ids[int(i)] for i in triangles)
    return {'vertices': vertices, 'indices': indices, 'borders': borders}


def alignments(ids, names, parents):
    nato = set('us ca gb fr be nl lu dk no is it pt gr tr de es'.split())
    warsaw = set('su dd pl cs hu ro bg'.split())
    west = set('il jp kr au nz ph th tw eg hn sv gt pk za'.split())
    east = set('cu vn la kh mn kp af ao mz et yd ni'.split())
    neutral = set('se ch at fi ie'.split())
    notes = {
        'nz': ('Västanknutet: ANZUS, men pågående kärnvapentvist med USA; USA:s formella suspendering kom senare 1986.', 'Western alignment: ANZUS, with an ongoing nuclear-policy dispute; formal US suspension followed later in 1986.'),
        'za': ('Västorienterad antikommunism under apartheid; internationellt isolerat, inte NATO-medlem.', 'Western-oriented anti-communism under apartheid; internationally isolated, not a NATO member.'),
        'ni': ('Sovjetiskt/kubanskt stöd till sandinistregeringen. Färgen gäller regeringen, inte Contras eller hela befolkningen.', 'Soviet/Cuban support for the Sandinista government. Colour describes the government, not the Contras or the population.'),
        'cn': ('Utanför de två blocken: egen kommunistisk stormakt efter brytningen med Sovjetunionen.', 'Outside the two blocs: an independent communist power after the Sino-Soviet split.'),
        'yu': ('Alliansfritt socialistiskt land; inte medlem av Warszawapakten.', 'Non-aligned socialist state; not a Warsaw Pact member.'),
        'al': ('Utanför blocken: Albanien hade lämnat Warszawapakten 1968.', 'Outside the blocs: Albania left the Warsaw Pact in 1968.'),
        'ir': ('Alliansfritt; krig med Irak och kontakter i flera riktningar innebär inte blockmedlemskap.', 'Non-aligned; war with Iraq and dealings across blocs do not imply bloc membership.'),
        'iq': ('Alliansfritt; både sovjetiska och västliga förbindelser. Ingen entydig blockklassning.', 'Non-aligned; both Soviet and Western relationships. No unambiguous bloc classification.'),
        'sy': ('Alliansfritt med omfattande sovjetiskt militärt stöd; inte klassat som formell östblocksmedlem.', 'Non-aligned with substantial Soviet military support; not classified as a formal Eastern-bloc member.'),
        'na': ('Sydvästafrika under sydafrikansk administration, med omstridd och internationellt förkastad kontroll.', 'South West Africa under South African administration, whose control was contested and internationally rejected.'),
        'ps': ('Palestinska områden under israelisk ockupation; färgen anger ingen suveränitet eller neutralitet.', 'Palestinian territories under Israeli occupation; colour implies neither sovereignty nor neutrality.'),
        'tl': ('Östtimor under indonesisk ockupation; färgen anger ingen suveränitet eller neutralitet.', 'East Timor under Indonesian occupation; colour implies neither sovereignty nor neutrality.'),
        'aq': ('Antarktis: ingen blockklassning.', 'Antarctica: no bloc classification.'),
    }
    result = []
    for id in sorted(ids):
        bloc, basis = 'other', 'outside_or_unclassified'
        sv, en = 'Övrigt/alliansfritt: ingen entydig blockklassning i denna översikt.', 'Other/non-aligned: no unambiguous bloc classification in this overview.'
        if id in nato:
            bloc, basis = 'west', 'nato_member'
            sv, en = 'Västblocket: NATO-medlem den 28 februari 1986.', 'Western bloc: NATO member on 28 February 1986.'
        elif id in warsaw:
            bloc, basis = 'east', 'warsaw_member'
            sv, en = 'Östblocket: medlem av Warszawapakten den 28 februari 1986.', 'Eastern bloc: Warsaw Pact member on 28 February 1986.'
        elif id in west:
            bloc, basis = 'west', 'western_alignment'
            sv, en = 'Västanknutet: förenklad politisk/säkerhetspolitisk orientering, inte NATO-medlemskap.', 'Western-aligned: simplified political/security alignment, not NATO membership.'
        elif id in east:
            bloc, basis = 'east', 'soviet_alignment'
            sv, en = 'Sovjetanknutet: förenklad politisk/säkerhetspolitisk orientering, inte medlemskap i Warszawapakten.', 'Soviet-aligned: simplified political/security alignment, not Warsaw Pact membership.'
        elif id in neutral:
            basis = 'neutral_or_militarily_non_aligned'
            sv, en = 'Neutralt/militärt alliansfritt den 28 februari 1986.', 'Neutral/militarily non-aligned on 28 February 1986.'
        elif parents.get(id) in {'US1', 'GB1', 'FR1', 'NL1', 'DN1', 'AU1'}:
            bloc, basis = 'west', 'administered_territory'
            sv, en = 'Territorium administrerat av en västanknuten stat; inte eget blockmedlemskap.', 'Territory administered by a Western-aligned state; not separate bloc membership.'
        if id == 'na': bloc, basis = 'west', 'administered_territory'
        if id in notes: sv, en = notes[id]
        result.append({'id': id, 'name': names.get(id, id), 'bloc': bloc, 'basis': basis, 'sv': sv, 'en': en, 'en_source': sv})
    return result


def flags():
    sources = json.loads((SOURCES / 'sources.json').read_text(encoding='utf-8'))
    entries = json.loads((CONTENT / 'world.json').read_text(encoding='utf-8'))['entries']
    countries = {e['id'] for e in entries if e['kind'] == 'country'}
    countries.update(p['flag'] for e in entries for p in e.get('participants', []) if p.get('flag'))
    atlas = Image.new('RGBA', (576, math.ceil(len(countries) / 8) * 56))
    rows = []
    for n, id in enumerate(sorted(countries)):
        # CairoSVG fits each source's original aspect ratio; two-pixel gutters
        # keep bilinear sampling from leaking neighbouring flags into the icon.
        png = cairosvg.svg2png(url=str(SOURCES / (id+'.svg')), output_width=64)
        img = Image.open(io.BytesIO(png)).convert('RGBA')
        if img.height > 48: img = img.resize((round(img.width * 48 / img.height), 48), Image.Resampling.LANCZOS)
        x, y = (n % 8) * 72 + 4, (n // 8) * 56 + 4
        atlas.paste(img, (x, y))
        rows.append({'id': id, 'rect': [x, y, img.width, img.height], **sources[id]})
    atlas.save(CONTENT / 'flags_1986.png')
    dump(CONTENT / 'flags_1986.json', {'schema': 1, 'reference_date': '1986-02-28', 'flags': rows})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--countries', type=Path, required=True)
    parser.add_argument('--states', type=Path, required=True)
    parser.add_argument('--write-alignments', action='store_true', help='Explicitly reset editorial bloc assignments to the initial snapshot')
    args = parser.parse_args()
    shapes, names, parents = historical_shapes(json.loads(args.countries.read_text()), json.loads(args.states.read_text()))
    result = [{'id': id, **mesh(g)} for id, g in sorted(shapes.items())]
    dump(CONTENT / 'land_1986.json', {'schema': 1, 'reference_date': '1986-02-28',
        'source': 'Natural Earth public domain; generalised historical reconstruction, see Tools/WorldAtlas/APPEARANCE_1986.md',
        'countries': result}, compact=True)
    if args.write_alignments or not (CONTENT / 'alignments_1986.json').exists():
        dump(CONTENT / 'alignments_1986.json', {'schema': 1, 'reference_date': '1986-02-28',
            'classification': 'Editorial simplification: alliance members plus selected aligned governments. Other does not mean neutral.',
            'sources': ['https://www.nato.int/en/about-us/organization/nato-member-countries',
                'https://history.state.gov/milestones/1953-1960/warsaw-treaty',
                'https://history.state.gov/milestones/1945-1952/anzus',
                'https://www.cia.gov/readingroom/document/cia-rdp85t00287r000901590001-9'],
            'countries': alignments(shapes, names, parents)})
    flags()
    print(f"{len(result)} land/territory meshes, {sum(len(r['indices'])//3 for r in result)} triangles; 33 historical flag icons")


if __name__ == '__main__': main()
