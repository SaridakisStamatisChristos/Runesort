#define RUNE24_TRACE_PATH
#include "rune_sort.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
using U=std::uint64_t;
static size_t cases=0, routed=0;
static void verify(const std::vector<U>& input, int expected_route=-1){
  auto ref=input, candidate=input, control=input, frozen=input;
  std::sort(ref.begin(),ref.end());
  rune_v24::prefetch_route_calls=0;
  rune_v24::sort(candidate);
  const auto calls=rune_v24::prefetch_route_calls;
  if(expected_route>=0 && calls!=size_t(expected_route && rune_v24::x86_dispatch_enabled)){
    std::fprintf(stderr,"route mismatch case=%zu n=%zu expected=%d actual=%zu\n",cases,input.size(),expected_route,calls);
    std::exit(5);
  }
  routed+=calls;
  rune_v24::ablation0(control.data(),control.size());
  rune_v17::sort(frozen);
  if(candidate!=ref || control!=ref || frozen!=ref){
    std::fprintf(stderr,"exact oracle mismatch n=%zu\n",input.size());std::exit(4);
  }
  ++cases;
}
int main(){
  rune_v24::sort(nullptr,0);rune_v24::ablation0(nullptr,0);
  for(size_t n:{0u,1u,2u,63u,64u,65u,127u,128u,129u,65535u,65536u,65537u}){
    for(unsigned mode=0;mode<10;++mode){
      std::mt19937_64 g(331+n*157+mode*1193);std::vector<U>a(n);
      for(size_t i=0;i<n;++i){const U x=g();switch(mode){
        case 0:a[i]=x;break;case 1:a[i]=x&0xffffffffULL;break;
        case 2:a[i]=x&65535;break;case 3:a[i]=i;break;
        case 4:a[i]=n-i;break;case 5:a[i]=UINT64_MAX;break;
        case 6:a[i]=(i&1)?0:UINT64_MAX;break;case 7:a[i]=x%3;break;
        case 8:a[i]=(x%85000)*0x9e3779b97f4a7c15ULL;break;
        case 9:a[i]=(x&65535)<<48;break;
      }}verify(a,0);
    }
  }
  // Exercise both sides of the exact gate, including offset pointers/canaries.
  for(size_t n:{999999u,1000000u,1000001u,4000000u}){
    std::mt19937_64 g(812093+n);std::vector<U>a(n);
    for(auto&x:a)x=g();verify(a,n>=1000000);
    std::vector<U> guarded(n+2,0xdeadbeefcafebabeULL);
    std::copy(a.begin(),a.end(),guarded.begin()+1);
    auto ref=guarded;std::sort(ref.begin()+1,ref.end()-1);
    rune_v24::sort(guarded.data()+1,n);
    if(guarded!=ref)std::exit(6);
    if(n!=1000000)continue;
    for(auto&x:a)x&=0xffffffffULL;verify(a,0);
    for(auto&x:a)x=(g()&65535)<<48;verify(a,0);
    for(size_t i=0;i<n;++i)a[i]=i;verify(a,0);
    std::reverse(a.begin(),a.end());verify(a,0);
    for(size_t i=0;i<n;++i)a[i]=(i&1)?0:UINT64_MAX;verify(a,0);
    for(auto&x:a)x=42;verify(a,0);
    for(auto&x:a)x=g();std::sort(a.begin(),a.begin()+64);verify(a,0);
    // False low-cardinality and equal samples must still yield exact results.
    for(size_t i=0;i<64;++i)a[i]=0;verify(a,0);
    for(size_t i=0;i<64;++i)a[i]=(i&1)?0:UINT64_MAX;verify(a,1);
    // An entropy-heavy prefix can route an otherwise ordered tail safely.
    for(size_t i=0;i<n;++i)a[i]=i;
    for(size_t i=0;i<64;++i)a[i]=g();verify(a,1);
  }
  std::printf("v24 exact differential OK: %zu arrays, %zu selected routes, 4 guarded subarrays\n",cases,routed);
}
