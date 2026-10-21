#pragma once
#include "simd_msd.hpp"
namespace rune_v25 {
#ifdef RUNE25_TRACE_PATH
inline std::size_t selected_route_calls=0;
#endif
namespace legacy = rune_v17::detail;
// Reuse all exact early exits and bounded distinct-key reconstruction from v17.
// BEGIN INHERITED PREFIX
// Sorts uint64_t values in ascending order, in place. Not a stable record sort.
// Fixed-width uint64 values, worst-case O(n log n) time and O(n) auxiliary memory.
template<bool Network>
inline void selected_dispatch(uint64_t* a, size_t n) {
  if(n<2) return;
  if(n<64){std::sort(a,a+n);return;}
  const uint64_t first=a[0];
  // A small sample can select a profitable path only after its precondition
  // is checked over the whole array, or can choose a universally correct
  // general radix method without any assumption about unseen keys.
  constexpr size_t P=64;
  bool equal_sample=true, asc_sample=true, desc_sample=true;
  unsigned sampled_descents=0;
  uint64_t sampled_mask=0;
  uint64_t second_sample=first;
  bool at_most_two_sample=true;
  uint64_t sample_unique[65]{first};
  size_t sample_unique_count=1;
  for(size_t i=1;i<P;++i){
    uint64_t x=a[i],y=a[i-1];
    equal_sample&=(x==first);
    if(x<y){asc_sample=false;++sampled_descents;}
    if(x>y)desc_sample=false;
    sampled_mask|=(x^first);
    if(sample_unique_count<=63){
      size_t j=0;
      for(;j<sample_unique_count && sample_unique[j]!=x;++j){}
      if(j==sample_unique_count)sample_unique[sample_unique_count++]=x;
    }
    if(x!=first){
      if(second_sample==first)second_sample=x;
      else if(second_sample!=x)at_most_two_sample=false;
    }
  }
  if(equal_sample){
    size_t i=P;
    for(;i+7<n;i+=8){
      uint64_t diff=0;
      for(size_t j=0;j<8;++j)diff|=(a[i+j]^first);
      if(diff)break;
    }
    while(i<n && a[i]==first)++i;
    if(i==n)return;
  } else if(asc_sample || desc_sample){
    bool asc=asc_sample,desc=desc_sample;
    for(size_t i=P;i<n && (asc||desc);++i){
      asc&=(a[i-1]<=a[i]);desc&=(a[i-1]>=a[i]);
    }
    if(asc)return;
    if(desc){std::reverse(a,a+n);return;}
  }
  if(at_most_two_sample && !equal_sample){
    size_t cf=0, i=0;
    for(;i<n;++i){
      uint64_t x=a[i];
      if(x==first)++cf;
      else if(x!=second_sample)break;
    }
    if(i==n){
      if(first<second_sample){std::fill_n(a,cf,first);std::fill_n(a+cf,n-cf,second_sample);}
      else {std::fill_n(a,n-cf,second_sample);std::fill_n(a+n-cf,cf,first);}
      return;
    }
  }
  // Test bounded exact frequency reconstruction before the general scout.
  // The third tier is guided by duplicate evidence from the second-tier rejection.
  // Preserve the v2 small-table fallback for cases below the size cutoff.
  if(n>=65536){
    if(legacy::medium_distinct_reconstruct(a,n))return;
    size_t rejection_duplicates=0;
    if(legacy::extended_distinct_reconstruct(a,n,&rejection_duplicates))return;
    // An observed duplicate count is a heuristic for domain cardinality.
    // This dispatch affects only performance; the radix fallback is exact.
    if(n>=262144){
      if(rejection_duplicates>=400 &&
         legacy::large_distinct_reconstruct(a,n))return;
      // A high duplicate count can still mean 16K--32K keys.
      // Failed 16K reconstruction leaves input intact; try exact 32K tier.
      if(rejection_duplicates>=250 &&
         legacy::fourth_distinct_reconstruct(a,n))return;
      // Fifth tier is attempted only when duplicate evidence, sufficient N,
      // and an independent 16K-sample repeat pilot jointly support it.
      if(n>=524288 && legacy::popcount(sampled_mask)>16 &&
         // v16c: the old 100-duplicate short scout wrongly rejects K81920
         // for two of five pinned seeds (89 and 97 duplicates). At N>=2M
         // allow 80, but still require the independent two-stage pilot and
         // the unchanged exact fail-closed 81920-key reconstruction.
         rejection_duplicates>=(n>=2000000 ? 80u : 100u) &&
         legacy::fifth_tier_pilot(a,n) &&
         legacy::fifth_distinct_reconstruct(a,n))return;
    }
  }
  if(sample_unique_count<=48 && legacy::distinct_reconstruct(a,n))return;
  // Unordered, high-entropy sample: full byte radix always sorts correctly.
  // The sample selects a method, not a restricted value domain.
  if(n>=16384 && sampled_descents>=20 && legacy::popcount(sampled_mask)>=60){
#ifdef RUNE25_TRACE_PATH
    ++selected_route_calls;
#endif
    kernel<Network>(a,n);
    return;
  }
// END INHERITED PREFIX
  rune_v17::sort(a,n); // Defensive fallback; wrapper and prefix use the same gate.
}
// A negative probe delegates to the original compiled v17 sorter. In particular,
// narrow domains and ordered prefixes never enter a duplicate full dispatcher.
template<bool Network=true>
inline void sort_with_leaf(std::uint64_t* a,std::size_t n){
  if(n<1000000 || !simd_available()){rune_v17::sort(a,n);return;}
  std::uint64_t mask=0;unsigned descents=0;
  for(unsigned i=1;i<64;++i){mask|=a[i]^a[0];descents+=a[i]<a[i-1];}
  if(descents<20 || legacy::popcount(mask)<60){rune_v17::sort(a,n);return;}
  selected_dispatch<Network>(a,n);
}
inline void sort(std::uint64_t* a,std::size_t n){sort_with_leaf<true>(a,n);}
inline void sort(std::vector<std::uint64_t>& a){sort(a.data(),a.size());}
inline void ablation(std::uint64_t* a,std::size_t n){sort_with_leaf<false>(a,n);}
} // namespace rune_v25
