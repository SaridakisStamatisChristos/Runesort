#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
static int checks=0;
static void test(std::vector<U> a,bool attempt_direct) {
  auto reference=a;std::sort(reference.begin(),reference.end());
  if(attempt_direct){
    auto working=a;
    bool ok=rune::detail::sixth_distinct_reconstruct(working.data(),working.size());
    if((ok&&working!=reference)||(!ok&&working!=a)){
      std::cerr<<"Sixth tier violated sort / failed-call preservation\n";std::exit(8);
    }
  }
  rune::sort(a);
  if(a!=reference){std::cerr<<"Sorting mismatch case "<<checks<<"\n";std::exit(7);}
  ++checks;
}
int main(){
  std::mt19937_64 rng(207104);
  for(size_t n:{size_t(0),size_t(1),size_t(63),size_t(64),
                size_t(65535),size_t(65536),size_t(524287),
                size_t(524288),size_t(1000000)}){
    for(size_t k:{size_t(40960),size_t(73728),size_t(81920),
                  size_t(81921),size_t(90112),size_t(114688),
                  size_t(131072),size_t(131073),size_t(147456)}){
      std::vector<U> keys(k),a(n);
      for(auto&x:keys)x=rng();
      for(auto&x:a)x=keys[rng()%k];
      test(a,n>=524288 && (k==81920||k==90112||k==131072||k==147456));
    }
  }
  for(size_t k:{size_t(131073),size_t(147456)}){
    std::vector<U> keys(k),a(200000);
    for(auto&x:keys)x=rng();
    for(size_t i=0;i<k;++i)a[i]=keys[i];
    for(size_t i=k;i<a.size();++i)a[i]=keys[rng()%k];
    test(a,true);
  }
  // Exact full-width collision witnesses: 33 different keys hash to a
  // single initial sixth-tier bucket and trigger bounded-probe rejection.
  std::vector<U> collision;
  for(U x=0;x<25000000 && collision.size()<33;++x){
    U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;h^=h>>33;
    if((h>>(64-19))==1742)collision.push_back(x);
  }
  if(collision.size()!=33){std::cerr<<"Collision witness missing\n";return 9;}
  std::vector<U>a(500000);
  for(size_t i=0;i<a.size();++i)a[i]=collision[i%collision.size()];
  auto before=a;
  if(rune::detail::sixth_distinct_reconstruct(a.data(),a.size())||a!=before){
    std::cerr<<"Collision rejection mutated input\n";return 10;
  }
  test(std::move(a),false);
  // High-entropy, sparse-bit and misleading-sample cases.
  a.resize(1000000);
  for(auto&x:a)x=rng();test(a,true);
  for(auto&x:a)x=rng()&65535;test(a,true);
  for(size_t i=0;i<64;++i)a[i]=rng();test(a,false);
  std::cout<<"PASSED "<<checks<<" targeted v9 cases\n";
}
