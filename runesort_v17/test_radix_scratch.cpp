#include "rune_sort.hpp"
#define rune rune_v16
#include "runesort_v16/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
using U=std::uint64_t;
static void check(const std::vector<U>& a){
  auto oracle=a; std::sort(oracle.begin(),oracle.end());
  auto worker=a,candidate=a,baseline=a;
  rune::detail::fixed_radix<8>(worker.data(),worker.size());
  rune::sort(candidate);
  rune_v16::sort(baseline);
  if(worker!=oracle || candidate!=oracle || baseline!=oracle){
    std::fputs("FAIL: worker/v17/v16 diverged from std::sort\n",stderr);
    std::exit(3);
  }
}
int main(){
  unsigned tests=0;
  for(size_t n:{size_t(2),size_t(3),size_t(7),size_t(31),size_t(63),size_t(64),
                size_t(65),size_t(1023),size_t(4096),size_t(65536),size_t(262144)}){
    for(unsigned mode=0;mode<8;++mode){
      std::mt19937_64 g(871823u+n*37u+mode*991u);
      std::vector<U> a(n);
      for(size_t i=0;i<n;++i)switch(mode){
        case 0:a[i]=g();break;
        case 1:a[i]=g()&0xffffffffULL;break;
        case 2:a[i]=(g()&0xffffULL)<<48;break;
        case 3:a[i]=U(i);break;
        case 4:a[i]=U(n-i);break;
        case 5:a[i]=std::numeric_limits<U>::max();break;
        case 6:a[i]=i&1?0:std::numeric_limits<U>::max();break;
        case 7:a[i]=(g()%19)*0x9e3779b97f4a7c15ULL;break;
      }
      check(a); ++tests;
    }
  }
  for(size_t K:{size_t(81920),size_t(85000)}){
    std::mt19937_64 g(991001+K);
    std::vector<U> keys(K),a(2000000);
    for(auto&x:keys)x=g();
    for(auto&x:a)x=keys[g()%K];
    check(a);++tests;
  }
  std::printf("PASS %u differential cases\n",tests);
}
