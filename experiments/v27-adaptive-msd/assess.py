#!/usr/bin/env python3
"""Recompute six raw matrices and frozen gates without pooling host latencies."""
import argparse,csv,hashlib,importlib.util,json
from pathlib import Path
p=Path(__file__).with_name('verify.py')
spec=importlib.util.spec_from_file_location('v27_verify',p)
v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)
def assess(root,sha):
    reports=[];wins=[];failures=[];benefits=[];observations=0;hashes={}
    for isa,runner in [('x86_64','ubuntu-24.04'),('aarch64','ubuntu-24.04-arm')]:
        for group in v.MODES:
            directory=root/f'rune-v27-{runner}-{group}'
            provenance=(directory/'provenance.txt').read_text()
            if sha not in provenance.splitlines():raise ValueError('source provenance mismatch')
            cpu=next(line.split(':',1)[1].strip() for line in provenance.splitlines() if line.startswith('Model name:'))
            rows=[]
            for n in (100000,1000000,4000000):
                path=directory/f'{group}_{n}.csv'
                rows+=v.verify(path,group,n,isa)
                hashes[str(path.relative_to(root))]=hashlib.sha256(path.read_bytes()).hexdigest()
            observations+=len(rows)
            computed=directory/'recomputed.csv';v.summarize(rows,computed,isa)
            if computed.read_bytes()!=(directory/'summary.csv').read_bytes():raise ValueError('summary does not reproduce byte for byte')
            data=list(csv.DictReader(computed.open(newline='')))
            reports.append(dict(isa=isa,group=group,cpu=cpu,rows=len(rows),source=sha))
            cells={}
            for row in data:cells.setdefault((row['workload'],int(row['n'])),[]).append(row)
            for (mode,n),cell in cells.items():
                candidate=next(r for r in cell if r['algorithm']=='RUNE_v27')
                ratio=float(candidate['median_paired_ratio_to_v17'])
                if ratio>1.05:failures.append(dict(isa=isa,cpu=cpu,group=group,workload=mode,n=n,ratio=ratio))
                if isa=='x86_64' and mode in ('sparse17','sparse20') and n>=1000000:
                    base=next(r for r in cell if r['algorithm']=='RUNE_v17')
                    benefits.append(dict(workload=mode,n=n,ratio=ratio,
                        passed=ratio<=.90 and int(base['candidate_seed_wins'])==5 and int(candidate['compact_seeds'])==5))
                competitors=[r for r in cell if r['category']=='public' and r['algorithm']!='RUNE_v27']
                if all(float(r['candidate_paired_ratio'])<=.95 and int(r['candidate_seed_wins'])==5 for r in competitors):
                    wins.append(dict(isa=isa,cpu=cpu,group=group,workload=mode,n=n,
                        candidate_ms=float(candidate['median_ms']),comparators=[r['algorithm'] for r in competitors],
                        worst_paired_ratio=max(float(r['candidate_paired_ratio']) for r in competitors)))
    if len(benefits)!=4:raise ValueError('missing target cells')
    return dict(source=sha,observations=observations,timed_sorts=observations*5,verified_warmups=observations,
        host_experiments=reports,benefit_gates=benefits,regression_failures=failures,
        integration_gate='PASS' if not failures and all(b['passed'] for b in benefits) else 'HOLD',
        comparative_winning_cells=wins,scope='Only listed CPU/workload/size/pinned-competitor cells; not universal SOTA.',raw_sha256=hashes)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('directory',type=Path);parser.add_argument('source_sha');parser.add_argument('output',type=Path)
    args=parser.parse_args();result=assess(args.directory,args.source_sha)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print('Integration:',result['integration_gate'],'comparative winning cells:',len(result['comparative_winning_cells']))
