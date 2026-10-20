#pragma once
// RUNE Sort v6 experimental: bounded exact fourth frequency tier.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>
#include <utility>
#if defined(__BMI2__) && (defined(__x86_64__) || defined(_M_X64))
#include <immintrin.h>
#endif

namespace rune {
namespace detail {
inline unsigned popcount(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_popcountll(x);
#else
  unsigned n=0; while(x){x &= x-1; ++n;} return n;
#endif
}
inline unsigned ctz(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_ctzll(x);
#else
  unsigned n=0; while(!(x & 1)){x >>= 1; ++n;} return n;
#endif
}
inline uint64_t extract(uint64_t key,uint64_t mask){
#if defined(__BMI2__) && (defined(__x86_64__) || defined(_M_X64))
  return _pext_u64(key,mask);
#else
  uint64_t out=0, bit=1;
  while(mask){auto low=mask & (~mask+1); if(key & low)out|=bit; mask&=mask-1; bit<<=1;}
  return out;
#endif
}
inline uint64_t deposit(uint64_t code,uint64_t mask){
#if defined(__BMI2__) && (defined(__x86_64__) || defined(_M_X64))
  return _pdep_u64(code,mask);
#else
  uint64_t out=0, bit=1;
  while(mask){auto low=mask & (~mask+1); if(code & bit)out|=low; mask&=mask-1; bit<<=1;}
  return out;
#endif
}
struct Digit{uint64_t mask; unsigned shift; unsigned bits; size_t start; bool contiguous;};
inline uint64_t digit(uint64_t key, const Digit& d) {
  return d.contiguous ? ((key >> d.shift) & ((1ULL<<d.bits)-1)) : extract(key,d.mask);
}
// Exact, bounded distinct-key counting for arbitrary (possibly distant)
// 64-bit key values. A miss or too many keys returns without modifying a[].
inline bool distinct_reconstruct(uint64_t* a, size_t n) {
  constexpr size_t slots=256, max_keys=128, max_probe=32;
  std::array<uint64_t,slots> keys{};
  std::array<size_t,slots> counts{};
  std::array<uint8_t,slots> occupied{};
  size_t unique=0;
  for(size_t i=0;i<n;++i){
    uint64_t x=a[i];
    uint64_t h=x;
    h^=h>>33;h*=0xff51afd7ed558ccdULL;
    h^=h>>33;h*=0xc4ceb9fe1a85ec53ULL;
    h^=h>>33;
    size_t b=size_t(h)&(slots-1),probes=0;
    while(occupied[b] && keys[b]!=x){
      if(++probes>=max_probe)return false;
      b=(b+1)&(slots-1);
    }
    if(!occupied[b]){
      if(unique==max_keys)return false;
      occupied[b]=1;keys[b]=x;++unique;
    }
    ++counts[b];
  }
  std::array<std::pair<uint64_t,size_t>,max_keys> values{};
  size_t m=0;
  for(size_t j=0;j<slots;++j)if(occupied[j])values[m++]={keys[j],counts[j]};
  std::sort(values.begin(),values.begin()+m,[](const auto& x,const auto& y){return x.first<y.first;});
  size_t o=0;
  for(size_t j=0;j<m;++j){std::fill_n(a+o,values[j].second,values[j].first);o+=values[j].second;}
  return true;
}

// Bounded value-reconstruction for medium-cardinality, high-span keys.
// Uses an independent, larger table only when a sample suggests a modest K.
// Failure leaves the original array unchanged (correct fallback is mandatory).
inline bool medium_distinct_reconstruct(uint64_t* a, size_t n) {
  constexpr size_t slots=4096, max_keys=1024, max_probe=32;
  std::array<uint64_t,slots> keys{};
  std::array<uint32_t,slots> counts{};
  std::array<uint8_t,slots> used{};
  size_t unique=0;
  for(size_t i=0;i<n;++i){
    uint64_t x=a[i];
    uint64_t h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;
    h^=h>>33;
    size_t b=size_t(h>>(64-12));
    unsigned probes=0;
    while(used[b] && keys[b]!=x){
      if(++probes>=max_probe)return false;
      b=(b+1)&(slots-1);
    }
    if(!used[b]){
      if(unique==max_keys)return false;
      used[b]=1;keys[b]=x; ++unique;
    }
    // Counts remain safe if n exceeds 2^32 (avoids silent overflow).
    if(counts[b]==std::numeric_limits<uint32_t>::max())return false;
    ++counts[b];
  }
  std::vector<std::pair<uint64_t,uint32_t>> values;
  values.reserve(unique);
  for(size_t j=0;j<slots;++j) if(used[j])values.push_back({keys[j],counts[j]});
  std::sort(values.begin(),values.end(),[](const auto& a,const auto& b){return a.first<b.first;});
  size_t out=0;
  for(const auto& kv:values){std::fill_n(a+out,kv.second,kv.first);out+=kv.second;}
  return true;
}

// Second-tier exact bounded frequency reconstruction for 2K--4K keys.
// On rejection, optionally report repeats seen before reaching capacity.
// This is attempted only after the small frequency table rejected the input;
// rejection cannot change a[], so fallback is safe.
inline bool extended_distinct_reconstruct(uint64_t* a, size_t n, size_t* rejection_duplicates=nullptr) {
  constexpr size_t slots=16384, max_keys=4096, max_probe=32;
  std::vector<uint64_t> keys(slots);
  std::vector<uint32_t> counts(slots);
  std::vector<uint8_t> used(slots);
  size_t unique=0;
  size_t duplicates=0;
  auto reject=[&](){if(rejection_duplicates)*rejection_duplicates=duplicates;return false;};
  for(size_t i=0;i<n;++i){
    uint64_t x=a[i];
    uint64_t h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;
    h^=h>>33;
    size_t b=size_t(h>>(64-14));
    unsigned probes=0;
    while(used[b] && keys[b]!=x){
      if(++probes>=max_probe) return reject();
      b=(b+1)&(slots-1);
    }
    if(!used[b]){
      if(unique==max_keys)return reject();
      used[b]=1;keys[b]=x;++unique;
    } else ++duplicates;
    if(counts[b]==std::numeric_limits<uint32_t>::max())return reject();
    ++counts[b];
  }
  std::vector<std::pair<uint64_t,uint32_t>> values;
  values.reserve(unique);
  for(size_t j=0;j<slots;++j)if(used[j])values.push_back({keys[j],counts[j]});
  std::sort(values.begin(),values.end(),[](const auto& x,const auto& y){return x.first<y.first;});
  size_t out=0;
  for(const auto& kv:values){std::fill_n(a+out,kv.second,kv.first);out+=kv.second;}
  return true;
}

// v5: cache-bounded third frequency tier. It is used only after the
// 1024- and 4096-key attempts reject the input. It never modifies a[] on
// rejection, including collision/probe/counter overflow failure.
// Intended primarily for N >> K, K <= 16384, over arbitrary 64-bit keys.
inline bool large_distinct_reconstruct(uint64_t* a, size_t n) {
  constexpr size_t slots=65536, max_keys=16384, max_probe=32;
  std::vector<uint64_t> keys(slots);
  std::vector<uint32_t> counts(slots);
  std::vector<uint8_t> used(slots);
  size_t unique=0;
  for(size_t i=0;i<n;++i){
    uint64_t x=a[i];
    uint64_t h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;
    h^=h>>33;
    size_t b=size_t(h>>(64-16));
    unsigned probes=0;
    while(used[b] && keys[b]!=x){
      if(++probes>=max_probe) return false;
      b=(b+1)&(slots-1);
    }
    if(!used[b]){
      if(unique==max_keys)return false;
      used[b]=1;keys[b]=x;++unique;
    }
    if(counts[b]==std::numeric_limits<uint32_t>::max())return false;
    ++counts[b];
  }
  std::vector<std::pair<uint64_t,uint32_t>> pairs;
  pairs.reserve(unique);
  for(size_t j=0;j<slots;++j) if(used[j])pairs.emplace_back(keys[j],counts[j]);
  std::sort(pairs.begin(),pairs.end(),[](const auto& x,const auto& y){return x.first<y.first;});
  size_t out=0;
  for(const auto& kv:pairs){std::fill_n(a+out,kv.second,kv.first);out+=kv.second;}
  return true;
}

// v6 experimental fourth tier: exact counting for at most 32,768 distinct
// uint64_t keys. Never modifies input before a complete successful scan.
// On capacity, collision-probe, or counter overflow failure it returns false.
inline bool fourth_distinct_reconstruct(uint64_t* a, size_t n) {
  constexpr size_t slots=131072, max_keys=32768, max_probe=32;
  std::vector<uint64_t> keys(slots);
  std::vector<uint32_t> counts(slots);
  std::vector<uint8_t> used(slots);
  size_t unique=0;
  for(size_t i=0;i<n;++i){
    const uint64_t x=a[i];
    uint64_t h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;
    h^=h>>33;
    size_t b=size_t(h>>(64-17));
    unsigned probes=0;
    while(used[b] && keys[b]!=x){
      if(++probes>=max_probe)return false;
      b=(b+1)&(slots-1);
    }
    if(!used[b]){
      if(unique==max_keys)return false;
      used[b]=1;keys[b]=x;++unique;
    }
    if(counts[b]==std::numeric_limits<uint32_t>::max())return false;
    ++counts[b];
  }
  std::vector<std::pair<uint64_t,uint32_t>> pairs;
  pairs.reserve(unique);
  for(size_t j=0;j<slots;++j)if(used[j])pairs.emplace_back(keys[j],counts[j]);
  std::sort(pairs.begin(),pairs.end(),[](const auto& x,const auto& y){return x.first<y.first;});
  size_t out=0;
  for(const auto& kv:pairs){std::fill_n(a+out,kv.second,kv.first);out+=kv.second;}
  return true;
}

// Bounded insertion restoration: for data where misplaced keys need only
// a few leftward shifts, restore sortedness in O(n+K * #inversions).
// The prefix [0,i) is sorted on every completed iteration.
// Failure may modify the original array; the fallback sorts its exact multiset.
inline bool bounded_insertion_repair(uint64_t* a,size_t n,size_t max_shift=16){
  for(size_t i=1;i<n;++i){
    const uint64_t x=a[i];
    size_t j=i;
    while(j && j+max_shift>i && a[j-1]>x){a[j]=a[j-1];--j;}
    a[j]=x;
    if(j && a[j-1]>x) return false;
  }
  return true;
}

// Sufficient condition for at most 16 left-shifts per inserted key:
// a[i] is >= the maximum element from original positions [0,i-17].
// This is a read-only O(n) certificate, and an early failure is cheap.
inline bool certify_local16(const uint64_t* a,size_t n){
  uint64_t distant_max=a[0];
  for(size_t i=17;i<n;++i){
    distant_max=std::max(distant_max,a[i-17]);
    if(distant_max>a[i])return false;
  }
  return true;
}

// Fixed-width fused-histogram radix; scratch is value-initialized by std::vector.
// Correct for all keys when Passes == 8. Passes == 4 requires all keys
// to have the same upper 32 bits; the caller MUST certify that condition.
template<unsigned Passes>
inline void fixed_radix(uint64_t* a, size_t n) {
  static_assert(Passes==4 || Passes==8, "Supported widths 32,64");
  size_t h[Passes][256]{};
  for(size_t i=0;i<n;++i)
    for(unsigned d=0;d<Passes;++d) ++h[d][(a[i]>>(d*8))&255];
  for(unsigned d=0;d<Passes;++d){
    size_t pos=0;
    for(unsigned j=0;j<256;++j){auto z=h[d][j];h[d][j]=pos;pos+=z;}
  }
  std::vector<uint64_t> tmp(n);
  uint64_t *s=a,*t=tmp.data();
  for(unsigned d=0;d<Passes;++d){
    for(size_t i=0;i<n;++i){uint64_t v=s[i];t[h[d][(v>>(d*8))&255]++]=v;}
    std::swap(s,t);
  }
  if(s!=a)std::copy(s,s+n,a);
}

// Byte-radix worker: may process all 8 bytes without requiring a prior scout.
// Skipping invariant bytes is merely a performance optimization.
inline void dense_radix(uint64_t* a, size_t n, uint64_t variable) {
  size_t hist[8][256]{};
  unsigned shifts[8]; unsigned nd=0;
  for(unsigned sh=0;sh<64;sh+=8)
    if(((variable>>sh)&255)!=0) shifts[nd++]=sh;
  for(size_t i=0;i<n;++i){
    uint64_t x=a[i];
    for(unsigned j=0;j<nd;++j){unsigned sh=shifts[j]; ++hist[sh/8][(x>>sh)&255];}
  }
  for(unsigned j=0;j<nd;++j){
    unsigned sh=shifts[j]; size_t pos=0;
    for(size_t b=0;b<256;++b){auto c=hist[sh/8][b];hist[sh/8][b]=pos;pos+=c;}
  }
  std::vector<uint64_t> scratch(n);
  uint64_t *src=a,*dst=scratch.data();
  for(unsigned j=0;j<nd;++j){
    unsigned sh=shifts[j];auto* offsets=hist[sh/8];
    for(size_t i=0;i<n;++i){uint64_t x=src[i];dst[offsets[(x>>sh)&255]++]=x;}
    std::swap(src,dst);
  }
  if(src!=a)std::memcpy(a,src,n*sizeof(uint64_t));
}

// Recover an increasing backbone, independently sort extracted outliers,
// then merge. At all times scratch[0:front) is nondecreasing and every
// input value exists in scratch[0:front) or scratch[back:n).
// Worst-case invalid/hostile inputs safely abort BEFORE mutating a[].
inline bool displaced_merge(uint64_t* a, size_t n, size_t budget) {
  std::vector<uint64_t> scratch(n);
  size_t front=0, back=n;
  constexpr size_t REWIND=2;
  for(size_t i=0;i<n;++i) {
    uint64_t x=a[i];
    if(front && x<scratch[front-1]){
      // A small displaced high-valued run can be removed retroactively.
      // If there is no predecessor <= x nearby, discard x instead.
      size_t pop=0;
      while(pop<REWIND && pop<front && scratch[front-pop-1]>x) ++pop;
      bool rewind=(pop==front || (pop<front && scratch[front-pop-1]<=x));
      if(rewind){
        if(n-back+pop>budget) return false;
        for(size_t j=0;j<pop;++j){scratch[--back]=scratch[--front];}
      } else {
        if(n-back>=budget)return false;
        scratch[--back]=x;
        continue;
      }
    }
    scratch[front++]=x;
  }
  std::sort(scratch.begin()+back,scratch.end());
  std::merge(scratch.begin(),scratch.begin()+front,
             scratch.begin()+back,scratch.end(),a);
  return true;
}
} // namespace detail

// Sorts uint64_t values in ascending order, in place. Not a stable record sort.
// Fixed-width uint64 values, worst-case O(n log n) time and O(n) auxiliary memory.
inline void sort(uint64_t* a, size_t n) {
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
      if(rejection_duplicates>=400){
        if(detail::large_distinct_reconstruct(a,n))return;
      }else if(rejection_duplicates>=250){
        if(detail::fourth_distinct_reconstruct(a,n))return;
      }
    }
  }
  if(sample_unique_count<=48 && detail::distinct_reconstruct(a,n))return;
  // Unordered, high-entropy sample: full byte radix always sorts correctly.
  // The sample selects a method, not a restricted value domain.
  if(n>=16384 && sampled_descents>=20 && detail::popcount(sampled_mask)>=60){
    detail::fixed_radix<8>(a,n);
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
inline void sort(std::vector<uint64_t>& v){sort(v.data(),v.size());}
} // namespace rune