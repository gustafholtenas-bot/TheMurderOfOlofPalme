"""Compile reviewed timetable legs into UTC movements; Python 3.9+, tzdata required.

Never interpret a connecting itinerary as a nonstop flight. OCR candidates do not
enter the game. Local dates, source validity and day codes are authoritative.
"""
import argparse
import copy
import datetime as dt
import hashlib
import json
import math
from pathlib import Path
from zoneinfo import ZoneInfo

ROOT = Path(__file__).resolve().parents[2]
CONTENT = ROOT / 'Plugins/TMOPEngine/Content/WorldAtlas/Flights'
UTC = dt.timezone.utc

def require(value, message):
    if not value:
        raise ValueError(message)

def date(value):
    d = dt.date.fromisoformat(value)
    require(d.isoformat() == value, f'Noncanonical date: {value}')
    return d

def after_effective_month(value):
    # A month-only cover does not establish the exact day the edition took effect.
    require(isinstance(value, str) and len(value) == 7, 'Effective month must be YYYY-MM')
    first = date(value + '-01')
    return (first.replace(day=28) + dt.timedelta(days=4)).replace(day=1)

def instant(value):
    d = dt.datetime.fromisoformat(value.replace('Z', '+00:00'))
    require(d.tzinfo is not None, 'UTC offset required: ' + value)
    return d.astimezone(UTC)

def iso(value):
    return value.astimezone(UTC).isoformat(timespec='seconds').replace('+00:00', 'Z')

def indexed(rows, label):
    out = {}
    for row in rows:
        key = row.get('id')
        require(isinstance(key, str) and key and key not in out, f'{label}: duplicate/empty ID {key}')
        out[key] = row
    return out

def local_to_utc(day, clock, timezone, fold=None):
    require(len(clock) == 5 and clock[2] == ':', f'Clock must be HH:MM: {clock}')
    local = dt.datetime.combine(day, dt.time.fromisoformat(clock))
    zone = ZoneInfo(timezone)
    candidates = {}
    for f in (0, 1):
        candidate = local.replace(tzinfo=zone, fold=f).astimezone(UTC)
        if candidate.astimezone(zone).replace(tzinfo=None) == local:
            candidates[f] = candidate
    require(candidates, f'Nonexistent local time: {local} {timezone}')
    if len(set(candidates.values())) > 1:
        require(fold in candidates, f'Ambiguous local time needs fold: {local} {timezone}')
    return candidates[fold] if fold in candidates else next(iter(candidates.values()))

def compile_catalog(catalog):
    require(catalog['schema'] == 1, 'Unsupported catalog schema')
    anchor = instant(catalog['anchor'])
    start, end = anchor - dt.timedelta(hours=24), anchor + dt.timedelta(hours=24)
    countries = indexed(catalog['countries'], 'countries')
    airlines = indexed(catalog['airlines'], 'airlines')
    airports = indexed(catalog['airports'], 'airports')
    sources = indexed(catalog['sources'], 'sources')
    schedules = indexed(catalog['schedules'], 'schedules')
    for airport in airports.values():
        require(airport['country'] in countries, 'Unknown airport country')
        require(math.isfinite(airport['lat']) and -90 <= airport['lat'] <= 90, 'Latitude')
        require(math.isfinite(airport['lon']) and -180 <= airport['lon'] <= 180, 'Longitude')
        require(bool(airport['coordinate_source']), 'Airport coordinate source required')
        ZoneInfo(airport['timezone'])
    for airline in airlines.values():
        require(airline['countries'] and all(c in countries for c in airline['countries']), 'Airline country')
        require(all(s in sources for s in airline['source_ids']), 'Airline source')
    for source in sources.values():
        require(source['url'].startswith('https://'), 'Source URL must use HTTPS')
        if source.get('effective_from_month'):
            after_effective_month(source['effective_from_month'])
        if source.get('valid_from') and source.get('valid_to'):
            require(date(source['valid_from']) <= date(source['valid_to']), 'Source dates reversed')
    observations = {}
    for row in catalog.get('observations', []):
        require(row['schedule'] in schedules, 'Observation has no schedule')
        key = row['schedule'], row['departure_date']
        require(key not in observations, 'Conflicting observations require manual resolution')
        require(row['review_status'] == 'reviewed' and row['source'] in sources and row['page'], 'Observation evidence')
        require(sources[row['source']]['kind'] == 'movement_record', 'Actual movement requires movement-record evidence')
        require(row['status'] in {'confirmed', 'cancelled'}, 'Unknown observation status')
        observations[key] = row
    legs, excluded, duplicates, used_observations = [], [], set(), set()
    for key, s in schedules.items():
        require(s['airline'] in airlines and s['origin'] in airports and s['destination'] in airports, key + ': reference')
        require(s['origin'] != s['destination'], key + ': same airport')
        require(s['source'] in sources and s['page'] and s['flight'], key + ': citation/flight')
        require(s['review_status'] in {'candidate', 'reviewed', 'rejected'}, key + ': review status')
        if s['review_status'] != 'reviewed':
            continue
        source = sources[s['source']]
        require(source['kind'] == 'timetable' and source['access'] in {'full_scan', 'partial_scan'}, key + ': not a readable timetable')
        require(s['nonstop'] is True, key + ': split intermediate stops into physical legs first')
        first, last = date(s['valid_from']), date(s['valid_to'])
        require(first <= last, key + ': reversed validity')
        require(source.get('window_reviewed') is True and (source.get('valid_from') or source.get('valid_to') or source.get('effective_from_month')), key + ': source applicability not reviewed')
        if source.get('valid_from'):
            require(date(source['valid_from']) <= first, key + ': before source validity')
        if source.get('valid_to'):
            require(last <= date(source['valid_to']), key + ': after source validity')
        if source.get('effective_from_month') and not source.get('valid_from'):
            require(first >= after_effective_month(source['effective_from_month']), key + ': exact effective day is unknown within this month')
        require(s['days'] and len(set(s['days'])) == len(s['days']) and all(type(v) is int and 1 <= v <= 7 for v in s['days']), key + ': weekdays')
        require(type(s['arrival_day_offset']) is int and -2 <= s['arrival_day_offset'] <= 3, key + ': arrival date offset')
        require(type(s.get('service_day_offset', 0)) is int and -3 <= s.get('service_day_offset', 0) <= 3, key + ': service date offset')
        omitted = {date(x) for x in s.get('excluded_dates', [])}
        added = {date(x) for x in s.get('included_dates', [])}
        require(not omitted.intersection(added), key + ': conflicting date exceptions')
        origin, destination = airports[s['origin']], airports[s['destination']]
        # Include flights already airborne at the left boundary and date-line offsets.
        day = max(first, start.date() - dt.timedelta(days=4))
        stop = min(last, end.date() + dt.timedelta(days=2))
        while day <= stop:
            if day not in omitted and (day.isoweekday() in s['days'] or day in added):
                departure = local_to_utc(day, s['departure'], origin['timezone'], s.get('departure_fold'))
                arrival = local_to_utc(day + dt.timedelta(days=s['arrival_day_offset']), s['arrival'], destination['timezone'], s.get('arrival_fold'))
                require(0 < (arrival - departure).total_seconds() <= 72 * 3600, key + ': implausible/reversed duration')
                obskey = key, day.isoformat()
                observation = observations.get(obskey)
                status = 'scheduled'
                if observation:
                    used_observations.add(obskey)
                    if observation['status'] == 'cancelled':
                        if departure < end and arrival > start:
                            excluded.append({'schedule': key, 'departure_date': day.isoformat(), 'reason': 'cancelled', 'source': observation['source'], 'page': observation['page']})
                        day += dt.timedelta(days=1)
                        continue
                    departure, arrival = instant(observation['departure_utc']), instant(observation['arrival_utc'])
                    require(0 < (arrival - departure).total_seconds() <= 72 * 3600, key + ': actual duration')
                    status = 'confirmed'
                if departure < end and arrival > start:
                    identity = s['airline'], s['flight'], s['origin'], s['destination'], iso(departure)
                    require(identity not in duplicates, key + ': duplicate physical movement; resolve overlapping editions/codeshares')
                    duplicates.add(identity)
                    local_dep = departure.astimezone(ZoneInfo(origin['timezone']))
                    local_arr = arrival.astimezone(ZoneInfo(destination['timezone']))
                    leg = dict(id=key + '@' + day.isoformat(), schedule=key, airline=s['airline'], flight=s['flight'],
                               origin=s['origin'], destination=s['destination'], status=status,
                               departure_utc=iso(departure), arrival_utc=iso(arrival),
                               departure_local=local_dep.isoformat(timespec='minutes'), arrival_local=local_arr.isoformat(timespec='minutes'),
                               departure_seconds=(departure-start).total_seconds(), arrival_seconds=(arrival-start).total_seconds(),
                               source=s['source'], page=s['page'], notes=s.get('notes', {'sv':'','en':'','en_source':''}),
                               service_group=s.get('service_group',key), leg_index=s.get('leg_index',1))
                    leg['journey'] = s['airline'] + '/' + s.get('service_group',key) + '@' + (day-dt.timedelta(days=s.get('service_day_offset',0))).isoformat()
                    if observation:
                        leg.update(source=observation['source'], page=observation['page'], timetable_source=s['source'])
                    legs.append(leg)
            day += dt.timedelta(days=1)
    require(set(observations).issubset(used_observations), 'Observation does not match an operating schedule date in this window')
    legs.sort(key=lambda x: (x['departure_seconds'], x['id']))
    journeys = {}
    for leg in legs:
        journeys.setdefault(leg['journey'], []).append(leg)
    for journey, parts in journeys.items():
        parts.sort(key=lambda p:p['leg_index'])
        for a,b in zip(parts,parts[1:]):
            require(a['leg_index'] != b['leg_index'], journey + ': duplicate leg number')
            require(a['arrival_seconds'] <= b['departure_seconds'], journey + ': overlapping legs')
            if b['leg_index'] == a['leg_index']+1:
                require(a['destination'] == b['origin'], journey + ': disconnected consecutive legs')
    require(len(legs) <= 200000, 'Split research into additional windows before exceeding runtime limit')
    result = {k: copy.deepcopy(catalog[k]) for k in ('countries','airlines','airports','sources')}
    result.update(schema=1, anchor_utc=iso(anchor), start_utc=iso(start), end_utc=iso(end),
                  anchor_seconds=86400, duration_seconds=172800, catalog_sha256=hashlib.sha256(json.dumps(catalog,sort_keys=True,ensure_ascii=False).encode()).hexdigest(),
                  coverage_complete=False, method=catalog['method'], legs=legs, excluded=excluded,
                  coverage={'registered_airlines': len(airlines), 'registered_countries': len(countries),
                            'reviewed_schedules':sum(s['review_status']=='reviewed' for s in schedules.values()),
                            'animated_airlines':len({x['airline'] for x in legs}),
                            'scheduled_movements':sum(x['status']=='scheduled' for x in legs),
                            'confirmed_movements':sum(x['status']=='confirmed' for x in legs)})
    return result

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--catalog', type=Path, default=CONTENT/'catalog.json')
    parser.add_argument('--output', type=Path, default=CONTENT/'flights.json')
    parser.add_argument('--check', action='store_true', help='Fail if checked-in runtime JSON is stale')
    args = parser.parse_args()
    output = compile_catalog(json.loads(args.catalog.read_text(encoding='utf-8')))
    text = json.dumps(output, ensure_ascii=False, indent=2) + '\n'
    if args.check:
        require(args.output.read_text(encoding='utf-8') == text, 'flights.json is stale; run build_flights.py')
    else:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(text,encoding='utf-8')
    print(json.dumps(output['coverage'],ensure_ascii=False))

if __name__ == '__main__':
    main()
