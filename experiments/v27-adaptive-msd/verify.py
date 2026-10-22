#!/usr/bin/env python3
"""Frozen matrix identity, oracle and chronological-trial validator + paired gates."""
import argparse,csv,math,statistics
from collections import defaultdict
from pathlib import Path
MODES={
 'entropy':['uniform64','random32','random16','extremes','sparse16','sparse16alt','contig16','split16','sparse17','sparse20','sparse23','sparse24'],
 'cardinality':['K16','K128','K1024','K4096','K8192','K16384','K32768','K40960','K65536','K73728','K81920','K131072'],
 'ordered':['sorted','reversed','rotated','nearly','local','equal','organpipe','sawtooth','alternating']}
SEEDS=[708003,708007,708011,708017,708023]
BASE=['RUNE_v27','RUNE_v17','RUNE_v24','RUNE_v26b','RUNE27_byte128_WORKER','VQSort','IPS4o_seq','pdqsort','spreadsort','std_sort']
def verify(path,group,n,isa):
    rows=list(csv.DictReader(Path(path).open(newline='')))
    algos=BASE+(['x86_simd_sort','CAFS_pdq'] if isa=='x86_64' else [])
    expected={(m,n,s+i*131071,a) for i,m in enumerate(MODES[group]) for s in SEEDS for a in algos}
    observed={}
    for row in rows:
        key=(row['workload'],int(row['n']),int(row['seed']),row['algorithm'])
        if key not in expected or key in observed:raise ValueError(f'unexpected/duplicate cell: {key}')
        trials=[float(row[f'trial{i}_ms']) for i in range(5)]
        if any(not math.isfinite(v) or v<=0 for v in trials):raise ValueError('invalid trial')
        for field,want in [('median_ms',statistics.median(trials)),('min_ms',min(trials)),('max_ms',max(trials))]:
            if not math.isclose(float(row[field]),want,abs_tol=2e-9):raise ValueError(f'{field} mismatch')
        if row['group']!=group or row['correct']!='true' or row['repetitions']!='5':raise ValueError('invalid oracle/count/group')
        if any(row[f] not in ('0','1') for f in ('bmi2_available','simd_available','compact_selected')):raise ValueError('invalid capability')
        if row['algorithm']!='RUNE_v27' and row['compact_selected']!='0':raise ValueError('false route')
        observed[key]=row
    if set(observed)!=expected:raise ValueError(f'missing cells: {len(expected-set(observed))}')
    return rows

def summarize(rows,output,isa):
    by=defaultdict(dict)
    for r in rows:by[r['workload'],int(r['n']),r['algorithm']][int(r['seed'])]=r
    summary=[]
    for (mode,n,algo),cases in sorted(by.items()):
        base=by[mode,n,'RUNE_v17'];cand=by[mode,n,'RUNE_v27']
        ratios=[float(cases[s]['median_ms'])/float(base[s]['median_ms']) for s in base]
        vs_candidate=[float(cand[s]['median_ms'])/float(cases[s]['median_ms']) for s in base]
        summary.append(dict(isa=isa,workload=mode,n=n,algorithm=algo,category='worker' if algo.endswith('_WORKER') else 'public',
            median_ms=statistics.median(float(r['median_ms']) for r in cases.values()),
            median_paired_ratio_to_v17=statistics.median(ratios),
            candidate_paired_ratio=statistics.median(vs_candidate),
            candidate_seed_wins=sum(v<1 for v in vs_candidate),
            compact_seeds=sum(r['compact_selected']=='1' for r in cand.values()),
            regression_guard=('PASS' if statistics.median(ratios)<=1.05 else 'FAIL') if algo=='RUNE_v27' else 'N/A'))
    with Path(output).open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(summary[0]));w.writeheader();w.writerows(summary)
    failed=[r for r in summary if r['regression_guard']=='FAIL']
    print(f'{isa}: {len(rows)} complete observations, {len(failed)} regression guard failures')
    for r in failed:print('GUARD FAIL',r['workload'],r['n'],r['median_paired_ratio_to_v17'])
    # Performance gates report failures; they must never hide negative evidence or alter correctness CI.
    for r in summary:
        if r['algorithm']=='RUNE_v27' and r['workload'] in ('sparse17','sparse20') and r['n']>=1000000:
            print('TARGET',r['workload'],r['n'],'ratio',r['median_paired_ratio_to_v17'],'selected',r['compact_seeds'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('group',choices=MODES);p.add_argument('isa',choices=['x86_64','aarch64']);p.add_argument('directory');args=p.parse_args()
    rows=[]
    for n in (100000,1000000,4000000):rows+=verify(Path(args.directory)/f'{args.group}_{n}.csv',args.group,n,args.isa)
    summarize(rows,Path(args.directory)/'summary.csv',args.isa)
