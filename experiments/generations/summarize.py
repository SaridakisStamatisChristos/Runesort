#!/usr/bin/env python3
"""Fail closed for missing/duplicate rows, invalid or altered trials or wrong algorithm universe."""
import argparse,csv,math,statistics
from pathlib import Path
SEEDS=(20301001,20301011,20301021,20301031,20301041)
MODES=("uniform64","random32","K85000","sorted","alternating","reverse",
       "sparse16","almostsorted","sortedprefix","entropy_prefix")
SIZES=(1000000,4000000)
ALGORITHMS=('std_sort','RUNE5_full','RUNE6_full','RUNE7_full','RUNE13_full','RUNE14_full','RUNE16_full','RUNE17_full','RUNE24_full','RUNE25_full','RUNE26b_full','RUNE22_radix11_worker','RUNE22_radix16_worker','RUNE23_prefetch128_worker','RUNE26b_nibble_worker')
def main(files,isa,output):
    data={}
    capabilities=set()
    for f in files:
      with Path(f).open(newline='') as input:
       for row in csv.DictReader(input):
        key=(row["mode"],int(row["n"]),int(row["seed"]),row["algorithm"])
        if key in data: raise ValueError(f"duplicate row {key}")
        if row["correct"]!="true": raise ValueError(f"incorrect {key}")
        if row["simd_available"] not in ("0","1"): raise ValueError(f"invalid simd field: {key}")
        capabilities.add(row["simd_available"])
        times=[float(row[f"trial{i}_ms"]) for i in range(3)]
        if not all(math.isfinite(t) and t>0 for t in times): raise ValueError(f"invalid trials {key}")
        for field,actual in (("median_ms",statistics.median(times)),("min_ms",min(times)),("max_ms",max(times))):
         if not math.isclose(float(row[field]),actual,rel_tol=1e-13,abs_tol=1e-13):
          raise ValueError(f"raw/summary inconsistency {key}:{field}")
        data[key]=statistics.median(times)
    expected={(m,n,s,a) for m in MODES for n in SIZES for s in SEEDS for a in ALGORITHMS}
    if set(data)!=expected: raise ValueError(f"missing {len(expected-set(data))}, unexpected {len(set(data)-expected)}")
    if len(capabilities)!=1 or (isa=="aarch64" and capabilities!={"0"}):
      raise ValueError("invalid CPU capability")
    rows=[]
    for m in MODES:
     for n in SIZES:
      for a in ALGORITHMS:
       trials=[data[m,n,s,a] for s in SEEDS]
       ref=[data[m,n,s,"RUNE26b_full"] for s in SEEDS]
       ratios=[x/y for x,y in zip(trials,ref)]
       rows.append({"isa":isa,"simd_available":next(iter(capabilities)),
                    "mode":m,"n":n,"algorithm":a,"median_ms":statistics.median(trials),
                    "ratio_vs_v26b":statistics.median(ratios),
                    "wins_vs_v26b":sum(x<y for x,y in zip(trials,ref))})
    with Path(output).open("w",newline="") as o:
     writer=csv.DictWriter(o,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    print(f"{isa}: validated {len(data)} rows, {len(data)*3} measured sorts plus {len(data)} verified warmups")
    for n in SIZES:
     print(f"=== {isa}: uniform64 n={n} (public API and separately flagged workers) ===")
     for r in sorted((r for r in rows if r["mode"]=="uniform64" and r["n"]==n),key=lambda r:r["median_ms"]):
      print(f'{r["algorithm"]:29s} {r["median_ms"]:10.3f} ms  relative-to-v26b={r["ratio_vs_v26b"]:.3f}x')
if __name__=="__main__":
 p=argparse.ArgumentParser()
 p.add_argument("--isa",required=True);p.add_argument("--output",required=True)
 p.add_argument("csvs",nargs="+");a=p.parse_args()
 main(a.csvs,a.isa,a.output)
