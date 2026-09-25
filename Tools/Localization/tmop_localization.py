#!/usr/bin/env python3
"""Export/merge/validate TMOP text packs without rewriting gameplay tables.

Use --tables with an explicit list of current JSON exports. Historical backups
are never selected implicitly. Blank translations keep the Swedish source.
"""
from __future__ import annotations
import argparse
import ast
import hashlib
import json
import re
from pathlib import Path

SCHEMA_VERSION = 2
STATUSES = {"untranslated", "draft", "reviewed", "needs_review"}
# The 09_07 intro export changed filename casing, but the Unreal asset did not.
TABLE_NAME_ALIASES = {'dt_tmop_introcards': 'DT_TMOP_IntroCards'}
# Human-facing fields only. IDs, paths, names, addresses, routes and times stay out.
TEXT_FIELDS = {
    'Title', 'Heading', 'Body', 'Text', 'Caption', 'Description', 'Description1986',
    'SourceNote', 'SupportingFactors', 'ContradictingFactors',
    'ShortDescription', 'LongDescription', 'ShortSummary', 'Summary',
    'AgentTimelineSummary', 'ObservationSummary', 'PostMurderEventsSummary',
    'BeforeShot', 'AfterShot', 'Occupation', 'Nationality', 'OriginalText',
    'Transcript', 'Subtitle', 'SubtitleText', 'DialogText', 'DialogueText',
    'EvidenceStatus', 'DateNote', 'Source', 'BuildingDescription', 'History',
    'PresentDayNote', 'FloorLabel', 'RegistrationNotes', 'AppearanceDescription',
    'Signalement', 'PublicSummary', 'PublicDescription', 'Overview',
    'JacketOrCoat', 'OtherCharacteristics', 'ObservedDescription', 'OriginalSummary',
    'ObservationConditions', 'SectionDescription', 'ImplementedSummary', 'RemainingWork',
    'PageOrLocation', 'CitationText', 'RegistryAddressQualifier', 'RouteSegmentName',
    'HairDescription', 'ClothingDescription', 'SourceSummary',
}
NEVER_FIELDS = {
    'Name', 'EntityId', 'FullName', 'FirstName', 'LastName', 'ArchivalFullName',
    'InGameDisplayName', 'FamilySurname', 'StreetName', 'HistoricalAddress',
    'SourceReference', 'GeneralSourceReference', 'AgentInfoSourceReference',
    'Uppslag', 'Notes', 'AssetPath', 'RowName', 'RecordingId', 'SegmentId',
}
STRING = r'"(?:[^"\\]|\\.)*"'
MACRO = re.compile(r'NSLOCTEXT\(\s*('+STRING+r')\s*,\s*('+STRING+r')\s*,\s*((?:'+STRING+r'\s*)+)\)',re.S)

def decode_cpp(value: str) -> str:
    return ''.join(ast.literal_eval(x) for x in re.findall(STRING,value))

def read_json(path: Path):
    raw=path.read_bytes()
    encoding="utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding))

def source_key(source: str) -> str:
    return hashlib.sha1(source.encode('utf-8')).hexdigest()

def table_key(table: str, row: str, field: str) -> str:
    # Slash and percent are escaped; dots/brackets belong to the field path.
    return '/'.join(v.replace('%', '%25').replace('/', '%2F') for v in (table, row, field))

def source_text(value: str) -> str:
    m=MACRO.fullmatch(value.strip())
    if m: return decode_cpp(m[3])
    # Culture-invariant imported values must not be promoted to translatable text.
    if value.startswith('INVTEXT('): return ''
    return value

def arguments(value: str) -> list[str]:
    # Same grammar as the runtime loader; preserve escaped braces and exact tokens.
    found=[]; i=0
    while i < len(value):
        if value[i]=='`': i+=2; continue
        if value[i]=='{':
            start=i; i+=1
            while i<len(value) and value[i]!='}':i+=1
            found.append(value[start:i+1])
        i+=1
    return sorted(found)

def validate(pack: dict) -> list[str]:
    if not isinstance(pack,dict):return ["pack root must be an object"]
    errors=[]
    version=pack.get('schema_version')
    if version not in (1,2): errors.append('schema_version must be 1 or 2')
    if pack.get('language') != 'en': errors.append('language must be en')
    rows=pack.get('entries')
    if not isinstance(rows,list) or len(rows)>100000:return errors+['entries must be an array of at most 100000 entries']
    seen=set()
    for index,e in enumerate(rows):
        if not isinstance(e,dict) or not all(isinstance(e.get(k),str) for k in ['namespace','key','source','translation']):
            errors.append(f'entry {index}: missing string fields');continue
        ns,key,source,translation=(e[k] for k in ['namespace','key','source','translation'])
        if ns not in ('TMOP','TMOP.Source','TMOP.Table'):errors.append(f'{index}: unsupported namespace')
        if not key or not source:errors.append(f'{index}: empty identity/source')
        if len(source)>1000000 or len(translation)>1000000:errors.append(f'{index}: text exceeds limit')
        if ns=='TMOP.Source' and key!=source_key(source):errors.append(f'{index}: source hash mismatch')
        if version==2 or ns=='TMOP.Table':
            if e.get('source_hash')!=source_key(source):errors.append(f'{index}: source_hash mismatch')
            if e.get('status') not in STATUSES:errors.append(f'{index}: invalid review status')
            if e.get('status')=='reviewed' and not translation:errors.append(f'{index}: reviewed entry is empty')
            if ns=='TMOP.Table':
                parts=[e.get(k) for k in ('table','row','field')]
                if not all(isinstance(v,str) and v for v in parts) or key!=table_key(*parts):
                    errors.append(f'{index}: table identity mismatch')
            aliases=e.get('native_ids',[])
            if not isinstance(aliases,list) or any(not isinstance(a,dict) or not all(isinstance(a.get(k),str) and a[k] for k in ('namespace','key')) for a in aliases):
                errors.append(f'{index}: invalid native_ids')
        if (ns,key) in seen:errors.append(f'{index}: duplicate identity')
        seen.add((ns,key))
        if translation and arguments(source)!=arguments(translation):errors.append(f'{index}: changed format arguments')
    return errors

def spawn_candidate(table: str, row: dict) -> bool:
    """Data-level candidate, not proof of a spawned actor in the current map."""
    if table not in ('DT_TMOP_People','DT_TMOP_HistoricalVehicles'):return True
    person=table=='DT_TMOP_People'
    if not row.get('bSpawnInSimulation',person):return False
    identity=row.get('EntityId' if person else 'VehicleId')
    if not identity or identity=='None':return False
    def enum(value):return str(value).rsplit('::',1)[-1]
    return any(enum(e.get('Action')) in ('InitialPlacement','Spawn') and
        (not person or enum(e.get('Usage','Simulation'))!='DocumentationOnly')
        for e in row.get('Timeline',[]) if isinstance(e,dict))

def latest_dated_tables(root: Path, tables: list[Path]) -> list[Path]:
    """Find newest MM_DD export per explicit table name, ignoring dated backups of other tables."""
    candidates={}
    for path in (root/'DataTables').glob('*/*.json'):
        if not re.fullmatch(r'\d{2}_\d{2}',path.parent.name):continue
        month,day=map(int,path.parent.name.split('_'))
        if not (1<=month<=12 and 1<=day<=31):continue
        candidates.setdefault(path.name.casefold(),[]).append(((month,day),path))
    selected=[]
    for requested in tables:
        options=candidates.get(requested.name.casefold(),[])
        if not options:raise ValueError(f'No dated export for {requested.name}')
        date=max(d for d,p in options)
        newest=[p for d,p in options if d==date]
        if len(newest)!=1:raise ValueError(f'Ambiguous same-date exports: {newest}')
        selected.append(newest[0].relative_to(root))
    return selected

def collect(root: Path,tables: list[Path],spawn_only: bool=False):
    entries={};audit=[]
    def add(ns,key,source,context):
        if not source.strip():return
        ident=(ns,key)
        if ident in entries and entries[ident]['source']!=source:
            raise ValueError(f'Conflicting source for {ident}: {context}')
        e=entries.setdefault(ident,dict(namespace=ns,key=key,source=source,source_hash=source_key(source),translation='',status='untranslated',contexts=[]))
        if context not in e['contexts']:e['contexts'].append(context)
        return e
    runtime=root/'Plugins/TMOPEngine/Source/TMOPEngine'
    for path in sorted(runtime.rglob('*')):
        if path.suffix not in ('.cpp','.h') or 'Tests' in path.name:continue
        content=path.read_text(encoding='utf-8-sig')
        for m in MACRO.finditer(content):
            ns,key,source=(decode_cpp(m[i]) for i in (1,2,3))
            if ns=='TMOP':add(ns,key,source,f'{path.relative_to(root)}:{content[:m.start()].count(chr(10))+1}')
        # Explicitly list remaining unsafe conversions instead of claiming 100% coverage.
        for lineno,line in enumerate(content.splitlines(),1):
            if 'FText::FromString' in line or ('FString::Printf' in line and '/UI/' in str(path)):
                audit.append({'file':str(path.relative_to(root)),'line':lineno,'code':line.strip()})
    skipped={}; table_info=[]; table_ids=set(); excluded_rows=[]
    for path in tables:
        path=path if path.is_absolute() else root/path
        table=TABLE_NAME_ALIASES.get(path.stem.casefold(), path.stem)
        if table in table_ids:raise ValueError(f'Duplicate table identity {table}; select one current export per table')
        table_ids.add(table)
        data=read_json(path)
        if not isinstance(data,list):raise ValueError(f'{path}: expected exported DataTable row array')
        selected_count=0
        table_record={'path':str(path.relative_to(root)),'rows':len(data)}
        table_info.append(table_record)
        def walk(value,context,row_id,field_path='',field=''):
            if isinstance(value,dict):
                for k,v in value.items():
                    walk(v,context+'.'+k,row_id,field_path+'.'+k if field_path else k,k)
            elif isinstance(value,list):
                identity=None
                for candidate in ('LocalizationId','SegmentId','EntryId'):
                    ids=[v.get(candidate) if isinstance(v,dict) else None for v in value]
                    if ids and all(isinstance(v,str) and v and v!='None' for v in ids) and len(set(ids))==len(ids):
                        identity=candidate;break
                for i,v in enumerate(value):
                    token=str(i)
                    if identity:
                        escaped=v[identity].replace('%','%25').replace('[','%5B').replace(']','%5D')
                        token='@'+identity+'='+escaped
                    walk(v,f'{context}[{i}]',row_id,f'{field_path}[{token}]',field)
            elif isinstance(value,str) and value.strip():
                allowed=field in TEXT_FIELDS or (field=='DisplayName' and not any(x in path.name for x in ['People','Address']))
                if field in NEVER_FIELDS:allowed=False
                # Timeline notes are displayed in the person inspector. Other Notes
                # fields remain internal/editorial and must not be translated.
                if table == 'DT_TMOP_People' and re.fullmatch(r'Timeline\[[^\]]+\]\.Notes', field_path):
                    allowed=True
                if allowed:
                    source=source_text(value)
                    if source and source not in ('None','none'):
                        e=add('TMOP.Table',table_key(table,row_id,field_path),source,context)
                        e.update(table=table,row=row_id,field=field_path)
                        m=MACRO.fullmatch(value.strip())
                        if m and decode_cpp(m[1]) and decode_cpp(m[2]):e['native_ids']=[dict(namespace=decode_cpp(m[1]),key=decode_cpp(m[2]))]
                elif field not in NEVER_FIELDS and not field.endswith(('Id','Ids','Path','Class','Asset','Time','Name')):
                    skipped[field]=skipped.get(field,0)+1
        row_ids=set()
        for row in data:
            row_id=row.get('Name') if isinstance(row,dict) else None
            if not isinstance(row_id,str) or not row_id or row_id in row_ids:
                raise ValueError(f'{path}: missing/duplicate stable row Name: {row_id}')
            row_ids.add(row_id)
            if spawn_only and not spawn_candidate(table,row):
                excluded_rows.append({'table':table,'row':row_id,'reason':'not enabled with a usable initial-placement/spawn action'})
                continue
            selected_count+=1
            walk(row,f'{path.relative_to(root)}:{row_id}',row_id)
        table_record['selected_rows']=selected_count
    return [entries[k] for k in sorted(entries)],dict(spawn_only=spawn_only,excluded_rows=excluded_rows,tables=table_info,remaining_code_conversions=audit,unclassified_string_fields=skipped)

def merge(entries,old):
    previous={(e['namespace'],e['key']):e for e in old.get('obsolete_entries',[])}
    previous.update({(e['namespace'],e['key']):e for e in old.get('entries',[])})
    legacy={e['key']:e for e in old.get('entries',[]) if e['namespace']=='TMOP.Source'}
    by_context={}
    for p in old.get('entries',[]):
        for c in p.get('contexts',[]):by_context.setdefault(c,[]).append(p)
    for e in entries:
        before=previous.get((e['namespace'],e['key']))
        migrated=False
        if before is None and e['namespace']=='TMOP.Table':
            before=legacy.get(source_key(e['source']));migrated=before is not None
        if before is None:
            candidates=[p for c in e.get('contexts',[]) for p in by_context.get(c,[]) if p['namespace']=='TMOP.Source']
            if len(candidates)==1:before=candidates[0];migrated=True
        if not before:continue
        if before['source']==e['source']:
            e['translation']=before.get('translation','')
            e['status']=before.get('status','draft' if e['translation'] else 'untranslated')
            # A source-only translation has never been reviewed in this row's context.
            if migrated and e['translation']:e['status']='draft'
            for field in ('previous_translation','previous_source','translator_note',
                          'machine_candidate','review_issues'):
                if field in before:e[field]=before[field]
        else:
            e['status']='needs_review'
            e['previous_source']=before['source']
            e['previous_translation']=before.get('translation') or before.get('previous_translation','')
            if 'translator_note' in before:e['translator_note']=before['translator_note']
    return entries

def compile_pack(pack):
    errors=validate(pack)
    if errors:raise ValueError('\n'.join(errors))
    if pack.get('schema_version')!=2:raise ValueError('Re-export/merge the legacy catalogue before compilation')
    compiled=[]
    for entry in pack['entries']:
        e={k:v for k,v in entry.items() if k in {
            'namespace','key','source','source_hash','translation','status',
            'table','row','field','native_ids'}}
        # Retain empty rows too: they prevent ambiguous source-only fallback from
        # borrowing another row's reviewed translation.
        if e['status']!='reviewed':e['translation']=''
        compiled.append(e)
    return dict(schema_version=2,language='en',native_language='sv',entries=compiled)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    sub=parser.add_subparsers(dest='command',required=True)
    ex=sub.add_parser('export');ex.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2]);ex.add_argument('--tables',type=Path,nargs='*',default=[]);ex.add_argument('--output',type=Path,required=True);ex.add_argument('--merge',type=Path);ex.add_argument('--table-list',type=Path);ex.add_argument('--audit',type=Path)
    ex.add_argument('--spawn-only',action='store_true',help='Only enabled spawn candidates in People/Vehicles; other tables unchanged')
    ex.add_argument('--latest-dated',action='store_true',help='Resolve latest MM_DD folder separately for each named input table')
    co=sub.add_parser('compile');co.add_argument('pack',type=Path);co.add_argument('--output',type=Path,required=True)
    va=sub.add_parser('validate');va.add_argument('pack',type=Path)
    args=parser.parse_args()
    if args.command=='compile':
        pack=compile_pack(read_json(args.pack))
        encoded=json.dumps(pack,ensure_ascii=False,separators=(',',':'))+'\n'
        if len(encoded.encode('utf-8'))>32*1024*1024:raise SystemExit('Runtime pack exceeds 32 MiB')
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(encoded,encoding='utf-8')
        print('Compiled reviewed translations only');return
    if args.command=='validate':
        errors=validate(read_json(args.pack))
        if errors:raise SystemExit('\n'.join(errors))
        print('Valid language pack');return
    tables=args.tables + ([Path(p) for p in read_json(args.table_list)] if args.table_list else [])
    if args.latest_dated:tables=latest_dated_tables(args.root.resolve(),tables)
    entries,audit=collect(args.root.resolve(),tables,args.spawn_only)
    old=read_json(args.merge) if args.merge else {}
    if args.merge:
        errors=validate(old)
        if errors:raise SystemExit('\n'.join(errors))
        entries=merge(entries,old)
    pack=dict(schema_version=2,language='en',native_language='sv',entries=entries)
    active={(e['namespace'],e['key']) for e in entries}
    historical={(e['namespace'],e['key']):e for e in old.get('obsolete_entries',[])+old.get('entries',[])}
    pack['obsolete_entries']=[e for identity,e in historical.items() if identity not in active]
    errors=validate(pack)
    if errors:raise SystemExit('\n'.join(errors))
    args.output.parent.mkdir(parents=True,exist_ok=True)
    encoded=json.dumps(pack,ensure_ascii=False,indent=2)+'\n'
    args.output.write_text(encoded,encoding='utf-8')
    if args.audit:args.audit.write_text(json.dumps(audit,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f'{len(entries)} unique entries, {sum(len(e["source"].split()) for e in entries)} source words; no translations generated.')
if __name__=='__main__':main()
