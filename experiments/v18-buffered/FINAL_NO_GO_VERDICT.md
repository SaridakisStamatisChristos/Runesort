# RUNE v18 — Final cross-ISA buffered radix verdict

**NO-GO: reject buffered radix as a general replacement for the frozen v17 full-width worker. HOLD MERGE, preserve negative evidence.**

The tested [implementation `d0b0e8d2e0c6b46d95cf117e2ed4323a2030b36a`](https://github.com/SaridakisStamatisChristos/Runesort/commit/d0b0e8d2e0c6b46d95cf117e2ed4323a2030b36a) is an isolated research kernel. No production dispatch, original RUNE versions or proprietary LICENSE were altered.

## Verification

- [Workflow run #39483684019](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39483684019): **4/4 passed** — x86 and ARM CMake/CTest and native microbenchmarks, x86 and ARM ASan/UBSan, x86 portable `-mno-bmi2`.
- [Existing CI push #39483682421](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39483682421): passed.
- [Existing CI PR #39483739151](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39483739151): passed.
- **300/300 per-sort measured rows correctly matched `std::sort`**, along with 152 additional differential datasets per architecture (150 small/boundary and two large).
- GCC 13.3, C++17, `-O3 -march=native -DNDEBUG`, one warmup and 3 measured repetitions/algorithm/seed, 5 independent held-out seeds and 5 input modes, N=1M and N=4M, randomized algorithm order. Timings only inside the sort. Two virtualized CPU models: AMD EPYC 7763 (x86) and ARM Neoverse-N2 (ARM). Cross-CPU comparisons are not controlled.

## Full-width and controls at N=4 million — ms (lower is better)

| CPU | Distribution | Frozen v17 | Buffer8 | Buffer16 | b8 paired speed | b16 paired speed |
|---|---|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 51.976 | 173.536 | 152.845 | 0.299× | 0.341× |
| x86_64 | random32 | 46.540 | 172.162 | 126.708 | 0.270× | 0.367× |
| x86_64 | K85000 | 55.719 | 184.760 | 117.912 | 0.302× | 0.473× |
| x86_64 | sorted | 52.037 | 165.483 | 152.093 | 0.315× | 0.345× |
| x86_64 | alternating | 41.059 | 184.537 | 107.371 | 0.223× | 0.382× |
| aarch64 | uniform64 | 59.833 | 84.055 | 70.155 | 0.709× | 0.851× |
| aarch64 | random32 | 79.966 | 78.867 | 73.742 | 0.998× | 1.084× |
| aarch64 | K85000 | 65.434 | 86.286 | 85.791 | 0.757× | 0.763× |
| aarch64 | sorted | 60.126 | 82.743 | 70.195 | 0.730× | 0.853× |
| aarch64 | alternating | 94.613 | 71.349 | 75.216 | 1.325× | 1.251× |

Speed columns are median of the five matched baseline/candidate ratios, **not** ratios of the table medians.

## Falsification

- x86 uniform64: **51.976 ms baseline vs 173.536 ms Buffer8 and 152.845 ms Buffer16**, 0/5 winning seeds for either buffer size. Buffer16 is **~2.94× slower** by displayed medians; Buffer8 **~3.34× slower**. Both buffered variants lose on all x86 modes/seeds tested at N=1M and N=4M.
- ARM uniform64: **59.833 vs 84.055 and 70.155 ms** at 4M. Both buffered variants lose all 5 seeds; no full-width benefit.
- ARM four-million random32: buffer16 gains about 8.4% by median of paired ratios (4/5 seeds), but buffer16 loses badly on uniform64 and K85000.
- ARM alternating: buffered approaches can win on high skew, **but this is an artificially forced eight-pass worker**, not the production dispatcher; special-case and sample sorting shortcuts would usually dominate.
- ARM sorted: forced eight-pass input is a negative control and not evidence about public RUNE latency (which detects sorted input early).
- Existing independent benchmarks show RUNE x86 uniform64 has a major gap against Highway VQSort. v18 worsens that gap and is not a competitive improvement.

Mechanism: staging uses cache-local writes and bulk memcpy, but costs per-key bucket tracking and flushes. Results show that these costs outweigh expected write-locality gains on the target x86/ARM uniform64 regimes. A standard stable LSD buffered write method is **not novel**, regardless of performance.

## Evidence files

- [x86 raw observations](x86_raw.csv) (150 rows)
- [ARM64 raw observations](arm64_raw.csv) (150 rows)
- [paired summary](paired_summary.csv) (20 architecture-mode-size groups)
- [provenance](provenance.json)

## Decision and next research

**NO-GO for integrating either staged-scatter kernel.** Leave PR #18 as a draft negative experiment, do not merge. Frozen v17 remains unaffected. The next high-information x86 investigation should profile memory traffic/cache behavior in the actual production full-width path and compare controlled on-CPU alternatives against pinned Highway VQSort, not make unmeasured architectural rewrites. Do not revive v15 six-pass 11-bit radix without new causal evidence.
