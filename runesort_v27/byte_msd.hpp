#pragma once
// Unqualified experiment: wider-digit MSD; never selected by the public API.
#include "../runesort_v26/rune_sort.hpp"
namespace rune_v27::experimental {
using U=std::uint64_t;
template<unsigned Leaf, unsigned Bits>
inline void node(U*src,U*buf,U*out,size_t n,int shift){
 static_assert(Bits>=1 && Bits<=11 && (Leaf==32 || Leaf==64 || Leaf==128));
 if(n<=Leaf){
#if RUNE25_X86
  if(rune_v25::simd_available()){
  if(n<=8)rune_v25::detail::small<8>(src,out,n);
  else if(n<=16)rune_v25::detail::small<16>(src,out,n);
  else if(n<=32)rune_v25::detail::small<32>(src,out,n);
  else if constexpr(Leaf>=64){
   if(n<=64)rune_v25::detail::small<64>(src,out,n);
   else if constexpr(Leaf>=128)rune_v25::detail::small<128>(src,out,n);
  }
  return;
  }
#endif
  if(src!=out)std::memcpy(out,src,n*sizeof(U));
  std::sort(out,out+n);
  return;
 }
 if(shift<0){if(src!=out)std::memcpy(out,src,n*8);return;}
 constexpr unsigned B=1u<<Bits;
 size_t count[B]{},start[B],cur[B];
 for(size_t i=0;i<n;++i)++count[(src[i]>>shift)&(B-1)];
 size_t total=0,max=0;
 for(unsigned j=0;j<B;++j){start[j]=cur[j]=total;total+=count[j];max=std::max(max,count[j]);}
 if(max==n){U v=0;for(size_t i=1;i<n;++i)v|=src[i]^src[0];if(!v){if(src!=out)std::memcpy(out,src,n*8);return;}node<Leaf,Bits>(src,buf,out,n,((63-__builtin_clzll(v))/Bits)*Bits);return;}
 for(size_t i=0;i<n;++i)buf[cur[(src[i]>>shift)&(B-1)]++]=src[i];
 for(unsigned j=0;j<B;++j)if(count[j])node<Leaf,Bits>(buf+start[j],src+start[j],out+start[j],count[j],shift-Bits);
}
template<unsigned Leaf,unsigned Bits>inline void run(std::vector<U>&a){if(a.size()<2)return;std::unique_ptr<U[]>buf(new U[a.size()]);node<Leaf,Bits>(a.data(),buf.get(),a.data(),a.size(),(63/Bits)*Bits);}
}
