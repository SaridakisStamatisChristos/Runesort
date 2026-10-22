#include "scalar_v27b.hpp"
#include <immintrin.h>
#include <random>
#include <iostream>
#include <chrono>
using U=std::uint64_t;
__attribute__((target("avx512f,bmi2")))
void reconstruct(U*a,const std::vector<std::uint32_t>&counts,U mask,U constant){
 size_t out=0;
 for(size_t b=0;b<counts.size();b+=16){
  const auto v=_mm512_loadu_si512(counts.data()+b);
  unsigned live=_mm512_cmpneq_epi32_mask(v,_mm512_setzero_si512());
  while(live){unsigned j=__builtin_ctz(live);live&=live-1;size_t copies=counts[b+j];
   auto key=constant|_pdep_u64(b+j,mask);const auto value=_mm512_set1_epi64((long long)key);
   while(copies>8){_mm512_storeu_si512(a+out,value);out+=8;copies-=8;}
   _mm512_mask_storeu_epi64(a+out,static_cast<__mmask8>((1u<<copies)-1),value);out+=copies;
  }
 }
}
__attribute__((target("avx512f,bmi2")))
void simd(U*a,size_t n,U mask){
 std::vector<std::uint32_t>counts(size_t(1)<<__builtin_popcountll(mask));
 for(size_t i=0;i<n;++i)++counts[_pext_u64(a[i],mask)];
 reconstruct(a,counts,mask,a[0]&~mask);
}
int main(){
 std::mt19937_64 g(707601);
 for(size_t n:{1000000u,4000000u})for(unsigned bits:{16u,17u,20u}){
  U mask=0;for(unsigned i=0;i<bits;++i)mask|=U(1)<<((i*29+7)&63);
  std::vector<U>input(n);for(auto&x:input)x=g()&mask;auto ref=input;std::sort(ref.begin(),ref.end());
  for(int rep=0;rep<6;++rep)for(int k=0;k<2;++k){int alg=(k+rep)%2;auto a=input;auto start=std::chrono::steady_clock::now();
   if(alg)simd(a.data(),n,mask);else rune_v27_scalar::detail::extracted_count(a.data(),n,mask,size_t(1)<<bits);
   auto stop=std::chrono::steady_clock::now();if(a!=ref)abort();if(rep)std::cout<<n<<','<<bits<<','<<alg<<','<<std::chrono::duration<double,std::milli>(stop-start).count()<<'\n';
  }
 }
}
