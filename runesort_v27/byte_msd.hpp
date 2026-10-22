#pragma once
// Unqualified wider-digit MSD experiment. Never selected by the public API.
#include "../runesort_v26/rune_sort.hpp"
namespace rune_v27::experimental {
using U = std::uint64_t;

// src and buffer are disjoint. out aliases exactly one of them. Each call
// sorts src[0,n) into out[0,n), preserving its multiset. A parent partitions
// by more significant digits before visiting any disjoint child interval.
template<unsigned Leaf, unsigned Bits>
inline void node(U* src, U* buffer, U* out, std::size_t n, int shift) {
  static_assert(Bits>=1 && Bits<=11);
  static_assert(Leaf==32 || Leaf==64 || Leaf==128);
  if (n<=Leaf) {
#if RUNE25_X86
    if (rune_v25::simd_available()) {
      if (n<=8) rune_v25::detail::small<8>(src,out,n);
      else if (n<=16) rune_v25::detail::small<16>(src,out,n);
      else if (n<=32) rune_v25::detail::small<32>(src,out,n);
      else if constexpr (Leaf>=64) {
        if (n<=64) rune_v25::detail::small<64>(src,out,n);
        else if constexpr (Leaf>=128) rune_v25::detail::small<128>(src,out,n);
      }
      return;
    }
#endif
    if (src!=out) std::memcpy(out,src,n*sizeof(U));
    std::sort(out,out+n);
    return;
  }
  if (shift<0) { // All digit bits consumed: the remaining keys are equal.
    if (src!=out) std::memcpy(out,src,n*sizeof(U));
    return;
  }
  constexpr unsigned B=1u<<Bits;
  std::array<std::size_t,B> counts{},starts{},cursors{};
  for (std::size_t i=0; i<n; ++i) ++counts[(src[i]>>shift)&(B-1)];
  std::size_t total=0,largest=0;
  for (unsigned j=0; j<B; ++j) {
    starts[j]=cursors[j]=total;
    total+=counts[j];
    largest=std::max(largest,counts[j]);
  }
  if (largest==n) {
    U varied=0;
    for (std::size_t i=1; i<n; ++i) varied|=src[i]^src[0];
    if (!varied) {
      if (src!=out) std::memcpy(out,src,n*sizeof(U));
      return;
    }
    // Parent-prefix invariance ensures the next varying digit is lower.
    const int next=((63-__builtin_clzll(varied))/Bits)*Bits;
    node<Leaf,Bits>(src,buffer,out,n,next);
    return;
  }
  for (std::size_t i=0; i<n; ++i)
    buffer[cursors[(src[i]>>shift)&(B-1)]++]=src[i];
  for (unsigned j=0; j<B; ++j) if (counts[j])
    node<Leaf,Bits>(buffer+starts[j],src+starts[j],out+starts[j],counts[j],shift-Bits);
}

template<unsigned Leaf, unsigned Bits>
inline void run(std::vector<U>& a) {
  if (a.size()<2) return;
  std::unique_ptr<U[]> buffer(new U[a.size()]);
  node<Leaf,Bits>(a.data(),buffer.get(),a.data(),a.size(),(63/Bits)*Bits);
}
} // namespace rune_v27::experimental
