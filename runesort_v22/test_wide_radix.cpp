#include "wide_radix.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=std::uint64_t;
static void verify(const std::vector<U>& input){
  auto reference=input,r17=input,r11=input,r16=input;
  std::sort(reference.begin(),reference.end());
  rune_v17::detail::fixed_radix<8>(r17.data(),r17.size());
  rune_v22::radix11(r11.data(),r11.size());
  rune_v22::radix16(r16.data(),r16.size());
  if(r17!=reference || r11!=reference || r16!=reference){
    std::fputs("RUNE v22 FAILED independent full-array sort comparison\n",stderr);
    std::exit(4);
  }
}
int main(){
  std::size_t count=0;
  const std::vector<std::size_t> sizes={0,1,2,3,7,8,15,16,17,31,63,64,65,
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
  std::printf("RUNE v22 differential OK: %zu verified arrays\n",count);
}
