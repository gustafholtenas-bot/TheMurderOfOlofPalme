"""Generate the country/airline research contents from the authoritative catalog."""
import argparse
import json
from collections import Counter
from pathlib import Path

CONTENT = Path(__file__).resolve().parents[2] / 'Plugins/TMOPEngine/Content/WorldAtlas/Flights'
OUTPUT = Path(__file__).resolve().parent / 'research/COUNTRY_INDEX.md'


def render(catalog, flights):
    movements = Counter(leg['airline'] for leg in flights['legs'])
    status = {
        'partial': 'Delvis granskad',
        'scan_found': 'Skanning hittad, ej inläst',
        'catalog_found': 'Katalogpost, ej inläst',
        'no_matching_edition': 'Giltig utgåva ej hittad',
        'not_searched': 'Manuell granskning återstår',
    }
    lines = [
        '# Flygtrafik 1986 – landvis innehållsförteckning',
        '',
        'Genererad från `catalog.json` och `flights.json` med `build_country_index.py`.',
        '✓ anger inlästa uppgifter, **inte en färdiginventerad operatör**. Status på varje rörelse skiljer tidtabell från dokumenterat genomförande.',
        '○ anger att inga avgångar från operatören har importerats. Antalet avser fysiska flygsträckor',
        'i ±24-timmarsfönstret. Flera hemländer kan lista samma operatör; landtalen får därför inte summeras.',
        'Operatörerna nedan grupperas efter hemland. Utländska bolags trafik visas i flygplatstabellen längre ned.',
        'Samtliga länder har ofullständig operatörsinventering. Listan över saknade bolag kan också vara ofullständig.',
        '',
        '| Land/territorium 1986 | Operatörer med granskade uppgifter | Registrerade operatörer utan rutter | Inventering |',
        '|---|---|---|---|',
    ]
    for country in catalog['countries']:
        airlines = [a for a in catalog['airlines'] if country['id'] in a['countries']]
        done = [f"✓ {a['name']} ({movements[a['id']]})" for a in airlines if movements[a['id']]]
        todo = [f"○ {a['name']} ({status.get(a['research_status'], a['research_status'])})" for a in airlines if not movements[a['id']]]
        census = 'Färdig' if country['operator_census_complete'] else 'Ofullständig'
        lines.append(f"| {country['name']['sv']} | {'<br>'.join(done) or '—'} | {'<br>'.join(todo) or '—'} | {census} |")
    lines += ['', f"**Totalt:** {len(catalog['countries'])} länder/territorier, {len(catalog['airlines'])} registrerade operatörer, {flights['coverage']['scheduled_movements']} tidtabellslagda och {flights['coverage']['confirmed_movements']} dokumenterat genomförda flygsträckor.", '',
              '## Trafik till och från landets flygplatser', '',
              'Här räknas varje fysisk delsträcka en gång per berört land, inklusive utländska operatörer.',
              'Ett inrikesflyg räknas en gång. Ett utrikesflyg finns under båda ändpunkternas länder; summera därför inte kolumnen.',
              'Noll betyder att inga rörelser importerats, inte att landet saknade flygtrafik.', '',
              '| Land/territorium 1986 | Importerade flygsträckor | Operatörer |', '|---|---:|---|']
    airports = {a['id']: a for a in catalog['airports']}
    names = {a['id']: a['name'] for a in catalog['airlines']}
    for country in catalog['countries']:
        legs = [r for r in flights['legs'] if country['id'] in
                {airports[r['origin']]['country'], airports[r['destination']]['country']}]
        operators = Counter(r['airline'] for r in legs)
        labels = [f'{names[key]} ({count})' for key, count in sorted(operators.items(), key=lambda x: names[x[0]])]
        lines.append(f"| {country['name']['sv']} | {len(legs)} | {'; '.join(labels) or '—'} |")
    lines.append('')
    return '\n'.join(lines)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    text = render(json.loads((CONTENT / 'catalog.json').read_text(encoding='utf-8')), json.loads((CONTENT / 'flights.json').read_text(encoding='utf-8')))
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text(encoding='utf-8') != text:
            raise SystemExit('COUNTRY_INDEX.md is stale; run build_country_index.py')
    else:
        OUTPUT.write_text(text, encoding='utf-8')
        print(OUTPUT)
