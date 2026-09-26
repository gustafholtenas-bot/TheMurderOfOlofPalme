"""Resumable Internet Archive discovery by country and airline. No automatic route import.

The catalog is a growing research queue, not a complete list of 1986 operators.
Search results can be covers, later editions, duplicates or unrelated records.
"""
import argparse
import concurrent.futures
import datetime as dt
import json
from pathlib import Path
import urllib.parse
import urllib.request
from build_flights import CONTENT

def discover(airline):
    names = [airline['name']] + airline.get('search_aliases', [])
    # Escape Archive's Lucene query syntax inside exact phrases.
    phrases = ['"' + name.replace('\\','\\\\').replace('"','\\"') + '"' for name in names]
    query = '(' + ' OR '.join(phrases) + ') AND (1985 OR 1986) AND (timetable OR timetables OR schedule OR schedules)'
    params = {'q':query, 'output':'json', 'rows':200, 'fl[]':['identifier','title','date']}
    url = 'https://archive.org/advancedsearch.php?' + urllib.parse.urlencode(params,doseq=True)
    row = dict(airline=airline['id'], countries=airline['countries'], query=query, url=url,
               checked_at=dt.datetime.now(dt.timezone.utc).isoformat(timespec='seconds'))
    try:
        with urllib.request.urlopen(url, timeout=25) as response:
            result=json.load(response)['response']
        row.update(status='searched', total=result['numFound'], truncated=result['numFound']>200,
                   candidates=[dict(identifier=x['identifier'], title=x.get('title',''), date=x.get('date'),
                                    url='https://archive.org/details/'+x['identifier'], review_status='unreviewed') for x in result['docs']])
    except Exception as error:
        row.update(status='error', error=str(error))
    return row

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--catalog',type=Path,default=CONTENT/'catalog.json')
    parser.add_argument('--country',help='Historical country ID; omit to process the whole registered queue')
    parser.add_argument('--output',type=Path,default=Path(__file__).parent/'research/archive_airline_searches.json')
    parser.add_argument('--refresh',action='store_true')
    args=parser.parse_args()
    catalog=json.loads(args.catalog.read_text(encoding='utf-8'))
    old=json.loads(args.output.read_text(encoding='utf-8')) if args.output.exists() else {'schema':1,'searches':[]}
    records={x['airline']:x for x in old['searches']}
    queue=[x for x in catalog['airlines'] if (not args.country or args.country in x['countries'])
           and (args.refresh or records.get(x['id'],{}).get('status')!='searched')]
    args.output.parent.mkdir(parents=True,exist_ok=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        for result in pool.map(discover,queue):
            records[result['airline']]=result
            temporary=args.output.with_suffix('.tmp')
            temporary.write_text(json.dumps({'schema':1,'coverage_complete':False,'searches':list(records.values())},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
            temporary.replace(args.output)
            print(result['airline'],result['status'],result.get('total',''),flush=True)
    print('Saved',len(records),'airline search records to',args.output)

if __name__=='__main__':
    main()
