import copy
import json
import tempfile
import unittest
from pathlib import Path
from tmop_localization import arguments,collect,merge,read_json,source_key,source_text,validate,table_key,compile_pack

class LocalizationTests(unittest.TestCase):
    def entry(self,source='Hej {0}',translation='Hello {0}'):
        return dict(namespace='TMOP.Source',key=source_key(source),source=source,translation=translation,contexts=['People:A.Text'])
    def pack(self,*entries):return dict(schema_version=1,language='en',entries=list(entries))
    def test_source_exact_utf8(self):
        self.assertNotEqual(source_key('År'),source_key('Ar'))
        self.assertNotEqual(source_key('Hej'),source_key('Hej '))
    def test_imported_unreal_text(self):
        self.assertEqual(source_text('NSLOCTEXT("Table [GUID]", "Row_Text", "Hej\\nÅr")'),'Hej\nÅr')
        self.assertEqual(source_text('INVTEXT("Sveavägen")'),'')
    def test_validation(self):
        self.assertEqual(validate(self.pack(self.entry())),[])
        self.assertTrue(validate(self.pack(self.entry(translation='Hello'))))
        self.assertTrue(validate(self.pack(self.entry(),self.entry())))
        bad=self.entry();bad['key']='wrong';self.assertTrue(validate(self.pack(bad)))
        self.assertEqual(validate(self.pack(self.entry(translation=''))),[])
        self.assertEqual(arguments('`{literal`} {0}'),['{0}'])
    def test_merge_preserves_and_invalidates(self):
        old=self.entry();new=self.entry(translation='')
        self.assertEqual(merge([new],self.pack(old))[0]['translation'],'Hello {0}')
        changed=self.entry('Hej igen {0}','')
        result=merge([changed],self.pack(old))[0]
        self.assertEqual(result['translation'],'')
        self.assertEqual(result['previous_translation'],'Hello {0}')
        self.assertEqual(result['status'],'needs_review')
    def test_table_export_protects_game_data_and_separates_contexts(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);table=root/'DT_TMOP_People.json'
            rows=[dict(Name='PERSON_1',FullName='Olof Palme',EntityId='PERSON_1',Occupation='Författare',Timeline=[dict(Text='Hej {0}',AnchorId='Sveavagen_1')]),dict(Name='PERSON_2',Occupation='Författare')]
            raw=json.dumps(rows,ensure_ascii=False).encode('utf-16');table.write_bytes(raw)
            entries,audit=collect(root,[table])
            self.assertEqual({e['source'] for e in entries},{'Författare','Hej {0}'})
            self.assertEqual(len([e for e in entries if e['source']=='Författare']),2)
            self.assertEqual(len({e['key'] for e in entries}),3)
            self.assertEqual(table.read_bytes(),raw)
            self.assertEqual(read_json(table),rows)
    def test_conflicting_code_identity_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);p=root/'Plugins/TMOPEngine/Source/TMOPEngine/a.cpp';p.parent.mkdir(parents=True)
            p.write_text('NSLOCTEXT("TMOP", "key", "A")\nNSLOCTEXT("TMOP", "key", "B")')
            with self.assertRaises(ValueError):collect(root,[])

    def test_only_person_timeline_notes_are_exported(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);table=root/'DT_TMOP_People.json'
            table.write_text(json.dumps([dict(Name='A', Notes='Internal note',
                Timeline=[dict(EntryId='E1', Notes='Han väntar.')],
                Other=[dict(Notes='Do not translate')])]))
            entries,_=collect(root,[table])
            self.assertEqual([(e['field'], e['source']) for e in entries],
                             [('Timeline[@EntryId=E1].Notes', 'Han väntar.')])
            table.rename(root/'DT_TMOP_Other.json')
            entries,_=collect(root,[root/'DT_TMOP_Other.json'])
            self.assertEqual(entries,[])

    def test_intro_export_uses_actual_asset_identity(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);table=root/'DT_TMOP_introcards.json'
            table.write_text(json.dumps([dict(Name='A',Body='Inledning')]))
            entries,_=collect(root,[table])
            self.assertEqual(entries[0]['table'],'DT_TMOP_IntroCards')
            self.assertEqual(entries[0]['key'],'DT_TMOP_IntroCards/A/Body')

    def stable_entry(self,row='A',source='Hej {0}',translation='Hello {0}',status='reviewed'):
        return dict(namespace='TMOP.Table',key=table_key('DT_TMOP_Test',row,'Body'),
            table='DT_TMOP_Test',row=row,field='Body',source=source,source_hash=source_key(source),
            translation=translation,status=status)
    def stable_pack(self,*entries):return dict(schema_version=2,language='en',entries=list(entries))
    def test_stable_merge_changed_source_and_repeated_export(self):
        old=self.stable_entry()
        changed=self.stable_entry(source='Ny källa {0}',translation='',status='untranslated')
        result=merge([changed],self.stable_pack(old))[0]
        self.assertEqual(result['key'],old['key'])
        self.assertEqual(result['status'],'needs_review')
        self.assertEqual(result['previous_translation'],'Hello {0}')
        again=merge([self.stable_entry(source='Ny källa {0}',translation='',status='untranslated')],self.stable_pack(result))[0]
        self.assertEqual(again['status'],'needs_review')
        self.assertEqual(again['previous_translation'],'Hello {0}')
        self.assertEqual(again['translation'],'')

    def test_merge_retains_machine_review_evidence_only_for_same_source(self):
        old=self.stable_entry(translation='', status='needs_review')
        old.update(machine_candidate='Candidate with a damaged token',
                   review_issues=['protected_token_mismatch'])
        same=merge([self.stable_entry(translation='',status='untranslated')],self.stable_pack(old))[0]
        self.assertEqual(same['machine_candidate'],old['machine_candidate'])
        self.assertEqual(same['review_issues'],old['review_issues'])
        changed=merge([self.stable_entry(source='Ny text',translation='',status='untranslated')],self.stable_pack(old))[0]
        self.assertNotIn('machine_candidate',changed)
        self.assertNotIn('review_issues',changed)
    def test_legacy_migration_requires_context_review(self):
        result=merge([self.stable_entry(translation='',status='untranslated')],self.pack(self.entry()))[0]
        self.assertEqual(result['translation'],'Hello {0}')
        self.assertEqual(result['status'],'draft')
        self.assertEqual(compile_pack(self.stable_pack(result))['entries'][0]['translation'],'')
    def test_only_reviewed_compiles_without_losing_empty_contexts(self):
        entries=[self.stable_entry(row=str(i),status=status) for i,status in enumerate(['reviewed','draft','needs_review','untranslated'])]
        result=compile_pack(self.stable_pack(*entries))
        self.assertEqual(len(result['entries']),4)
        self.assertEqual([e['translation'] for e in result['entries']],['Hello {0}','','',''])
        self.assertEqual(validate(result),[])
    def test_stable_validation(self):
        original=self.stable_entry()
        self.assertEqual(validate(self.stable_pack(original)),[])
        for field,value in [('source','Changed'),('row','Other'),('status','approved'),('native_ids',[{}]),('translation','')]:
            bad=copy.deepcopy(original);bad[field]=value
            self.assertTrue(validate(self.stable_pack(bad)),field)
    def test_archived_entry_is_restored(self):
        old=self.stable_pack();old['obsolete_entries']=[self.stable_entry()]
        new=self.stable_entry(translation='',status='untranslated')
        self.assertEqual(merge([new],old)[0]['translation'],'Hello {0}')
    def test_date_directory_and_row_order_do_not_change_keys(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            rows=[dict(Name='B',Body='Andra'),dict(Name='A',Body='Första')]
            paths=[root/'09_19/DT_TMOP_Test.json',root/'09_25/DT_TMOP_Test.json']
            for p in paths:p.parent.mkdir();p.write_text(json.dumps(rows));rows.reverse()
            a,_=collect(root,[paths[0]]);b,_=collect(root,[paths[1]])
            self.assertEqual([(e['key'],e['source']) for e in a],[(e['key'],e['source']) for e in b])
            with self.assertRaises(ValueError):collect(root,paths)
    def test_missing_or_duplicate_row_id_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);p=root/'DT_TMOP_Test.json'
            for rows in [[dict(Body='Hej')],[dict(Name='A',Body='Hej'),dict(Name='A',Body='Hej')]]:
                p.write_text(json.dumps(rows))
                with self.assertRaises(ValueError):collect(root,[p])
    def test_native_alias_and_nested_field_export(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);p=root/'DT_TMOP_Test.json'
            p.write_text(json.dumps([dict(Name='A',Items=[dict(Text='NSLOCTEXT("Native", "Key", "Hej")')])]))
            entries,_=collect(root,[p]);e=entries[0]
            self.assertEqual(e['key'],'DT_TMOP_Test/A/Items[0].Text')
            self.assertEqual(e['native_ids'],[dict(namespace='Native',key='Key')])
    def test_nested_segment_identity_survives_reorder(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);p=root/'DT_TMOP_Test.json'
            items=[dict(SegmentId='S1',Transcript='Första'),dict(SegmentId='S2',Transcript='Andra')]
            p.write_text(json.dumps([dict(Name='A',SpeechSegments=items)]))
            first,_=collect(root,[p]);items.reverse()
            p.write_text(json.dumps([dict(Name='A',SpeechSegments=items)]))
            second,_=collect(root,[p])
            self.assertEqual([(e['key'],e['source']) for e in first],[(e['key'],e['source']) for e in second])
            self.assertIn('SpeechSegments[@SegmentId=S1].Transcript',first[0]['key'])

    def test_key_escaping_is_unambiguous(self):
        self.assertNotEqual(table_key('A/B','C','D'),table_key('A','B/C','D'))
        self.assertEqual(table_key('A','B%/C','Body'),'A/B%25%2FC/Body')
    def test_runtime_field_allowlist_matches_exporter(self):
        from tmop_localization import TEXT_FIELDS
        import re
        path=Path(__file__).resolve().parents[2]/'Plugins/TMOPEngine/Source/TMOPEngine/Private/Localization/TMOPTableTextFields.inl'
        self.assertEqual(set(re.findall(r'TEXT\("([^"\n]+)"\)',path.read_text())),TEXT_FIELDS)

    def test_spawn_filter_respects_flags_ids_and_simulation_actions(self):
        from tmop_localization import spawn_candidate
        person=dict(EntityId='P',bSpawnInSimulation=True,Timeline=[dict(Action='InitialPlacement',Usage='Simulation')])
        self.assertTrue(spawn_candidate('DT_TMOP_People',person))
        for patch in [dict(bSpawnInSimulation=False),dict(EntityId='None'),dict(Timeline=[]),dict(Timeline=[dict(Action='Spawn',Usage='DocumentationOnly')]),dict(Timeline=[dict(Action='MoveToAnchor')])]:
            self.assertFalse(spawn_candidate('DT_TMOP_People',dict(person,**patch)))
        self.assertTrue(spawn_candidate('DT_TMOP_HistoricalVehicles',dict(VehicleId='V',bSpawnInSimulation=True,Timeline=[dict(Action='Spawn')])))
        self.assertFalse(spawn_candidate('DT_TMOP_HistoricalVehicles',dict(VehicleId='V',Timeline=[dict(Action='Spawn')])))
        self.assertTrue(spawn_candidate('DT_TMOP_MurderKnowledge',dict(Body='Text')))
    def test_spawn_filter_leaves_other_tables_unchanged(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);p=root/'DT_TMOP_People.json';k=root/'DT_TMOP_MurderKnowledge.json'
            p.write_text(json.dumps([dict(Name='A',EntityId='A',bSpawnInSimulation=True,Timeline=[dict(Action='Spawn')],Occupation='Yrke'),dict(Name='B',bSpawnInSimulation=False,Occupation='Arkiv')]))
            k.write_text(json.dumps([dict(Name='K',Body='Kunskap')]))
            entries,audit=collect(root,[p,k],spawn_only=True)
            self.assertEqual({e['source'] for e in entries},{'Yrke','Kunskap'})
            self.assertEqual(audit['tables'][0]['selected_rows'],1)
            self.assertEqual(audit['excluded_rows'][0]['row'],'B')
    def test_latest_date_is_per_table_not_last_folder(self):
        from tmop_localization import latest_dated_tables
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            for relative in ['DataTables/09_19/A.json','DataTables/09_21/A.json','DataTables/09_14/B.json','DataTables/09_25/A_IMPORT.json']:
                p=root/relative;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('[]')
            self.assertEqual(latest_dated_tables(root,[Path('A.json'),Path('B.json')]),[Path('DataTables/09_21/A.json'),Path('DataTables/09_14/B.json')])

if __name__=='__main__':unittest.main()
