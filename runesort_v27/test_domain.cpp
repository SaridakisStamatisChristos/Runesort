#define RUNE27_TRACE_PATH
#include "rune_sort.hpp"
#include "byte_msd.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
#if defined(__unix__)
#include <sys/mman.h>
#include <unistd.h>
#endif
using U=std::uint64_t;
static unsigned cases=0;
static void require(bool ok,const char* msg) {
  if (!ok) { std::fprintf(stderr,"FAIL case=%u %s\n",cases,msg);std::exit(4); }
}
static U mask(unsigned bits,unsigned offset=7) {
  U m=0;for(unsigned i=0;i<bits;++i)m|=U(1)<<((i*29+offset)&63);return m;
}
static void check(const std::vector<U>& input,int route=-1) {
  auto oracle=input;std::sort(oracle.begin(),oracle.end());
  auto actual=input;
  rune_v27::detail::compact_calls=0;
  rune_v27::sort(actual);
  require(actual==oracle,"exact public oracle");
  if(route>=0)require(rune_v27::detail::compact_calls==unsigned(route),"route");
  actual=input;
  bool accepted=rune_v27::detail::compact_count(actual.data(),actual.size());
  require(actual==(accepted?oracle:input),"trial preserves input on rejection");
  // Worker is an independent candidate, including its portable leaf fallback.
  auto worker=input;
  rune_v27::experimental::run<128,8>(worker);
  require(worker==oracle,"exact experimental MSD oracle");
  ++cases;
}
int main() {
  rune_v27::sort(nullptr,0);rune_v27::sort(nullptr,1);
  require(!rune_v27::detail::compact_count(nullptr,0),"empty trial");
  if(sizeof(std::size_t)>4)
    require(!rune_v27::detail::compact_count(nullptr,std::size_t(UINT32_MAX)+1),"counter bound before dereference");
  std::mt19937_64 g(703801);
  for(std::size_t n:{0u,1u,2u,7u,31u,32u,33u,63u,64u,65u,127u,128u,129u,
                     255u,256u,257u,4097u,65535u,65536u,65537u,131072u}) {
    for(unsigned mode=0;mode<10;++mode) {
      std::vector<U>a(n);
      for(std::size_t i=0;i<n;++i) {
        const U x=g();
        switch(mode) {
          case 0:a[i]=x;break;case 1:a[i]=x&0xffff;break;
          case 2:a[i]=(x&0xffff)<<48;break;case 3:a[i]=i;break;
          case 4:a[i]=n-i;break;case 5:a[i]=UINT64_MAX;break;
          case 6:a[i]=(i&1)?0:UINT64_MAX;break;
          case 7:a[i]=0xffff000000000000ULL|(x&0xfffff);break;
          case 8:a[i]=x&mask(20);break;case 9:a[i]=(i%101)?0:x;break;
        }
      }
      check(a);
    }
  }
  // Sweep domain boundaries; constants may occupy bit 63 and holes in the mask.
  for(unsigned bits:{11u,12u,16u,17u,20u,21u}) {
    for(bool contiguous:{false,true}) {
      U m=contiguous?((U(1)<<bits)-1)<<31:mask(bits);
      const U constant=g()&~m;
      std::vector<U>a(600001);
      for(auto&x:a)x=constant|(g()&m);
      const int route=(bits>=12 && bits<=20 && (contiguous||rune_v27::detail::bmi2_available()))?1:0;
      check(a,route);
    }
  }
  // False sample: extra bits appear only outside all three inspected windows.
  for(std::size_t outlier:{64u,4095u,4096u,123456u,599900u}) {
    std::vector<U>a(600001);for(auto&x:a)x=g()&0xffff;
    a[outlier]=UINT64_MAX;check(a,0);
  }
  // Reject unsuitable table density and preserve late certificate failures.
  std::vector<U>a(65536);for(auto&x:a)x=g()&0xfffff;check(a,0);
  a.resize(600001);for(auto&x:a)x=g()&0xffff;
  std::sort(a.begin(),a.end());check(a,0);
  std::reverse(a.begin(),a.end());check(a,0);
  for(auto&x:a)x=g();std::sort(a.begin()+64,a.end());check(a,0);
  unsigned guards=0;
#if defined(__unix__)
  const auto page=std::size_t(sysconf(_SC_PAGESIZE));
  const std::size_t bytes=((65537*sizeof(U)+page-1)/page)*page;
  void* memory=mmap(nullptr,bytes+2*page,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
  require(memory!=MAP_FAILED,"mmap");
  auto* usable=static_cast<char*>(memory)+page;
  require(mprotect(usable,bytes,PROT_READ|PROT_WRITE)==0,"mprotect");
  for(std::size_t n:{1u,31u,32u,33u,127u,128u,129u,65535u,65536u,65537u}) {
    for(bool left:{false,true}) {
      U* p=left?reinterpret_cast<U*>(usable):reinterpret_cast<U*>(usable+bytes)-n;
      for(std::size_t i=0;i<n;++i)p[i]=(g()&mask(17))|(~mask(17)&0xf0f0f0f0f0f0f0f0ULL);
      std::vector<U>oracle(p,p+n);std::sort(oracle.begin(),oracle.end());
      rune_v27::sort(p,n);
      require(std::equal(oracle.begin(),oracle.end(),p),"protected public boundary");++guards;
    }
  }
#if RUNE27_BMI2
  if(rune_v27::detail::simd_reconstruct_available()) {
    for(std::size_t n=1;n<=129;++n) {
      auto* p=reinterpret_cast<U*>(usable+bytes)-n;
      std::vector<std::uint32_t> counts(16,0);counts[15]=std::uint32_t(n);
      rune_v27::detail::reconstruct_simd(p,counts,15,UINT64_MAX&~U(15));
      require(std::all_of(p,p+n,[](U x){return x==UINT64_MAX;}),"masked reconstruction tail");++guards;
    }
  }
#endif
  require(munmap(memory,bytes+2*page)==0,"munmap");
#endif
  std::printf("v27 OK: bmi2=%d, simd=%d, %u exact arrays, %u protected boundaries\n",
              rune_v27::detail::bmi2_available(),rune_v25::simd_available(),cases,guards);
}
