# v26a frozen baseline — 21 October 2026

This records negative results **before** the v26b dispatcher revision.
Measured code head: `e65e3379b8c4041fa96759c3fdb1090bd11c8d47`.
[Four-job native/sanitizer qualification](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39608916781).
[Standard GitHub CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39608994252).

- Both complete native CTest matrices passed 31/31; two sanitizer jobs passed.
- x86 and ARM each validated 1200 exact observations, 3600 measured sorts
  and 1200 warmups against the full independent oracle.
- Neither CI runner exposed AVX-512: the public v26a nibble worker therefore
  used `std::sort` leaves on both runners.
- Same-seed paired median **v26a/v17 ratios** (smaller better):

| Input | x86 / AMD EPYC 7763 | ARM / Neoverse-N2 |
|---|---:|---:|
| uniform64 1M | 1.083 | 1.107 |
| uniform64 4M | 1.112 | 1.048 |
| entropy-prefix 1M | 0.996 | 1.288 |
| entropy-prefix 4M | 1.027 | 1.334 |

Some ARM performance against pinned VQSort was favorable; these failures
against v17 still **block universal promotion**. The policy follow-up is
evaluated separately; no source, threshold, or comparator is retroactively
redefined for these archived results.
