# Extended RuneSort internal + external SOTA comparison

Re-run PR #14's **33 exact workload generators and 3 sizes (100K/1M/4M)**
on native x86 and ARM runners with **3 original fixed seeds**, one
oracle-verified warmup, 3 interleaved timed calls per algorithm, all
checked exactly against an independently sorted std::sort control.

**Complete public sorters**: PR #14 Rune v5/v6/v7/v13/v14/ARM16; prior
PR #9 v10 (uninitialized scratch); v17, v24, v25, v26b; Google Highway
VQSort; IPS4o sequential; Boost pdqsort/spreadsort; standard C++
std::sort/stable_sort. CAFS+pdqsort on compatible x86 only.
**Standalone worker, not a public sorter**: PR #21 bucket-local radix7.
19 total on compatible x86, 18 on ARM.

Full rows expected: x86 5643, ARM 5346 = **10,989**; each gives all
three individual trial durations and verifies exact sorted output.
Median-of-seed-paired ratios, never different-hardware pooled ratios.
Upstream revisions: Highway 3a5844a43b095751a4870ac0b66940d71ab60c38;
IPS4o 8d5a97bb4c4048a1eef18b537f191a31462b051a;
CAFS 13488afef446ae88e7ad631ec6f862def896094a.
Boost installed version retained in Actions provenance.

Source snapshots copied *unaltered* from PR #9 head
c3ebdbd2703f7a6e351d47ea4875289888b18abe and PR #21 head
cd453ae45d0bb1f05e328f8a3f30e0b51da37448. These independent
proprietary research snapshots are benchmark-only; no original
algorithm, branch, LICENSE, or main modified.

**Research hold:** v24 previous PR #26 had AMD counterexample and x86
regression-guard failures; v26b lacks AVX512 performance qualification.
Virtual machine timing is not proof of generic SOTA. Jobs separated by
ISA and group may land on different host CPUs; never pool such absolute
latencies into universal rankings. Do not merge based on this experiment.
