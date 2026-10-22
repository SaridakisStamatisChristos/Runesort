#pragma once
// Exact compact-domain counting before v17's hash/scout pipeline.
// The sample only selects a trial; a full-array mask certifies every key.
#include "../../../runesort_v22/wide_radix.hpp"
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__)) && !defined(RUNE27_DISABLE_BMI2)
#include <immintrin.h>
#define RUNE27_BMI2 1
#else
#define RUNE27_BMI2 0
#endif
namespace rune_v27_scalar {
namespace detail {
using U = std::uint64_t;
#ifdef RUNE27_TRACE_PATH
inline std::size_t compact_calls = 0;
#endif
inline bool bmi2_available() {
#if RUNE27_BMI2
  return __builtin_cpu_supports("bmi2");
#else
  return false;
#endif
}
inline void contiguous_count(U* a, std::size_t n, U mask, unsigned shift,
                             std::size_t bins) {
  std::vector<std::uint32_t> counts(bins, 0);
  for (std::size_t i=0; i<n; ++i) ++counts[(a[i]&mask)>>shift];
  const U constant = a[0]&~mask;
  std::size_t out=0;
  for (std::size_t b=0; b<bins; ++b) {
    const std::size_t count=counts[b];
    if (count) { std::fill_n(a+out, count, constant|(U(b)<<shift)); out+=count; }
  }
}
#if RUNE27_BMI2
__attribute__((target("bmi2")))
inline void extracted_count(U* a, std::size_t n, U mask, std::size_t bins) {
  std::vector<std::uint32_t> counts(bins, 0);
  for (std::size_t i=0; i<n; ++i) ++counts[_pext_u64(a[i],mask)];
  const U constant = a[0]&~mask;
  std::size_t out=0;
  for (std::size_t b=0; b<bins; ++b) {
    const std::size_t count=counts[b];
    if (count) { std::fill_n(a+out,count,constant|_pdep_u64(b,mask)); out+=count; }
  }
}
#endif
inline bool compact_count(U* a, std::size_t n) {
  // uint32 counters cannot overflow when the entire input fits uint32_t.
  if (n<65536 || n>std::numeric_limits<std::uint32_t>::max()) return false;
  U sampled=0;
  // Distant windows reject ordered / locally perturbed and mixed-domain inputs.
  for (std::size_t begin : {std::size_t(0),n/2,n-64}) {
    unsigned local_descents=0;
    for (std::size_t i=1; i<64; ++i) {
      sampled |= a[begin+i]^a[0];
      local_descents += a[begin+i]<a[begin+i-1];
    }
    if (local_descents<20) return false;
  }
  const unsigned sampled_bits=rune_v17::detail::popcount(sampled);
  if (sampled_bits<12 || sampled_bits>20) return false;
  if ((std::size_t(1)<<(sampled_bits-1))>n) return false;
  // Without fast extraction, a sparse sample is already enough to select the
  // exact fallback. It need not prove the full domain is sparse: declining a
  // potentially profitable attempt changes performance only, never semantics.
  if (!bmi2_available()) {
    const unsigned sampled_shift=rune_v17::detail::ctz(sampled);
    if ((sampled>>sampled_shift)!=((U(1)<<sampled_bits)-1)) return false;
  }
  // Early rejection is read-only; no partially reconstructed array can escape.
  U variable=sampled;
  for (std::size_t begin=0; begin<n;) {
    const std::size_t end=begin+std::min(std::size_t(4096),n-begin);
    U v0=0,v1=0,v2=0,v3=0;
    std::size_t i=begin;
    for (; end-i>=4; i+=4) {
      v0|=a[i]^a[0]; v1|=a[i+1]^a[0];
      v2|=a[i+2]^a[0]; v3|=a[i+3]^a[0];
    }
    variable|=v0|v1|v2|v3;
    for (;i<end;++i) variable|=a[i]^a[0];
    if (rune_v17::detail::popcount(variable)>20) return false;
    begin=end;
  }
  const unsigned bits=rune_v17::detail::popcount(variable);
  const std::size_t bins=std::size_t(1)<<bits;
  if (bins/2>n) return false;
  const unsigned shift=rune_v17::detail::ctz(variable); // sample certifies nonzero
  if ((variable>>shift)==U(bins-1)) {
    contiguous_count(a,n,variable,shift,bins);
  } else {
#if RUNE27_BMI2
    if (!bmi2_available()) return false;
    extracted_count(a,n,variable,bins);
#else
    return false;
#endif
  }
#ifdef RUNE27_TRACE_PATH
  ++compact_calls;
#endif
  return true;
}
} // namespace detail
inline void sort(std::uint64_t* a, std::size_t n) {
  if (n<2) return;
  if (detail::compact_count(a,n)) return;
  rune_v17::sort(a,n);
}
inline void sort(std::vector<std::uint64_t>& a) { sort(a.data(),a.size()); }
} // namespace rune_v27_scalar
