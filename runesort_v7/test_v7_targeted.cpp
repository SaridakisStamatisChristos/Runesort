#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <cstdlib>
using U=uint64_t;
static unsigned long long checks=0;
static void verify(std::vector<U> a,bool direct=true){
  auto original=a,expected=a;std::sort(expected.begin(),expected.end());
  if(direct){bool ok=rune::detail::fifth_distinct_reconstruct(a.data(),a.size());
    if(ok&&a!=expected){std::cerr<<"Bad accepted fifth reconstruction\n";std::exit(5);}
    if(!ok&&a!=original){std::cerr<<"Rejected reconstruction changed input\n";std::exit(6);}
    a=original;
  }
  rune::sort(a);
  if(a!=expected){std::cerr<<"Sort wrong at case "<<checks<<"\n";std::exit(7);}
  ++checks;
}
int main(){
  std::mt19937_64 rng(0x52756e65000FULL);
  for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(63),size_t(64),size_t(65),size_t(4096),size_t(65536),size_t(262143),size_t(262144),size_t(524287),size_t(524288),size_t(525000)}){
    for(size_t k:{size_t(1),size_t(2),size_t(16),size_t(4096),size_t(16384),size_t(32767),size_t(32768),size_t(32769),size_t(40960),size_t(65535),size_t(65536),size_t(65537),size_t(73728),size_t(81920),size_t(81921),size_t(98304)}){
      std::vector<U> keys(k),a(n);for(auto&x:keys)x=rng();for(auto&x:a)x=keys[rng()%k];
      verify(a,n>=4096);
      if(n>=65536){for(size_t i=0;i<std::min(k,n);++i)a[i]=keys[i];verify(a,false);}
    }
    std::vector<U>a(n);for(auto&x:a)x=rng();verify(a);
    for(auto&x:a)x=rng()&65535;verify(a);
    std::sort(a.begin(),a.end());verify(a);
    if(n) {std::rotate(a.begin(),a.begin()+n/3,a.end());verify(a);}
  }
  for(size_t k:{size_t(40960),size_t(65536),size_t(73728),size_t(81920),size_t(81921),size_t(98304),size_t(0)}){
    size_t n=1000000;std::vector<U>a(n),keys(k);
    for(auto&x:keys)x=rng();if(k)for(auto&x:a)x=keys[rng()%k];else for(auto&x:a)x=rng();
    verify(a,true);for(size_t i=0;i<1000;++i)a[n-i-1]=rng();verify(a,true);
  }
  std::vector<U> collision;
  for(U x=0;collision.size()<33&&x<30000000;++x){U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;h^=h>>33;if((h>>(64-18))==913)collision.push_back(x);}
  if(collision.size()!=33){std::cerr<<"Failed to construct collision witness\n";return 9;}
  std::vector<U>a(800000);for(size_t i=0;i<a.size();++i)a[i]=collision[i%33];
  auto initial=a;
  if(rune::detail::fifth_distinct_reconstruct(a.data(),a.size())||a!=initial){std::cerr<<"Collision rejection modified input\n";return 10;}
  verify(std::move(a),false);
  std::cout<<"PASSED "<<checks<<" fifth-tier adversarial cases\n";
}
