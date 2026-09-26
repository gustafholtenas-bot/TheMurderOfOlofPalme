"""Validate the historical dataset and its built-in English menu translations."""
import argparse
import datetime
import json
import math
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT = ROOT / 'Plugins/TMOPEngine/Content/StockMarket/markets.json'

def validate(data):
    def require(condition, message):
        if not condition:
            raise ValueError(message)
    def date(value):
        parsed = datetime.date.fromisoformat(value)
        require(parsed.isoformat() == value, f'Noncanonical date: {value}')
        return parsed
    def localized(value, field):
        require(isinstance(value, dict) and isinstance(value.get('sv'), str), f'{field}: missing Swedish source')
        require(isinstance(value.get('en'), str), f'{field}: missing English translation')
        require(not value['sv'] or value['en'], f'{field}: empty English translation')
        require(value.get('en_source') == value['sv'], f'{field}: English requires review')
    require(type(data.get('schema')) is int and data['schema'] == 1, 'Unsupported schema')
    require(date(data['before_date']) < date(data['after_date']), 'Dates are reversed')
    localized(data['method'], 'method')
    require(isinstance(data['sources'], list) and 1 <= len(data['sources']) <= 256, 'Invalid sources')
    require(isinstance(data['entries'], list) and 1 <= len(data['entries']) <= 4096, 'Invalid entries')
    ids = set()
    for source in data['sources']:
        require(isinstance(source['id'], str) and source['id'] and source['id'] not in ids, 'Duplicate or missing source ID')
        ids.add(source['id'])
        require(source['url'].startswith('https://'), 'Source URL must use HTTPS')
        require(bool(source['title']), 'Source title is empty')
        date(source['published'])
    seen = set()
    comparable = 0
    for entry in data['entries']:
        key = entry['id']
        require(bool(key) and key not in seen, f'Duplicate or missing ID: {key}')
        seen.add(key)
        for field in ('market', 'name', 'notes'):
            localized(entry[field], f'{key}.{field}')
        require(entry['region'] in {'europe','north_america','asia_pacific','africa','world'}, f'{key}: region')
        require(entry['kind'] in {'market','sector','world'}, f'{key}: kind')
        require(entry['status'] in {'close','daily','provisional','source_conflict','missing'}, f'{key}: status')
        require(type(entry['decimals']) is int and 0 <= entry['decimals'] <= 4, f'{key}: precision')
        for field in ('before', 'after'):
            value = entry[field]
            require(value is None or type(value) in (int,float) and math.isfinite(value) and 0 <= value <= 1e12, f'{key}: invalid {field}')
        a,b = entry['before'],entry['after']
        if a is None or b is None:
            require(entry['status'] in {'missing','source_conflict'}, f'{key}: unmarked missing value')
        elif a > 0:
            comparable += 1
        require(isinstance(entry['citations'], list) and 1 <= len(entry['citations']) <= 32, f'{key}: invalid citations')
        for ref in entry['citations']:
            require(ref['source'] in ids and bool(ref['pages']), f'{key}: broken citation')
    return comparable

def check_menu_translations():
    base = ROOT / 'Plugins/TMOPEngine/Source/TMOPEngine'
    code = (base / 'Private/StockMarket/STMOPStockMarket.cpp').read_text(encoding='utf-8')
    code += (base / 'Private/UI/TMOPPauseMenuWidget.cpp').read_text(encoding='utf-8')
    declared = {k:v for k,v in re.findall(r'NSLOCTEXT\("TMOP", "((?:Market|HubStockMarket)[^"]*)", "([^"]*)"\)',code)}
    translations = (base / 'Private/Localization/TMOPMenuTranslations.inl').read_text(encoding='utf-8')
    builtins = {k:(sv,en) for k,sv,en in re.findall(r'Add\(TEXT\("([^"]+)"\), TEXT\("([^"]*)"\), TEXT\("([^"]*)"\)\)',translations)}
    for key,source in declared.items():
        if key not in builtins or builtins[key][0] != source or not builtins[key][1]:
            raise ValueError(f'Missing/stale English menu entry: {key}')
        if set(re.findall(r'\{\d+\}',source)) != set(re.findall(r'\{\d+\}',builtins[key][1])):
            raise ValueError(f'Translation placeholders differ: {key}')
    return len(declared)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', nargs='?', type=pathlib.Path, default=DEFAULT)
    args = parser.parse_args()
    data = json.loads(args.file.read_text(encoding='utf-8'))
    count = validate(data)
    menus = check_menu_translations()
    print(f'OK: {len(data["entries"])} indices, {count} calculable changes, {menus} translated menu strings.')
