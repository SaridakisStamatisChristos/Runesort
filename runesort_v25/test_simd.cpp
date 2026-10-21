#define RUNE25_TRACE_PATH
#include "rune_sort.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
#if defined(__unix__)
#include <sys/mman.h>
#include <unistd.h>
#endif
using U=std::uint64_t;
static std::size_t cases=0,networks=0;
static void require(bool ok,const char* what){if(!ok){std::fprintf(stderr,"FAIL %s case=%zu\n",what,cases);std::exit(4);}}
static void verify(const std::vector<U>& input,int route=-1){
  auto ref=input;std::sort(ref.begin(),ref.end());
  auto a=input,b=input,c=input,d=input;
  rune_v25::selected_route_calls=0;rune_v25::sort(a);
  if(route>=0)require(rune_v25::selected_route_calls==std::size_t(route && rune_v25::simd_available()),"routing");
  rune_v25::ablation(b.data(),b.size());
  rune_v25::kernel<true>(c.data(),c.size());
  rune_v25::kernel<false>(d.data(),d.size());
  require(a==ref && b==ref && c==ref && d==ref,"full oracle");++cases;
}
int main(){
  rune_v25::sort(nullptr,0);rune_v25::kernel(nullptr,0);
  std::mt19937_64 g(880301);
#if RUNE25_X86
  if(rune_v25::simd_available()){
    // Exhaustive permutations for the 8-lane network.
    U a[8]={0,1,2,3,4,5,6,7},out[8];
    do{rune_v25::detail::small<8>(a,out,8);for(unsigned i=0;i<8;++i)require(out[i]==i,"8-lane permutation");++networks;}
    while(std::next_permutation(a,a+8));
    // Zero-one principle: exhaust every binary vector for the 16-lane network.
    for(unsigned bits=0;bits<65536;++bits){
      U in[16],got[16];for(unsigned i=0;i<16;++i)in[i]=(bits>>i)&1;
      rune_v25::detail::small<16>(in,got,16);std::sort(in,in+16);
      require(std::equal(in,in+16,got),"16-lane zero-one");++networks;
    }
    for(unsigned trial=0;trial<4096;++trial){
      U in[32],got[32];for(auto&x:in)x=(trial&1)?g():g()&1;
      rune_v25::detail::small<32>(in,got,32);std::sort(in,in+32);
      require(std::equal(in,in+32,got),"32-lane network");++networks;
    }
  }
#endif
  for(std::size_t n:{0u,1u,2u,7u,8u,9u,15u,16u,17u,31u,32u,33u,63u,64u,65u,127u,128u,129u,255u,256u,257u,4097u,65536u}){
    for(unsigned mode=0;mode<12;++mode){
      std::vector<U>a(n);
      for(std::size_t i=0;i<n;++i){const U x=g();switch(mode){
        case 0:a[i]=x;break;case 1:a[i]=x&0xffffffffULL;break;
        case 2:a[i]=(x&65535)<<48;break;case 3:a[i]=i;break;
        case 4:a[i]=n-i;break;case 5:a[i]=UINT64_MAX;break;
        case 6:a[i]=(i&1)?0:UINT64_MAX;break;case 7:a[i]=x&1;break;
        case 8:a[i]=(x%85000)*0x9e3779b97f4a7c15ULL;break;
        case 9:a[i]=0xabcdef0000000000ULL|(x&0xffffff);break;
        case 10:a[i]=U(1)<<(i%64);break;case 11:a[i]=(i%101)?0:x;break;
      }}verify(a,0);
    }
  }
  for(std::size_t n:{999999u,1000000u,1000001u,4000000u}){
    std::vector<U>a(n);for(auto&x:a)x=g();verify(a,n>=1000000);
    if(n!=1000000)continue;
    for(auto&x:a)x&=0xffffffffULL;verify(a,0);
    for(auto&x:a)x=g();std::sort(a.begin(),a.begin()+64);verify(a,0);
    for(auto&x:a)x=g()&1;verify(a,0);
    for(std::size_t i=0;i<n;++i)a[i]=i;
    for(unsigned i=0;i<64;++i)a[i]=g();verify(a,1);
  }
  // No masked load/store may touch a protected page past the input tail.
  unsigned guards=0;
#if defined(__unix__)
  const std::size_t page=static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
  void* mapping=mmap(nullptr,3*page,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  require(mapping!=MAP_FAILED,"mmap");auto* middle=static_cast<char*>(mapping)+page;
  require(mprotect(middle,page,PROT_READ|PROT_WRITE)==0,"mprotect");
  for(std::size_t n=1;n<=129;++n){
    auto* a=reinterpret_cast<U*>(middle+page)-n;
    for(std::size_t i=0;i<n;++i)a[i]=g();std::vector<U>ref(a,a+n);
    std::sort(ref.begin(),ref.end());rune_v25::kernel(a,n);
    require(std::equal(ref.begin(),ref.end(),a),"guarded tail");++guards;
  }
  require(munmap(mapping,3*page)==0,"munmap");
#endif
  std::printf("v25 OK: simd=%d, %zu exact arrays, %zu network cases, %u protected-page tails\n",
      rune_v25::simd_available(),cases,networks,guards);
}
