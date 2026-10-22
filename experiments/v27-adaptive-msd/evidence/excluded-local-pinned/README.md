# Excluded incomplete local attempt

A supplementary CPU-2-pinned AVX-512 matrix attempt produced only two uniform64
100K seed blocks before its execution session became unavailable. No running
benchmark process remained when inspected. Trial jitter was extreme (for example,
std::sort 100K ranged into hundreds of milliseconds). This incomplete experiment
is retained as negative operational evidence and excluded from every gate and
performance claim. The completed native GitHub Intel entropy experiment supplies
the full SIMD-active domain comparison instead. No cause of the local session
termination is asserted.
