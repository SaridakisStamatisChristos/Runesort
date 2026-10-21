#include "prefetch_radix.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=std::uint64_t;
static void verify(const std::vector<U>& input){
  auto reference=input,r17=input,r0=input,r32=input,r128=input;
  std::sort(reference.begin(),reference.end());
  rune_v17::detail::fixed_radix<8>(r17.data(),r17.size());
  rune_v23::radix16<0>(r0.data(),r0.size());
  rune_v23::radix16<32>(r32.data(),r32.size());
  rune_v23::radix16<128>(r128.data(),r128.size());
  if(r17!=reference || r0!=reference || r32!=reference || r128!=reference){
    std::fputs("RUNE v23 FAILED independent full-array sort comparison\n",stderr);
    std::exit(4);
  }
}
int main(){
  // Subarray API contract: neither adjacent sentinel may be changed.
  for(std::size_t n:{0u,1u,2u,31u,32u,33u,127u,128u,129u,4097u}){
    std::vector<U> a(n+2,UINT64_C(0xdeadbeefcafebabe));
    for(std::size_t i=0;i<n;++i)a[i+1]=n-i;
    auto reference=a;std::sort(reference.begin()+1,reference.end()-1);
    for(unsigned d:{0u,32u,128u}){
      auto test=a;
      if(d==0)rune_v23::radix16<0>(test.data()+1,n);
      if(d==32)rune_v23::radix16<32>(test.data()+1,n);
      if(d==128)rune_v23::radix16<128>(test.data()+1,n);
      if(test!=reference)std::exit(4);
    }
  }
  rune_v23::radix16<32>(nullptr,0);
  rune_v23::radix16<128>(nullptr,0);
  std::size_t count=0;
  const std::vector<std::size_t> sizes={0,1,2,3,7,8,15,16,17,31,32,33,63,64,65,127,128,129,
    255,256,257,511,512,513,1023,1024,4095,4096,4097,65535,65536,65537};
  for(auto n:sizes)for(unsigned mode=0;mode<12;++mode){
    std::mt19937_64 g(104729+n*139+mode*991);
    std::vector<U> a(n);
    for(std::size_t i=0;i<n;++i){
      const U x=g();
      switch(mode){
        case 0:a[i]=x;break;
        case 1:a[i]=x & UINT64_C(0xffffffff);break;
        case 2:a[i]=x & UINT64_C(0xffff);break;
        case 3:a[i]=i;break;
        case 4:a[i]=n-i;break;
        case 5:a[i]=UINT64_MAX;break;
        case 6:a[i]=(i&1)?0:UINT64_MAX;break;
        case 7:a[i]=x%3;break;
        case 8:a[i]=(x%81920)*UINT64_C(0x9e3779b97f4a7c15);break;
        case 9:a[i]=(x&2047ULL)<<53;break;
        case 10:a[i]=(x&65535ULL)<<48;break;
        case 11:a[i]=UINT64_C(0x8000000000000000)|(x&65535ULL);break;
      }
    }
    verify(a);++count;
  }
  for(const auto n:{std::size_t(1000000),std::size_t(2000000)}){
    std::mt19937_64 g(n+70001);std::vector<U> a(n);
    for(auto&x:a)x=g();
    verify(a);++count;
    for(auto&x:a)x&=UINT64_C(0xffffffff);
    verify(a);++count;
  }
  std::printf("RUNE v23 differential OK: %zu verified arrays plus 30 guarded subarrays\n",count);
}
