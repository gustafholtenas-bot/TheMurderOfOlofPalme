"""Select the current People DataTable in Content Browser; run outside Play."""
import json
import unreal
selected=[x for x in unreal.EditorUtilityLibrary.get_selected_assets() if isinstance(x,unreal.DataTable)]
if len(selected)!=1:raise RuntimeError('Select only DT_TMOP_People first.')
table=selected[0]
original=unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
rows=json.loads(original)
ids={'SYSTEM_TEST_DRIVER','SYSTEM_TEST_PASSENGER'}
found=set()
for row in rows:
 eid=row.get('EntityId')
 if eid not in ids:continue
 lines=[x for x in row.get('AutomaticSpeech',[]) if x.get('LineId')==eid+'_ARRIVED']
 if len(lines)!=1:raise RuntimeError('Expected one second test speech line for '+eid)
 line=lines[0]
 line.update(Time={'Hour':23,'Minute':0,'Second':35},TimingMode='Absolute',SharedEventId='None',OffsetSeconds=0,DisplayDurationOverrideSeconds=12,Text='[BILTEST] '+('Föraren' if eid.endswith('DRIVER') else 'Passageraren')+': syns min pratbubbla medan jag sitter i bilen?',Notes='Visuellt biltest 23:00:35 efter planerad ombordstigning 23:00:15. Inte en bekräftelse på lyckad ombordstigning.')
 found.add(eid)
if found!=ids:raise RuntimeError('Both system-test people must exist; nothing changed.')
if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table,json.dumps(rows,ensure_ascii=False)):
 unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table,original)
 raise RuntimeError('Import failed; original restored.')
unreal.log('Second speech moved to 23:00:35 for both test people, duration 12 seconds. Save People and restart Play.')
