# RUNE v22 — Wide-digit radix native cross-ISA verdict

**Decision: NO-GO for general integration or superiority, HOLD x86-only mechanism investigation (unpromoted). DO NOT MERGE.**

This experiment branches from the verified v20 diagnostic (head `d172516a691d82914b432a8d22bbaef55d828f91`), not rejected v21. Benchmarked source commit `c16ce4c431830f8fb8d90338d29c937974cad871`. The RUNE v17 production sorter/dispatcher, proprietary license and main branch remain untouched.

## Verification

- **All six GitHub CI jobs passed**, [specialized research run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39552710644) (native x86 and native ARM, both sanitizers, x86 no-BMI2), [push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39552710417), [PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39552757767).
- **600/600 measured full-array outputs** exactly matched `std::sort` oracle (300 each x86 and ARM), across 6 algorithms × 2 sizes × 5 modes × 5 seeds × 2 architectures. Differential tests on 328 more adversarial/small/large arrays per ISA (27 sizes × 12 modes + four million-scale variants), CMake/CTest passed, ASan/UBSan passed.
- GCC `-std=c++20 -O3 -march=native -DNDEBUG -pthread`; GitHub-hosted x86 AMD EPYC and ARM Neoverse-N2 virtual machines; 1 warmup + 3 randomized-order measured trials per algorithm; array copy/verification outside timing; exact independent oracle against each entire result.
- **Historic comparator pins, not current/latest:** Google Highway `c3407ccaaecffa735c911cae3971efa02263211d`; IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`. All source fetched at exact checked hashes by CI. No external code vendored.

## N=4M benchmark (median ms, lower better)

| ISA | Mode | Frozen direct fixed8 | Radix11 six-pass | Radix16 four-pass | Pinned VQSort | Median paired radix11/VQ / radix16/VQ |
|---|---|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 124.272 | 117.247 | 105.665 | 56.123 | 2.086× / 1.883× |
| x86_64 | random32 | 89.451 | 78.548 | 73.866 | 56.195 | 1.398× / 1.317× |
| x86_64 | K85000 | 96.869 | 84.274 | 100.006 | 56.755 | 1.487× / 1.760× |
| x86_64 | sorted | 125.303 | 113.232 | 105.966 | 51.083 | 2.217× / 2.075× |
| x86_64 | alternating | 63.812 | 55.564 | 50.031 | 3.244 | 17.015× / 15.535× |
| arm64 | uniform64 | 70.957 | 72.339 | 90.055 | 76.175 | 0.950× / 1.183× |
| arm64 | random32 | 93.331 | 75.378 | 83.912 | 76.109 | 0.990× / 1.103× |
| arm64 | K85000 | 71.979 | 66.816 | 76.725 | 62.132 | 1.079× / 1.233× |
| arm64 | sorted | 75.038 | 81.869 | 92.980 | 1.562 | 51.601× / 59.328× |
| arm64 | alternating | 103.630 | 73.533 | 69.025 | 3.484 | 21.132× / 19.899× |

Displayed milliseconds are median of five seed-level medians, ratio metrics are separately medians of *same-seed* timing ratios, not quotient of displayed latency columns. Cross-CPU absolute latency is not a controlled comparison.

### Primary x86 full-width uniform64

- Frozen fixed8 worker **124.272 ms**, radix11 **117.247 ms**, radix16 **105.665 ms**, pinned VQSort **56.123 ms**.
- Radix16 wins against frozen fixed8 in **5/5 seeds** with a median seed-paired ratio of **0.848×** (candidate/base: smaller is faster) — a bounded meaningful **~15.2% latency reduction** on this CPU/run.
- Radix11 is also faster than frozen worker in **5/5 seeds**, median ratio **0.941×**. Nevertheless **radix16 still 1.883× slower than pinned VQSort**, losing all five matched seeds. The primary superiority/closure criterion **FAILED**.
- At N=1M uniform64 x86, radix16 has the same direction (~22.41 vs 30.27ms worker) but VQSort remains ~12.87ms.

### ARM portability counterevidence

- For N=4M uniform64 on ARM: frozen worker **70.957 ms**, radix11 **72.339 ms**, radix16 **90.055 ms**. The x86-best four-pass radix16 **regresses on ARM**, with median paired ratio candidate/base **1.269×**. Thus unconditional substitution of the v17 worker would make the total sorter worse on important ARM distributions.
- The six-pass radix11 can beat the *historical* pinned VQSort on ARM uniform64 but does not improve frozen ARM v17 at 4M. This is a platform-sensitive trade-off, not an algorithm dominance claim.
- Sorted and alternating inputs in experimental kernels force full radix; existing production RUNE early-exits very quickly. These pathological controls cannot be presented as public RUNE end-to-end performance.

## Mechanism interpretation and limits

Pass reduction improves x86 bandwidth/scatter regime, but the larger 16-bit counter workspace penalizes ARM and still cannot match pinned VQSort. Neither radix width is novel; no broad state-of-art or release claim. We did **not** run the historical comparator at its newest release, different physical CPUs, or PMU/hardware counters. No evidence proves cache miss causality or cross-run CPU equivalence. The x86 number is suitable to design a follow-up **architecture-gated, fail-closed dispatcher trial** only, not to merge this research worker today. Do not reintroduce rejected v18 buffered scatter or v21 bucket+local comparison sorting unchanged.

## Exact raw evidence

- [x86 300 input-verified rows](x86_64_raw.csv)
- [ARM 300 input-verified rows](arm64_raw.csv)
- [20 architecture-size-mode paired groups](paired_summary.csv)
- [Reproducibility/provenance](provenance.json)

**Verdict: NO-GO for VQSort superiority or unconditional kernel replacement; research-only conditional x86 radix16 specialization hypothesis.**
