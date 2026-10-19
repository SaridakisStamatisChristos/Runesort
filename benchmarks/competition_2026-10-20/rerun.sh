#!/usr/bin/env bash
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo="$(cd "$here/../.." && pwd)"
n="${1:-1000000}"
reps="${2:-5}"
cmake -S "$repo" -B "$repo/build" -DCMAKE_BUILD_TYPE=Release -DRUNE_WITH_BOOST_BENCHMARK=OFF
cmake --build "$repo/build" --parallel 2
ctest --test-dir "$repo/build" --output-on-failure
"${CXX:-g++}" -O3 -march=native -DNDEBUG -std=c++17 -I "$repo/runesort_v5" "$repo/runesort_v5/bench_v5.cpp" -o "$here/bench_v5"
mkdir -p "$here/reproduced"
modes='uniform,random32,random48,sparse16,narrow,sorted,nearly,rotated,local,K128,K1024,K4096,K8192,K16384,K32768,zipf'
for seed in 271828 314159 777777 8675309; do
  "$here/bench_v5" "$n" "$reps" "$seed" "$modes" > "$here/reproduced/results_seed_${seed}.csv"
done
python3 "$here/make_report.py" "$here/reproduced"
