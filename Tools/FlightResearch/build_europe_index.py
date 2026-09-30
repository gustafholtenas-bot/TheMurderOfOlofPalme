#!/usr/bin/env python3
"""Generate the country-by-country Europe review from the current flight data."""
from pathlib import Path
from collections import Counter
import argparse
import json

ROOT = Path(__file__).resolve().parents[2]
RESEARCH = ROOT / 'Tools/FlightResearch/research'
DATA = ROOT / 'Plugins/TMOPEngine/Content/WorldAtlas/Flights'


def generate():
    catalog = json.loads((DATA / 'catalog.json').read_text(encoding='utf-8'))
    flights = json.loads((DATA / 'flights.json').read_text(encoding='utf-8'))
    queue = json.loads((RESEARCH / 'europe_country_queue.json').read_text(encoding='utf-8'))
    countries = {x['id']: x for x in catalog['countries']}
    airports = {x['id']: x for x in catalog['airports']}
    airlines = {x['id']: x for x in catalog['airlines']}
    rows = []
    assert len({x['id'] for x in queue['countries']}) == len(queue['countries'])
    for position, entry in enumerate(queue['countries'], 1):
        country_id = entry.get('catalog_country_id')
        if country_id is not None and country_id not in countries:
            raise ValueError(f'Unknown catalog country: {country_id}')
        legs = [x for x in flights['legs'] if country_id is not None and country_id in {
            airports[x['origin']]['country'], airports[x['destination']]['country']}]
        domestic = [x for x in legs if airports[x['origin']]['country'] == airports[x['destination']]['country']]
        by_airline = Counter(x['airline'] for x in legs)
        home = [x for x in catalog['airlines'] if country_id is not None and country_id in x['countries']]
        pairs = sorted({(x['origin'], x['destination']) for x in legs})
        rows.append(dict(order=position, id=entry['id'], name=entry['name'],
            catalog_country_id=country_id, review_status=entry['review_status'],
            coverage_complete=False, endpoint_movements=len(legs), domestic_movements=len(domestic),
            international_movements=len(legs)-len(domestic), directed_endpoint_pairs=pairs,
            movements_by_source_airline=dict(sorted(by_airline.items())),
            home_operator_ids=[x['id'] for x in home],
            imported_home_operator_ids=[x['id'] for x in home if any(y['airline']==x['id'] for y in flights['legs'])],
            registered_airport_ids=[x['id'] for x in catalog['airports'] if x['country']==country_id],
            endpoint_leg_ids=[x['id'] for x in legs], next_action=entry['next_action']))
    overview = dict(schema=1, checked_on=queue['checked_on'], package=queue['package'],
        total_movements=len(flights['legs']), next_country_id=queue['next_country_id'],
        coverage_complete=False, counting=queue['counting'], countries=rows)
    next_name = next(x['name'] for x in rows if x['id']==queue['next_country_id'])
    reviewed_names = ', '.join(x['name'] for x in rows if x['review_status']=='reviewed_with_gaps') or 'Inga länder'
    title = ['# Europa – landvis flyggenomgång', '',
        f"Uppdaterat {queue['checked_on']} för {queue['package']}. Paketet innehåller {len(flights['legs']):,} rörelser.".replace(',', ' '), '',
        f'Arbetsordningen gäller en ny granskning land för land. Äldre importer följer med, men räknas inte som en färdig landsinventering. Granskade urval i denna följd: {reviewed_names}. **{next_name} är nästa fördjupning**.', '',
        '**Rörelser räknas efter start- eller destinationsland, oavsett bolagets hemland.** Inrikesflyg räknas en gång i landraden. Ett gränsöverskridande flyg kan finnas i två landrader; landtalen ska inte summeras. Noll betyder inga inlagda rörelser, inte att flygtrafik saknades.', '',
        'Historiska länder används för 1986. Väst- och Östtyskland hålls isär, Västberlin redovisas separat, och Tjeckoslovakien/Jugoslavien/Sovjetunionen delas inte i dagens stater. Sovjetunionens och Turkiets siffror gäller hela kataloglandet, även asiatiska flygplatser. Cypern ingår i arbetsordningen. Särskilda territorier anges separat.', '',
        '| Ordning | Land/territorium 1986 | Inlagda rörelser | Inrikes | Granskning i denna följd |',
        '|---:|---|---:|---:|---|']
    for row in rows:
        status = 'Granskat urval; luckor kvar' if row['review_status']=='reviewed_with_gaps' else 'Nästa land' if row['id']==queue['next_country_id'] else 'I kö'
        title.append(f"| {row['order']} | {row['name']} | {row['endpoint_movements']} | {row['domestic_movements']} | {status} |")
    title += ['', '## Landvisa återstående uppgifter', '']
    for row in rows:
        title += [f"### {row['order']}. {row['name']}", '', row['next_action'], '']
        home = ', '.join(airlines[x]['name'] for x in row['home_operator_ids']) or 'Inga ännu registrerade'
        imported = ', '.join(f"{airlines[x]['name']} {count}" for x,count in row['movements_by_source_airline'].items()) or 'Inga inlagda rörelser'
        title += [f"Registrerade hemlandsoperatörer: {home}. Detta är inte en fullständig bolagslista.", '',
            f"Bolagskoder i landets inlagda rörelser: {imported}.", '']
    title += ['## Tillämpning', '',
        'För varje land: inventera samtida bolag och flygplatser, sök vintertabeller giltiga för mordhelgen, läs trafikdagar/fotnoter/stopp, importera endast tidsatta fysiska delsträckor och kontrollera mot befintliga rörelser. Dokumentera luckor innan nästa land. Återbesök ett tidigare land när nya källor hittas.', '',
        'En landgenomgång kan avslutas för denna omgång utan nya avgångar. `coverage_complete` förblir falskt. Charter, frakt, militärflyg och faktiska överflygningar behöver egna daterade underlag. Saknade klockslag uppskattas inte.', '',
        'Åland följs under Finland; Färöarna/Grönland under Danmark och Svalbard/Jan Mayen under Norge. Grönland är en nordisk utvidgning utanför Europa. Inga automatiska flygplatser eller avgångar skapas för dessa sökområden.', '',
        'Se `SWEDEN_COUNTRY_REVIEW_BATCH25.md`, `europe_country_queue.json` och `europe_country_index.json`. Den befintliga `COUNTRY_INDEX.md` grupperar i stället operatörer efter hemland.', '']
    return {RESEARCH/'EUROPE_COUNTRY_INDEX.md': '\n'.join(title),
        RESEARCH/'europe_country_index.json': json.dumps(overview, ensure_ascii=False, indent=2)+'\n'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    for path, content in generate().items():
        if args.check:
            if not path.exists() or path.read_text(encoding='utf-8') != content:
                raise SystemExit(f'Out of date: {path}')
        else:
            path.write_text(content, encoding='utf-8')
            print(path)


if __name__ == '__main__':
    main()
