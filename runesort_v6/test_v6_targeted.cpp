#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <cstdlib>
using U=uint64_t;
static unsigned long long cases=0;
static void verify(std::vector<U> a, bool direct) {
  auto before=a, expected=a;
  std::sort(expected.begin(),expected.end());
  if(direct){
    bool accepted=rune::detail::fourth_distinct_reconstruct(a.data(),a.size());
    if(accepted && a!=expected){std::cerr<<"bad accepted fourth tier\n";std::exit(3);}
    if(!accepted && a!=before){std::cerr<<"fourth tier modified on rejection\n";std::exit(4);}
    a=before;
  }
  rune::sort(a);
  if(a!=expected){std::cerr<<"bad full sort case "<<cases<<"\n";std::exit(5);}
  ++cases;
}
int main(){
  std::mt19937_64 r(0x9fa702024ULL);
  for(size_t n:{size_t(65),size_t(65536),size_t(262143),
                size_t(262144),size_t(300000),size_t(1000000)}){
    for(size_t k:{size_t(4096),size_t(16384),size_t(24576),
                  size_t(32767),size_t(32768),size_t(32769),
                  size_t(49152),size_t(65536)}){
      std::vector<U> keys(k);
      for(auto&x:keys)x=r();
      std::vector<U>a(n);
      for(int rep=0;rep<(n>=300000?1:2);++rep){
        for(auto&x:a)x=keys[r()%k];
        if(rep==1){for(size_t i=0;i<std::min(k,n);++i)a[i]=keys[i];}
        verify(a,rep==0 && n>=65536);
      }
    }
  }
  for(int rep=0;rep<2;++rep){
    std::vector<U>a(300000);
    U base=r();for(auto&x:a)x=base;
    for(size_t i=64;i<a.size();++i)a[i]=r();
    verify(a,true);
  }
  std::vector<U> collision;collision.reserve(33);
  for(U x=0;x<12000000 && collision.size()<33;++x){
    U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;
    h^=h>>33;
    if((h>>(64-17))==913)collision.push_back(x);
  }
  if(collision.size()!=33){std::cerr<<"collision construction failed\n";return 6;}
  std::vector<U>a(300000);
  for(size_t i=0;i<a.size();++i)a[i]=collision[i%33];
  auto original=a;
  if(rune::detail::fourth_distinct_reconstruct(a.data(),a.size()) || a!=original){
    std::cerr<<"collision rejection failed closed\n";return 7;
  }
  verify(std::move(a),false);
  std::cout<<"PASSED "<<cases<<" v6 targeted cases\n";
}
