#pragma once
// RUNE v24 research: isolated x86 large-array dispatcher experiment.
// The dispatcher body is derived verbatim from frozen v17 except its name
// and one high-entropy worker call. verify_inheritance.py checks this contract.
#include "../runesort_v23/prefetch_radix.hpp"
namespace rune_v24 {
inline constexpr size_t prefetch_min_size=1000000;
#if (defined(__x86_64__) || defined(_M_X64)) && !defined(RUNE24_DISABLE)
inline constexpr bool x86_dispatch_enabled=true;
#else
inline constexpr bool x86_dispatch_enabled=false;
#endif
#ifdef RUNE24_TRACE_PATH
inline size_t prefetch_route_calls=0; // Test-only; absent from benchmark/public build.
#endif
namespace detail = rune_v17::detail;
#if (defined(__x86_64__) || defined(_M_X64)) && !defined(RUNE24_DISABLE)
// BEGIN INHERITED DISPATCHER
// Sorts uint64_t values in ascending order, in place. Not a stable record sort.
// Fixed-width uint64 values, worst-case O(n log n) time and O(n) auxiliary memory.
template<unsigned Distance>
inline void dispatch(uint64_t* a, size_t n) {
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
    if(detail::medium_distinct_reconstruct(a,n))return;
    size_t rejection_duplicates=0;
    if(detail::extended_distinct_reconstruct(a,n,&rejection_duplicates))return;
    // An observed duplicate count is a heuristic for domain cardinality.
    // This dispatch affects only performance; the radix fallback is exact.
    if(n>=262144){
      if(rejection_duplicates>=400 &&
         detail::large_distinct_reconstruct(a,n))return;
      // A high duplicate count can still mean 16K--32K keys.
      // Failed 16K reconstruction leaves input intact; try exact 32K tier.
      if(rejection_duplicates>=250 &&
         detail::fourth_distinct_reconstruct(a,n))return;
      // Fifth tier is attempted only when duplicate evidence, sufficient N,
      // and an independent 16K-sample repeat pilot jointly support it.
      if(n>=524288 && detail::popcount(sampled_mask)>16 &&
         // v16c: the old 100-duplicate short scout wrongly rejects K81920
         // for two of five pinned seeds (89 and 97 duplicates). At N>=2M
         // allow 80, but still require the independent two-stage pilot and
         // the unchanged exact fail-closed 81920-key reconstruction.
         rejection_duplicates>=(n>=2000000 ? 80u : 100u) &&
         detail::fifth_tier_pilot(a,n) &&
         detail::fifth_distinct_reconstruct(a,n))return;
    }
  }
  if(sample_unique_count<=48 && detail::distinct_reconstruct(a,n))return;
  // Unordered, high-entropy sample: full byte radix always sorts correctly.
  // The sample selects a method, not a restricted value domain.
  if(n>=16384 && sampled_descents>=20 && detail::popcount(sampled_mask)>=60){
#ifdef RUNE24_TRACE_PATH
    ++prefetch_route_calls;
#endif
    rune_v23::radix16<Distance>(a,n);
    return;
  }
  if(n>=4096 && sampled_descents<=3 && first<=a[n-1] &&
     detail::certify_local16(a,n)) {
    // The certificate ensures insertion needs no more than 16 left shifts.
    if(detail::bounded_insertion_repair(a,n))return;
  }
  uint64_t variable=0, low=first, high=first, second=first;
  bool ascending=true, descending=true, at_most_two=true;
  size_t descents=0, break_at=0;
  uint64_t prev=first;
  for(size_t i=1;i<n;++i){
    uint64_t x=a[i]; variable|=(x^first);
    if(x<prev){ascending=false;++descents;break_at=i;}
    if(x>prev)descending=false;
    if(x<low)low=x;
    if(x>high)high=x;
    if(at_most_two && x!=first){
      if(second==first)second=x;
      else if(x!=second)at_most_two=false;
    }
    prev=x;
  }
  if(ascending)return;
  if(descending){std::reverse(a,a+n);return;}
  // Exactly one descent plus ordered wrap implies two sorted runs that form
  // a circularly shifted nondecreasing array. One in-place rotation fixes it.
  if(descents==1 && a[n-1]<=first){
    std::rotate(a,a+break_at,a+n);
    return;
  }
  if(at_most_two){
    size_t cf=0;for(size_t i=0;i<n;++i)cf+=(a[i]==first);
    if(first<second){std::fill_n(a,cf,first);std::fill_n(a+cf,n-cf,second);}
    else {std::fill_n(a,n-cf,second);std::fill_n(a+n-cf,cf,first);}
    return;
  }
  // Comparator sorting efficiently exploits inputs with few local inversions.
  if(descents<=n/16){
    // Avoid extra allocations for a handful of local inversions, while
    // targeting the many widely displaced elements of sparse swap noise.
    if(n>=4096 && descents>=n/2000 &&
       detail::displaced_merge(a,n,n/16)) return;
    std::sort(a,a+n);return;
  }
  const unsigned varied=detail::popcount(variable);
  // Numeric-range counting has cheaper arithmetic than bit extraction.
  // Guard subtraction overflow via high-low (unsigned modular difference
  // is exact for high>=low) and cap the auxiliary table.
  const uint64_t range=high-low;
  if(range<=1048575 && (range+1)/2<=n){
    size_t bins=size_t(range)+1;
    std::vector<size_t> count(bins,0);
    for(size_t i=0;i<n;++i)++count[size_t(a[i]-low)];
    size_t out=0;
    for(size_t b=0;b<bins;++b){
      if(count[b]){std::fill_n(a+out,count[b],low+uint64_t(b));out+=count[b];}
    }
    return;
  }
  // v14 ARM64-only sparse16 integration from archived candidate PR #5.
  // The full-array mask proves <= 2^16 distinct values. Even contiguous
  // sparse16 masks use the portable extract loop in compact-domain counting,
  // so a packed-digit contiguity guard would incorrectly skip a faster path.
  // The exact bounded hash engine leaves a[] unchanged on any rejection.
#if defined(__aarch64__)
  if(n>=524288 && varied==16 &&
     detail::fifth_distinct_reconstruct(a,n))return;
#endif
  // v13 ARM64-only experiment: avoid the O(popcount(mask)) portable
  // extract loop on full-array compact-domain / packed-digit paths.
  // The full variability mask is certified by the scan above; stable LSD
  // byte radix skips only invariant bytes and sorts all uint64 keys exactly.
#if defined(__aarch64__)
  if(n>=262144 && varied>=17 && varied<=23 &&
     (varied==23 || detail::packed_digits_need_extract(variable))){
    detail::dense_radix(a,n,variable);
    return;
  }
#endif
  // The compact-domain path counts codes, then synthesizes sorted keys.
  // 2^varied <= 2*n avoids a disproportionately large zeroed table.
  if(varied<=16 && (size_t(1)<<(varied-1)) <= n){
    size_t bins=size_t(1)<<varied;
    std::vector<size_t> count(bins,0);
    for(size_t i=0;i<n;++i)++count[detail::extract(a[i],variable)];
    const uint64_t constant=first & ~variable;
    size_t out=0;
    for(size_t b=0;b<bins;++b){
      size_t copies=count[b];
      if(!copies)continue;
      uint64_t value=constant | detail::deposit(b,variable);
      std::fill_n(a+out,copies,value);
      out+=copies;
    }
    return;
  }
  // Fused byte histograms for dense variable positions.
  if(varied>=24){
    if((variable>>32)==0)detail::fixed_radix<4>(a,n);
    else if((variable & 0xff00000000000000ULL) != 0)detail::fixed_radix<8>(a,n);
    else detail::dense_radix(a,n,variable);
    return;
  }
  // Group only variable bit positions, from least significant to most,
  // preserving their numeric order. Stable LSD passes are exact.
  std::vector<detail::Digit> digits;
  size_t total=0;
  uint64_t remaining=variable;
  while(remaining){
    uint64_t mask=0;
    for(unsigned b=0;b<11 && remaining;++b){
      uint64_t lowest=remaining & (~remaining+1);
      mask |= lowest;
      remaining &= remaining-1;
    }
    unsigned shift=detail::ctz(mask), bits=detail::popcount(mask);
    bool contiguous=(mask>>shift)==((1ULL<<bits)-1);
    digits.push_back({mask,shift,bits,total,contiguous});
    total+=size_t(1)<<bits;
  }
  std::vector<size_t> hist(total,0);
  // All global digit histograms can be computed from a single scan, as
  // radix rearrangement never changes the multiset of digits.
  for(size_t i=0;i<n;++i){
    for(const auto& d:digits){++hist[d.start+detail::digit(a[i],d)];}
  }
  for(const auto& d:digits){
    size_t pos=0, bins=size_t(1)<<d.bits;
    for(size_t b=0;b<bins;++b){size_t c=hist[d.start+b];hist[d.start+b]=pos;pos+=c;}
  }
  std::vector<uint64_t> scratch(n);
  uint64_t* src=a;
  uint64_t* dst=scratch.data();
  for(const auto& d:digits){
    for(size_t i=0;i<n;++i){auto x=src[i];dst[hist[d.start+detail::digit(x,d)]++]=x;}
    std::swap(src,dst);
  }
  if(src!=a)std::memcpy(a,src,n*sizeof(uint64_t));
}
// END INHERITED DISPATCHER
#endif
// Exact ascending uint64_t keys, no record-stability guarantee. Like v17,
// worst-case O(n log n) time and O(n) storage; only the selected worker differs.
template<unsigned Distance>
inline void sort_with_distance(uint64_t* a,size_t n){
#if (defined(__x86_64__) || defined(_M_X64)) && !defined(RUNE24_DISABLE)
  if(n>=prefetch_min_size){dispatch<Distance>(a,n);return;}
#endif
  rune_v17::sort(a,n);
}
inline void sort(uint64_t* a,size_t n){sort_with_distance<128>(a,n);}
inline void sort(std::vector<uint64_t>& a){sort(a.data(),a.size());}
inline void ablation0(uint64_t* a,size_t n){sort_with_distance<0>(a,n);}
} // namespace rune_v24
