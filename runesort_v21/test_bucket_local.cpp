#include "bucket_local.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=std::uint64_t;
static void verify(const std::vector<U>& source){
  auto oracle=source,baseline=source,b8=source,b12=source;
  std::sort(oracle.begin(),oracle.end());
  rune_v17::sort(baseline);
  rune_v21::bucket8(b8.data(),b8.size());
  rune_v21::bucket12(b12.data(),b12.size());
  if(oracle!=baseline||oracle!=b8||oracle!=b12){
    std::fputs("RUNE v21 full-array oracle mismatch\n",stderr);
    std::exit(3);
  }
}
int main(){
  std::size_t tests=0;
  const std::vector<std::size_t> sizes={0,1,2,3,7,8,15,16,17,31,63,64,65,127,
    255,256,257,511,512,513,1023,1024,4095,4096,4097,65535,65536};
  for(const auto n:sizes)for(unsigned mode=0;mode<13;++mode){
    std::mt19937_64 g(79271+n*101+mode*1009);
    std::vector<U> a(n);
    for(std::size_t i=0;i<n;++i){
      const U x=g();
      switch(mode){
        case 0:a[i]=x;break;
        case 1:a[i]=x&UINT64_C(0xffffffff);break;
        case 2:a[i]=x&UINT64_C(0xffff);break;
        case 3:a[i]=i;break;
        case 4:a[i]=n-i;break;
        case 5:a[i]=UINT64_MAX;break;
        case 6:a[i]=i%2?0:UINT64_MAX;break;
        case 7:a[i]=x%3;break;
        case 8:a[i]=(x%81920)*UINT64_C(0x9e3779b97f4a7c15);break;
        case 9:a[i]=(x&255ULL)<<56;break;
        case 10:a[i]=(x&255ULL)<<24;break;
        case 11:a[i]=(x&1023ULL)|UINT64_C(0x8000000000000000);break;
        case 12:a[i]=(x&4095ULL)|UINT64_C(0x7fffffffffff0000);break;
      }
    }
    verify(a);++tests;
  }
  for(std::size_t n:{std::size_t(1000000),std::size_t(2000000)}){
    std::mt19937_64 g(n+6191);std::vector<U> a(n);
    for(auto&x:a)x=g();
    verify(a);++tests;
    for(auto&x:a)x&=UINT64_C(0xffffffff);
    verify(a);++tests;
  }
  std::printf("v21 exact oracle differential PASS %zu input arrays\n",tests);
}
