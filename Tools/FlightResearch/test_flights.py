import copy
import datetime as dt
import json
import unittest
from build_flights import CONTENT, compile_catalog, local_to_utc, UTC

class FlightTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog=json.loads((CONTENT/'catalog.json').read_text(encoding='utf-8'))
        cls.output=compile_catalog(cls.catalog)

    def test_exact_window(self):
        x=self.output
        self.assertEqual(x['start_utc'],'1986-02-27T22:21:30Z')
        self.assertEqual(x['anchor_utc'],'1986-02-28T22:21:30Z')
        self.assertEqual(x['end_utc'],'1986-03-01T22:21:30Z')

    def test_weekdays_and_calendar(self):
        rows=[r for r in self.output['legs'] if r['airline']=='golden-air']
        self.assertEqual(len(rows),12)
        self.assertTrue(all(r['departure_local'].startswith('1986-02-28') for r in rows))
        karachi=[r for r in self.output['legs'] if r['airline']=='panam' and r['destination']=='KHI']
        self.assertEqual([r['departure_local'][:10] for r in karachi],['1986-02-28','1986-03-01'])
        los_angeles=[r for r in self.output['legs'] if r['airline']=='panam' and r['destination']=='LAX']
        self.assertEqual([r['departure_local'][:10] for r in los_angeles],['1986-02-27','1986-03-01'])
        # Thursday's flight is still airborne when the window starts at 22:21:30 UTC.
        self.assertLess(los_angeles[0]['departure_seconds'],0)
        self.assertGreater(los_angeles[0]['arrival_seconds'],0)

    def test_historical_airport_locations(self):
        airports={a['id']:a for a in self.output['airports']}
        self.assertAlmostEqual(airports['KSD-OLD']['lat'],59.36)
        self.assertAlmostEqual(airports['FBU']['lat'],59.895802)
        self.assertEqual(airports['YXD']['name'],'Edmonton Municipal')

    def test_date_line_and_next_day(self):
        row=next(r for r in self.output['legs'] if r['airline']=='air-nauru' and r['origin']=='PPG')
        self.assertEqual(row['departure_utc'],'1986-03-01T06:15:00Z')
        self.assertEqual(row['arrival_utc'],'1986-03-01T10:15:00Z')
        self.assertEqual(row['arrival_seconds']-row['departure_seconds'],4*3600)

    def test_overlap_instead_of_departure_only(self):
        rows=self.output['legs']
        self.assertTrue(any(r['departure_seconds']<0<r['arrival_seconds'] for r in rows))
        self.assertTrue(any(r['arrival_seconds']>172800>r['departure_seconds'] for r in rows))
        self.assertTrue(all(r['departure_seconds']<172800 and r['arrival_seconds']>0 for r in rows))

    def test_source_review_gates(self):
        c=copy.deepcopy(self.catalog); c['schedules'][0]['review_status']='candidate'
        ids={r['schedule'] for r in compile_catalog(c)['legs']}
        self.assertNotIn(c['schedules'][0]['id'],ids)
        c=copy.deepcopy(self.catalog); c['schedules'][0]['nonstop']=False
        with self.assertRaises(ValueError):compile_catalog(c)
        c=copy.deepcopy(self.catalog); c['schedules'][0]['valid_to']='1986-04-30'
        with self.assertRaises(ValueError):compile_catalog(c)

    def test_duplicate_and_missing_airports(self):
        c=copy.deepcopy(self.catalog); row=copy.deepcopy(c['schedules'][0]);row['id']+='-duplicate';c['schedules'].append(row)
        with self.assertRaises(ValueError):compile_catalog(c)
        c=copy.deepcopy(self.catalog);c['schedules'][0]['destination']='UNKNOWN'
        with self.assertRaises(ValueError):compile_catalog(c)

    def test_calendar_exceptions(self):
        c=copy.deepcopy(self.catalog);row=c['schedules'][0];row['excluded_dates']=['1986-02-28']
        self.assertFalse(any(f['schedule']==row['id'] for f in compile_catalog(c)['legs']))
        row['included_dates']=['1986-02-28']
        with self.assertRaises(ValueError):compile_catalog(c)

    def test_cancelled_and_confirmed_are_evidence_based(self):
        c=copy.deepcopy(self.catalog); s=c['schedules'][0]
        c['sources'].append(dict(id='test-log',title='Test evidence fixture',url='https://example.org/fixture',kind='movement_record',access='full_scan'))
        c['observations']=[dict(schedule=s['id'],departure_date='1986-02-28',review_status='reviewed',source='test-log',page='test',status='cancelled')]
        result=compile_catalog(c)
        self.assertFalse(any(f['schedule']==s['id'] for f in result['legs']))
        self.assertEqual(len(result['excluded']),1)
        o=c['observations'][0];o.update(status='confirmed',departure_utc='1986-02-28T06:25:00Z',arrival_utc='1986-02-28T07:00:00Z')
        f=next(f for f in compile_catalog(c)['legs'] if f['schedule']==s['id'])
        self.assertEqual(f['status'],'confirmed');self.assertEqual(f['source'],'test-log')
        c['sources'][-1]['kind']='timetable'
        with self.assertRaises(ValueError):compile_catalog(c)

    def test_dst_gap_fold_and_half_hour_zone(self):
        with self.assertRaises(ValueError):local_to_utc(dt.date(1986,3,30),'02:30','Europe/Stockholm')
        with self.assertRaises(ValueError):local_to_utc(dt.date(1986,9,28),'02:30','Europe/Stockholm')
        a=local_to_utc(dt.date(1986,9,28),'02:30','Europe/Stockholm',0)
        b=local_to_utc(dt.date(1986,9,28),'02:30','Europe/Stockholm',1)
        self.assertEqual((b-a).total_seconds(),3600)
        self.assertEqual(local_to_utc(dt.date(1986,2,28),'12:00','Asia/Kolkata'),dt.datetime(1986,2,28,6,30,tzinfo=UTC))

    def test_multistop_day_transition(self):
        rows=[f for f in self.output['legs'] if f['journey']=='lap/pz803@1986-02-28']
        self.assertEqual([f['origin'] for f in rows],['FRA','BRU','REC'])
        self.assertEqual([f['destination'] for f in rows],['BRU','REC','ASU'])
        self.assertTrue(all(a['arrival_seconds']<=b['departure_seconds'] for a,b in zip(rows,rows[1:])))
        c=copy.deepcopy(self.catalog)
        for s in c['schedules']:
            if s['airline']=='lap' and s['flight']=='803' and s['origin']=='REC':s['departure']='01:00'
        with self.assertRaises(ValueError):compile_catalog(c)

    def test_new_air_wisconsin_evidence_and_time_zones(self):
        conflict='air-wisconsin-2740-MKG-BTL-1536'
        self.assertEqual(next(s for s in self.catalog['schedules'] if s['id']==conflict)['review_status'],'candidate')
        self.assertFalse(any(f['schedule']==conflict for f in self.output['legs']))
        # Westbound arrival looks earlier on the local clock, but is 39 minutes later in UTC.
        saturday=next(f for f in self.output['legs'] if f['airline']=='air-wisconsin' and f['flight']=='2773' and f['origin']=='BEH' and f['departure_local'].startswith('1986-03-01'))
        self.assertEqual(saturday['arrival_seconds']-saturday['departure_seconds'],39*60)
        friday=[f for f in self.output['legs'] if f['airline']=='air-wisconsin' and f['origin']=='BTL' and f['destination']=='BEH' and f['departure_local'].startswith('1986-02-28')]
        self.assertEqual([f['flight'] for f in friday],['2779'])

    def test_new_air_zimbabwe_overnight_services(self):
        row=next(f for f in self.output['legs'] if f['airline']=='air-zimbabwe' and f['flight']=='748')
        self.assertEqual(row['departure_local'],'1986-02-28T22:35+02:00')
        self.assertEqual(row['arrival_local'],'1986-03-01T00:25+02:00')
        self.assertEqual(row['arrival_seconds']-row['departure_seconds'],110*60)
        ba=[f for f in self.output['legs'] if f['airline']=='british-airways' and f['flight']=='052']
        self.assertEqual(len(ba),2)
        self.assertLess(ba[0]['departure_seconds'],0)
        self.assertGreater(ba[-1]['arrival_seconds'],172800)

    def test_month_only_source_date(self):
        c=copy.deepcopy(self.catalog)
        source=next(s for s in c['sources'] if s['id']=='golden-19860327')
        source['valid_from']=None; source['valid_to']=None; source['effective_from_month']='1985-11'
        compile_catalog(c)
        source['effective_from_month']='1986-02'
        with self.assertRaises(ValueError):compile_catalog(c)
        source['valid_from']='1986-02-01'
        compile_catalog(c)
        source['effective_from_month']='1985-13'
        with self.assertRaises(ValueError):compile_catalog(c)

    def test_localized_content_is_current(self):
        def walk(value):
            if isinstance(value,dict):
                if 'sv' in value:
                    self.assertIn('en',value);self.assertEqual(value['en_source'],value['sv'])
                    self.assertTrue(not value['sv'] or value['en'])
                for v in value.values():walk(v)
            elif isinstance(value,list):
                for v in value:walk(v)
        walk(self.catalog)

if __name__=='__main__':unittest.main()
