#pragma once
#include "../runesort_v23/prefetch_radix.hpp"
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__)) && !defined(RUNE25_DISABLE_SIMD)
#include <immintrin.h>
#define RUNE25_X86 1
#define RUNE25_TARGET __attribute__((target("avx512f,popcnt")))
#else
#define RUNE25_X86 0
#endif
namespace rune_v25 {
inline bool simd_available(){
#if RUNE25_X86
  return __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("popcnt");
#else
  return false;
#endif
}
#if RUNE25_X86
namespace detail {
template<unsigned R,unsigned K,unsigned J>
constexpr unsigned max_mask(){
  unsigned mask=0;
  for(unsigned lane=0;lane<8;++lane){
    const unsigned i=8*R+lane;
    if(bool(i&J)!=bool(i&K))mask|=1u<<lane;
  }
  return mask;
}
template<unsigned R,unsigned K,unsigned J,unsigned N>
RUNE25_TARGET inline void lane_stage(__m512i (&v)[N/8]){
  const auto perm=_mm512_set_epi64(7^J,6^J,5^J,4^J,3^J,2^J,1^J,0^J);
  const auto other=_mm512_permutexvar_epi64(perm,v[R]);
  v[R]=_mm512_mask_blend_epi64(max_mask<R,K,J>(),
      _mm512_min_epu64(v[R],other),_mm512_max_epu64(v[R],other));
}
template<unsigned K,unsigned J,unsigned N,std::size_t... R>
RUNE25_TARGET inline void lanes(__m512i (&v)[N/8],std::index_sequence<R...>){
  (lane_stage<R,K,J,N>(v),...);
}
template<unsigned N,unsigned K=2,unsigned J=1>
RUNE25_TARGET inline void network(__m512i (&v)[N/8]){
  if constexpr(J<8){
    lanes<K,J,N>(v,std::make_index_sequence<N/8>{});
  }else{
    for(unsigned r=0;r<N/8;++r){
      const unsigned q=r^(J/8);
      if(r<q){
        const auto low=_mm512_min_epu64(v[r],v[q]);
        const auto high=_mm512_max_epu64(v[r],v[q]);
        if((r*8)&K){v[r]=high;v[q]=low;}else{v[r]=low;v[q]=high;}
      }
    }
  }
  if constexpr(J>1)network<N,K,J/2>(v);
  else if constexpr(K<N)network<N,K*2,K>(v);
}
template<unsigned N>
RUNE25_TARGET inline void small(const std::uint64_t* src,std::uint64_t* out,std::size_t n){
  __m512i v[N/8];
  for(unsigned r=0;r<N/8;++r){
    v[r]=_mm512_set1_epi64(-1);
    if(r*8<n){
      const auto count=std::min(std::size_t(8),n-r*8);
      v[r]=_mm512_mask_loadu_epi64(v[r],(1u<<count)-1,src+r*8);
    }
  }
  network<N>(v);
  for(unsigned r=0;r<N/8;++r)if(r*8<n){
    const auto count=std::min(std::size_t(8),n-r*8);
    _mm512_mask_storeu_epi64(out+r*8,(1u<<count)-1,v[r]);
  }
}
RUNE25_TARGET inline std::uint64_t variation(const std::uint64_t* a,std::size_t n){
  const auto first=_mm512_set1_epi64(static_cast<long long>(a[0]));
  auto accum=_mm512_setzero_si512();std::size_t i=0;
  for(;n-i>=8;i+=8)accum=_mm512_or_si512(accum,
      _mm512_xor_si512(_mm512_loadu_si512(a+i),first));
  auto mask=static_cast<std::uint64_t>(_mm512_reduce_or_epi64(accum));
  for(;i<n;++i)mask|=a[i]^a[0];
  return mask;
}
template<bool Network>
RUNE25_TARGET inline void node(std::uint64_t* src,std::uint64_t* scratch,
                              std::uint64_t* out,std::size_t n,int bit){
  if(bit<0){if(src!=out)std::memcpy(out,src,n*sizeof(*out));return;}
  if(n<=32){
    if constexpr(Network){
      if(n<=8)small<8>(src,out,n);
      else if(n<=16)small<16>(src,out,n);
      else small<32>(src,out,n);
    }else{
      if(src!=out)std::memcpy(out,src,n*sizeof(*out));
      std::sort(out,out+n);
    }
    return;
  }
  const auto mask=_mm512_set1_epi64(static_cast<long long>(std::uint64_t(1)<<bit));
  std::size_t left=0,right=n,i=0;
  for(;n-i>=8;i+=8){
    const auto v=_mm512_loadu_si512(src+i);
    const unsigned hi=_mm512_test_epi64_mask(v,mask);
    const unsigned count=__builtin_popcount(hi);
    right-=count;
    _mm512_mask_compressstoreu_epi64(scratch+left,static_cast<__mmask8>(~hi),v);
    _mm512_mask_compressstoreu_epi64(scratch+right,hi,v);
    left+=8-count;
  }
  for(;i<n;++i){
    const auto x=src[i];
    if((x>>bit)&1)scratch[--right]=x;else scratch[left++]=x;
  }
  if(left==0 || left==n){
    // A common prefix may skip many bits. Check exact variation before recursing.
    const auto varied=variation(src,n);
    if(!varied){if(src!=out)std::memcpy(out,src,n*sizeof(*out));return;}
    const int next=63-__builtin_clzll(varied);
    node<Network>(src,scratch,out,n,next);
    return;
  }
  node<Network>(scratch,src,out,left,bit-1);
  node<Network>(scratch+left,src+left,out+left,n-left,bit-1);
}
} // namespace detail
#endif
template<bool Network=true>
inline void kernel(std::uint64_t* a,std::size_t n){
  if(n<2)return;
#if RUNE25_X86
  if(simd_available()){
    std::unique_ptr<std::uint64_t[]> scratch(new std::uint64_t[n]);
    detail::node<Network>(a,scratch.get(),a,n,63);return;
  }
#endif
  rune_v17::sort(a,n);
}
} // namespace rune_v25
#if RUNE25_X86
#undef RUNE25_TARGET
#endif
