# Reproduce the v8 pilot-only calibration

Use **C++17**, without requiring external libraries:

```bash
g++ -O3 -std=c++17 experiments/v8-pilot-calibration.cpp -o pilot_calibration
./pilot_calibration calibration > runesort_v8/evidence/pilot_calibration.csv
./pilot_calibration heldout > runesort_v8/evidence/pilot_heldout.csv
```

The generated 300 rows per partition measure **how many repeated values appear
in 16,384 and 32,768 evenly spaced array samples**, not sorting speed. Each
sample uses exact `uint64_t` equality. The same `std::mt19937_64` C++ input
generation as the original local experiment is used with `seed+987`.

The v7 baseline accepts iff `d16>=1600`. The v8 experimental decision
accepts iff `d16>=1760` or `d16>1550 && d32>=6000`. Neither is a
correctness assertion. The full sorter always validates all values and retains
an exact general fallback.

The threshold was tuned using the **calibration** seeds, with the **heldout**
seeds used only for validation. This does not rule out pathological, correlated,
or adversarial samples; a future benchmark must validate full-sort runtime.
