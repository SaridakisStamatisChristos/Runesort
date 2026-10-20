#include "runesort_v12/rune_sort.hpp"
#define rune rune_v10
#include "runesort_v10/rune_sort.hpp"
#undef rune
#define rune rune_v11
#include "experiments/v12_baselines/v11/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;
using Clock = std::chrono::steady_clock;

static std::vector<U> generate(size_t n, const std::string &mode, U seed) {
  std::mt19937_64 g(seed);
  std::vector<U> a(n);
  if (mode == "uniform64") {
    for (auto &x : a) x = g();
  } else if (mode == "random32") {
    for (auto &x : a) x = g() & 0xffffffffULL;
  } else if (mode.rfind("sparse", 0) == 0) {
    unsigned bits = static_cast<unsigned>(std::stoul(mode.substr(6)));
    U mask = 0;
    for (unsigned i = 0; i < bits; ++i)
      mask |= U(1) << ((i * 29 + 3) & 63);
    for (auto &x : a) x = rune::detail::deposit(g(), mask);
  } else if (mode.size() > 1 && mode[0] == 'K') {
    size_t k = std::stoull(mode.substr(1));
    std::vector<U> keys(k);
    for (auto &x : keys) x = g();
    for (auto &x : a) x = keys[g() % k];
  } else if (mode == "local" || mode == "nearly") {
    for (auto &x : a) x = g();
    std::sort(a.begin(), a.end());
    for (size_t i = 0; i < n / 100; ++i) {
      size_t j = g() % n, k = mode == "local" ? (j + 1) % n : g() % n;
      std::swap(a[j], a[k]);
    }
  } else {
    std::abort();
  }
  return a;
}
int main(int argc, char **argv) {
  size_t n = argc > 1 ? std::stoull(argv[1]) : 1000000;
  constexpr size_t reps = 4;
  const std::array<U, 5> seeds = {173207,173209,173219,173227,173233};
  const std::array<std::string, 8> modes = {
      "uniform64", "random32", "sparse16", "sparse20",
      "sparse22", "sparse23", "K90112", "K131072"};
  std::cout << "workload,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
  for (size_t mi = 0; mi < modes.size(); ++mi) for (U seed : seeds) {
    auto data = generate(n, modes[mi], seed + mi * 137), oracle = data;
    std::sort(oracle.begin(), oracle.end());
    std::array<std::array<double, reps>, 3> times{};
    std::mt19937_64 order_rng(seed * 65537 + mi);
    for (size_t rep = 0; rep <= reps; ++rep) {
      std::array<int, 3> order{0,1,2};
      std::shuffle(order.begin(), order.end(), order_rng);
      for (int id : order) {
        auto a = data;
        auto start = Clock::now();
        if (id == 0) rune::sort(a);
        else if (id == 1) rune_v10::sort(a);
        else rune_v11::sort(a);
        auto end = Clock::now();
        if (a != oracle) {
          std::cerr << "INCORRECT n=" << n << " mode=" << modes[mi]
                    << " seed=" << seed << " sorter=" << id << "\n";
          return 3;
        }
        if (rep) times[id][rep-1] =
          std::chrono::duration<double,std::milli>(end-start).count();
      }
    }
    const std::array<const char*,3> name{"RUNE_v12","RUNE_v10","RUNE_v11"};
    for (int id = 0; id < 3; ++id) {
      auto ts = times[id];
      std::sort(ts.begin(), ts.end());
      std::cout << modes[mi] << ',' << n << ',' << seed << ',' << name[id] << ','
                << (ts[1]+ts[2])*0.5 << ',' << ts.front() << ','
                << ts.back() << ",true\n";
    }
    std::cout.flush();
  }
}
