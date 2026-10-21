# v25: correctness passed; SIMD performance not qualified

Measured source: `986253c5764c5b242c955addee5f0555aed61725`.
[Draft PR #27](https://github.com/SaridakisStamatisChristos/Runesort/pull/27).
[Qualification run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39602570271).

**HOLD integration.** The implementation adds AVX-512 compress-store MSD
partitioning with 8/16/32-key bitonic leaves, a std::sort-leaf ablation, and
runtime CPU dispatch. It passes exact correctness checks. Both full CI matrices
exercise fallback because neither runner exposes AVX-512. The supplementary
local SIMD pilot is contended and extremely noisy; it does not qualify performance. No SOTA,
novelty, universal speedup, or production-readiness claim is supported.

## Predeclared gates

| Gate | Result | Meaning |
|---|---|---|
| Correctness/build | PASS | All six CI jobs passed; both native suites passed 30/30 tests |
| Regression guard | PASS | 78/78 predeclared cells meet candidate/v17 <=1.05 |
| SIMD end-to-end benefit | NOT QUALIFIED | CI x86 runner has no AVX-512 |
| Network contribution | NOT QUALIFIED | CI cannot execute network leaves |
| Current pinned VQSort superiority | NOT QUALIFIED | Required SIMD-capable qualification is absent |

The x86 machine was AMD EPYC 7763; ARM was Neoverse-N2. Both report
`simd_available=0` in all rows. All 80 fallback cells, including the two cells
excluded from the predeclared guard, have paired ratios <=1.05. The largest
x86 ratio is 1.034733 (sortedprefix, 65,536); ARM's largest is 1.015584
(random32, 65,536). Thus this run supports fallback behavior only. It cannot
establish whether the new SIMD worker improves v24 or fixes its Intel regressions.

## Supplementary local SIMD pilot

The local host identifies as Intel Xeon Platinum 8573C, with AVX512F+POPCNT.
It is not the earlier AMD EPYC 9V74 host used for v24's counterexample.
The unchanged benchmark's separate pilot mode uses seed 991009, uniform64,
one verified warmup and three measured randomized-order trials. Comparator
revisions are the same current pins as CI. Local Highway compilation builds
only the unsigned-64 ascending and required shared objects using CMake's
generated flags; the full CI library builds are unmodified.

These are medians of three trials in milliseconds, not paired five-seed results:

| N | v17 | v24 | v25 network | v25 stdleaf | VQSort | IPS4o |
|---:|---:|---:|---:|---:|---:|---:|
| 1M | 187.680 | 110.476 | 63.892 | 115.841 | 34.843 | 109.525 |
| 4M | 2384.427 | 2349.073 | 584.166 | 2146.497 | 466.537 | 1017.000 |

The candidate's observed medians are lower than v17/v24 and the stdleaf
ablation. It remains 1.834x/1.252x VQSort's median time at 1M/4M. These ratios
are descriptive only. The 1M candidate trials range from 49.479 to 1392.635 ms
(over 28-fold), and 4M from 574.667 to 2881.240 ms. Other algorithms also vary
substantially. Local test activity was not verified to have stopped before timing; later
polling showed additional full-suite progress. This possible overlap is an
additional confounder. No precise or statistically established speedup follows
from this pilot. It is retained, without rerunning until favorable. All 36 measured
pilot sorts and 12 warmups match the full oracle. Raw trials and full local
compiler/CPU provenance are archived separately from qualification.

## Correctness and audit evidence

- Local optimized, ASan/UBSan, and baseline-x86 builds with runtime SIMD enabled
  each passed 284 exact arrays, 109,952 network cases, and 129 protected-page
  tails. Network checks include all 8-key permutations, all 16-key binary
  inputs, and randomized/adversarial 32-key cases. The baseline build disables
  global AVX-512 and BMI2, checking function-level feature dispatch.
- Local forced fallback passed 284 arrays and 129 protected-page cases. CI
  sanitizer jobs passed on both architectures with leak detection enabled;
  CI tests report zero network cases because SIMD is unavailable. Local
  sanitizer leak detection was disabled for the host environment.
- Exact arrays cover threshold boundaries, full-width and narrow values,
  duplicates, extreme keys, misleading samples, and guarded subarrays. The
  inherited v17 prefix is verified mechanically before each native build.
- Both CI native jobs pass the complete 30-test suite. Local full CMake build
  completed. Its redundant full CTest continued to log progress after a
  termination request; tests 1-24 were observed passing, but this is not claimed
  as a completed local 30/30 run. Its possible overlap with the pilot is
  disclosed above. Targeted local v25 checks had already passed independently.
- The qualification has exactly 2,400 aggregate rows: 7,200 measured sorts
  and 2,400 warmups, all checked against a complete std::sort oracle. Trial
  times, not only medians, are retained. Input copy and oracle construction
  are outside timing. Qualification and exploratory seeds are distinct.
- Both downloaded ZIP SHA-256 digests match GitHub metadata. Recomputed
  summaries match archived CSV bytes. An independent direct calculation
  verifies all 160 summary rows, every paired ratio, and every seed-win count.
- `CI_RESULTS.json` records job/run/artifact identities and verification log
  excerpts. `GATES.json` retains every eligibility decision and gate check.
  `EVIDENCE_LEDGER.json` binds source and artifact hashes to bounded claims.

The evidence commit changes reports/data/documentation only. The source SHA
above is the code measured in CI and the local pilot. No threshold, seed,
algorithm policy, or qualification gate was retuned after viewing results.
OmO was not installed; native tools executed the work. No OmO execution is claimed.

## Next development boundary

Run the frozen full matrix on a stable, identified AVX-512 runner. Include the
earlier AMD-capable counterexample hardware before adopting a broad x86 policy.
Require actual network execution in the sanitizer and benchmark evidence, then
apply the existing benefit, ablation, regression, and comparator gates. Do not
substitute this noisy single-seed pilot for that missing qualification.

The proposed mechanism overlaps established SIMD radix partitioning and sorting
network research, including F5, as described in `PLAN.md`. Current comparator
coverage excludes measured F5 and Intel x86-simd-sort results, so a future
VQSort win alone would still not establish universal SOTA. Proprietary license
is retained; no external implementation was copied into the sorter.

To reproduce analysis, run `summarize.py` for each ISA over its four raw CSVs,
then `evaluate.py --output GATES.json` with the two summaries. The committed
workflow contains the complete native build and benchmark commands and pins.
