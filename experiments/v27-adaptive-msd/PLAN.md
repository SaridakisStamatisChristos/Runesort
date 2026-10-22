# v27 contract and frozen qualification protocol — 23 October 2026

Objective: improve full end-to-end single-threaded ascending uint64 latency on
compact-domain inputs without material regression against frozen v17. Do not
replace the random64 fallback with an unqualified prototype. Preserve existing
research branches, negative results and proprietary license.

Public candidate: exact full-array mask certification and compact uint32 count
reconstruction before v17. Primary improvement cells: sparse17 and sparse20,
N=1M and 4M, on x86 with runtime BMI2. Acceptance requires candidate/v17 median
seed-paired latency <=0.90 and 5/5 seed wins in all four cells. Regression guard:
all 99 public candidate workload/size cells per ISA <=1.05 versus v17. Ordered
and unsupported sparse-domain paths remain guarded. Runner noise may motivate
an independently repeated frozen run; it does not erase a failed cell.

Comparative SOTA gate: every compared algorithm is a full public sorter with
the same uint64 ascending semantics. A claim is restricted to its actual CPU,
N, workload and pinned competitors. For a claimed winning cell, require
candidate/competitor <=0.95 and 5/5 seed wins against *every* public competitor.
A worker win cannot satisfy this gate. A universal SOTA verdict is not inferred
from a finite input matrix. No arbitrary comparator delegation is added to RUNE.

Matrix: retain the complete 33-mode PR #30 generator and the three sizes 100K,
1M, 4M. Five newly specified seeds [708003,708007,708011,708017,708023], plus
the existing mode-index offset 131071. These seeds are frozen before the first
v27 qualification run and differ from prior gauntlet seeds. One exact checked
warmup plus five chronological raw timed trials; deterministic shuffled
algorithm order per trial; full independent std::sort oracle after every call;
input construction, input copies and verification excluded from timing;
all sorting allocations included. `std::sort` is separately timed as a baseline.

Each group/ISA is a separate host experiment. CPU/compiler, native flags,
source SHA, comparator pins and Boost package version are logged. Never pool
absolute times across hosts or architectures. The frozen validator rejects
missing, duplicate, nonfinite, incorrectly summarized and non-oracle rows.
Negative performance gates are reported, not converted into correctness passes.

Public competitors: RUNE17, RUNE24, RUNE26b, Google Highway VQSort, sequential
IPS4o, Boost pdqsort/spreadsort, std::sort, CAFS+pdqsort on x86, and the newly
added x86-simd-sort baseline on supported x86. A separately labeled byte128 MSD
worker tests the alternate mechanism and is excluded from superiority counts.

Pins verified against upstream HEAD when this protocol was written:
- Highway: 8fd66948f4555b022328b42120dbce9315847493 (21 October 2026)
- numpy/x86-simd-sort: 6e059b14975df2e3c74260ec905167a1be5a34ca (9 October 2026)
- IPS4o: 8d5a97bb4c4048a1eef18b537f191a31462b051a
- CAFS: 13488afef446ae88e7ad631ec6f862def896094a

Prior-art scope: counting sort, order-preserving bit extraction, MSD radix and
SIMD sorting networks are established mechanisms. This is an engineering
qualification campaign. Current primary comparator documentation:
https://github.com/google/highway/tree/master/hwy/contrib/sort
https://github.com/numpy/x86-simd-sort
https://github.com/SaschaWitt/ips4o
https://github.com/kexibq-official/cafs-lib

Local exploratory MSD pilots (byte/nibble, 32/64/128 leaves, buffered and
streaming-store variants) were highly variable, some concurrent with builds.
They are not qualification evidence and selected no production worker. Their
negative direction motivates preserving v17 as fallback and qualifying the
compact-domain change separately. CI must prove its own full exact source.

## v27c reconstruction follow-up, frozen before its first CI run

The first two source revisions retain their original seeds and results in Git
and Actions. They do not qualify this revision. A separate local mechanism
pilot (seed 707601, two sizes, three bit domains, five alternating-order
measurements) identified sparse-table reconstruction as avoidable overhead.
AVX-512 enumerates nonzero 16-counter blocks and writes up to eight duplicates
with masked stores. Average multiplicity >8 retains scalar/compiler-vectorized
fill. Contiguous paths, domain limits, benefit and regression thresholds, sizes,
workloads and competitors are unchanged. All five seeds above are fresh for
v27c; the pilot and earlier v27 measurements are development data. Require new
source-specific correctness, ISA-disabled, sanitizer and full matrix results.
`RUNE27_DISABLE_SIMD` preserves BMI2 counting while ablating reconstruction.
