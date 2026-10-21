# RUNE v20 — Full-width baseline and phase investigation

**No RUNE algorithm change.** Research starts from frozen v17 head `927d29b778b75f67e8a908cffa914a5f6e88929f`, not rejected v18/v19. The user requested continued algorithm work, but the evidence gate requires diagnosing the x86 full-width bottleneck before more rewrites.

## Contract / priority
Exactly sort unsigned 64-bit keys ascending on the same input data and machine. Benchmark (1) full v17 public dispatcher; (2) frozen direct 8-pass radix worker; (3) pinned Highway VQSort; (4) pinned sequential IPS4o; (5) `std::sort`. Native x86/ARM GitHub virtual runners, 5 preregistered seeds, 5 workload classes, n=1M and n=4M, 1 warmup + 3 measured randomized-order trials; every output compared element-for-element with independent `std::sort`.

Pin Highway commit `c3407ccaaecffa735c911cae3971efa02263211d` and IPS4o commit `8d5a97bb4c4048a1eef18b537f191a31462b051a`, the same exact revisions previously used in RUNE v14. Fetch original upstreams via Git checkout and assert SHA; never vendor or edit external sources.

## Mechanism attribution

The dispatcher/worker ratio answers whether pilot, early rejection and dispatch decision costs dominate. Independent instrumented eight-pass radix times fused histogram build, offsets, scratch allocation, each stable scatter, and final copy. The instrumentation changes instruction timing and memory behavior; its phases are **approximate attribution**, not hardware-counter proof or uninstrumented baseline timing. CI perf events may be unavailable; do not invent counters.

Primary endpoint: N=4M, uniform64, x86, RUNE17_full/VQSort and RUNE17_fixed8/VQSort same-run paired ratios. Other regimes are negative controls. Cross-architecture times must never be used as direct ratios. Ordered/alternating full-worker forced sorting is a mechanism control, not an end-to-end algorithm claim.

## Acceptance / rejection

- All 500 output-verified competition observations correct across 2 CPUs × 2 sizes × 5 modes × 5 seeds × 5 algorithms. Native build and independent sanitizer checks.
- All phase-diagnostic outputs correct against sorted oracle. Archive raw timings.
- Report causal interpretation only to the extent timing supports it. If full RUNE ≈ direct radix but both lag VQSort, suspect scatter/memory and reject speculative dispatcher changes. If full dispatcher ≫ direct kernel, investigate dispatch.
- This is **measurement**, no claim of innovation or release-ready superiority, no merge.
