#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
N="${1:-1000000}"
REPS="${2:-5}"
g++ -O3 -march=native -std=c++17 -DNDEBUG bench_v5.cpp -o bench_v5
for SEED in 271828 314159 777777 8675309; do
  ./bench_v5 "$N" "$REPS" "$SEED" 'uniform,random32,random48,nearly,K128,K512,K4096,K8192,K16384,K32768' > "release_seed_${SEED}.csv"
done
python3 summarize_release.py