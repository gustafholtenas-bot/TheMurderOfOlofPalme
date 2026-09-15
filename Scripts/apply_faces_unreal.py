"""Select the live People and Appearance DataTables, then execute this script.
Face assignments, bespoke catalog rows, and the approved Alicia name are changed.
"""
import json
from pathlib import Path
import unreal

root = Path(__file__).resolve().parent
mapping = json.loads((root / 'FACE_MAPPING.json').read_text(encoding='utf-8'))
catalog_rows = json.loads((root / 'DT_TMOP_AppearanceAssets.json').read_text(encoding='utf-16'))
known_ids = {m['CatalogId'] for m in mapping}
selected = [a for a in unreal.EditorUtilityLibrary.get_selected_assets() if isinstance(a, unreal.DataTable)]
tables = {}
for table in selected:
    rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table))
    if rows and 'EntityId' in rows[0] and 'AppearanceProfile' in rows[0]:
        tables['people'] = (table, rows)
    elif rows and 'CatalogId' in rows[0] and 'PartType' in rows[0]:
        tables['assets'] = (table, rows)
if set(tables) != {'people', 'assets'}:
    raise RuntimeError('Select DT_TMOP_People and DT_TMOP_AppearanceAssets in Content Browser first.')
people_table, people = tables['people']
assets_table, assets = tables['assets']
by_id = {p['EntityId']: p for p in people}
by_name = {a['Name']: a for a in assets}
# Validate all bespoke meshes before touching either table.
for m in mapping:
    mesh = unreal.load_asset(m['Mesh'])
    if not isinstance(mesh, unreal.SkeletalMesh):
        raise RuntimeError('Missing SkeletalMesh: ' + m['Mesh'])
for row in catalog_rows:
    if row['CatalogId'].startswith('FACE_STANDARD_'):
        if not isinstance(unreal.load_asset(row['Mesh']), unreal.SkeletalMesh):
            raise RuntimeError('Missing standard SkeletalMesh: ' + row['Mesh'])
    if row['CatalogId'] in known_ids or row['CatalogId'].startswith('FACE_STANDARD_'):
        by_name[row['Name']] = row
missing_people = []
for m in mapping:
    person = by_id.get(m['EntityId'])
    if person is None:
        missing_people.append(m['EntityId'])
        continue
    face = person['AppearanceProfile']['Face']
    face['CatalogId'] = m['CatalogId']
    face['MeshOverride'] = 'None'
    face['StaticMeshOverride'] = 'None'
    face['MaterialOverride'] = 'None'
    person['AppearanceProfile']['UnknownFaceCatalogId'] = 'UNKNOWN_FACE_OBSCURED'
for person in people:
    ap = person['AppearanceProfile']
    if ap['Face']['CatalogId'] in ('FACE_MALE_GENERIC', 'FACE_FEMALE_GENERIC', 'FACE_UNISEX_GENERIC'):
        ap['Face']['CatalogId'] = 'None'
        ap['UnknownFaceCatalogId'] = 'UNKNOWN_FACE_OBSCURED'
# User-approved identification; preserve EntityId and all existing links.
alicia = by_id.get('FLICKA_1')
if alicia is not None:
    alicia['FullName'] = 'Alicia Appel'
    alicia['FirstName'] = 'Alicia'
    alicia['LastName'] = 'Appel'
    identification_note = 'Användaruppgift 2026-09-15: Alicia Appel var en av flickorna på Mon Chéri som talade med Victor Gunnarsson, enligt en tidningsuppgift. Befintlig simulationsrad FLICKA_1 har tilldelats namnet på användarens instruktion; kopplingen till just denna anonyma rad är en modelleringstilldelning. Tidning och datum ej angivna.'
    if identification_note not in alicia.get('Notes', ''):
        alicia['Notes'] = alicia.get('Notes', '') + '\n' + identification_note
missing_standard = []
for gender in ('MALE', 'FEMALE'):
    for age in (18, 30, 45, 65):
        cid = f'FACE_STANDARD_{gender}_{age}'
        row = by_name.get(cid)
        if not row or not isinstance(unreal.load_asset(row.get('Mesh', 'None')), unreal.SkeletalMesh):
            missing_standard.append(cid)
# Preserve newer local table content. Keep backups in memory for rollback.
backup_assets = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(assets_table)
backup_people = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(people_table)
try:
    for table, rows in ((assets_table, list(by_name.values())), (people_table, people)):
        if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json.dumps(rows, ensure_ascii=False)):
            raise RuntimeError('DataTable import failed: ' + table.get_name())
except Exception:
    unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(assets_table, backup_assets)
    unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(people_table, backup_people)
    raise
unreal.log('Face mappings applied. Save both DataTables after reviewing them.')
if missing_people:
    unreal.log_warning('Missing people: ' + ', '.join(missing_people))
if missing_standard:
    unreal.log_warning('Standard faces still missing: ' + ', '.join(missing_standard))
