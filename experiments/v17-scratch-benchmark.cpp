#include "runesort_v17/rune_sort.hpp"
#define rune rune_v16
#include "runesort_v16/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=std::uint64_t;using Clock=std::chrono::steady_clock;
static std::vector<U> make(size_t n,const std::string& mode,U seed){
  std::mt19937_64 g(seed);std::vector<U>a(n);
  if(mode=="uniform64")for(auto&x:a)x=g();
  else if(mode=="random32")for(auto&x:a)x=g()&0xffffffffULL;
  else if(mode=="sparse20"){
    U mask=0;for(int j=0;j<20;++j)mask|=U(1)<<((j*29+7)&63);
    for(auto&x:a)x=rune::detail::deposit(g(),mask);
  }else if(mode=="K81920"||mode=="K85000"){
    const size_t K=mode=="K81920"?81920:85000;
    std::vector<U>keys(K);for(auto&x:keys)x=g();
    for(auto&x:a)x=keys[g()%K];
  }else if(mode=="sorted"||mode=="nearly"){
    for(auto&x:a)x=g();
    std::sort(a.begin(),a.end());
    if(mode=="nearly")
      for(size_t i=0;i<n/1000;++i){size_t j=g()%n,k=g()%n;std::swap(a[j],a[k]);}
  }else std::abort();
  return a;
}
int main(int argc,char**argv){
  size_t n=argc>1?std::stoull(argv[1]):1000000;
  constexpr size_t reps=5;
  const std::array<U,5>seeds={61001,61007,61009,61027,61031};
  const std::array<std::string,7>modes={"uniform64","random32","sparse20",
    "K81920","K85000","sorted","nearly"};
  std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
  for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
    auto input=make(n,modes[mi],seed+mi*101),oracle=input;
    std::sort(oracle.begin(),oracle.end());
    std::array<std::array<double,reps>,2>times{};
    std::mt19937_64 order(seed+mi*131);
    for(size_t rep=0;rep<=reps;++rep){
      std::array<int,2>ids{0,1};std::shuffle(ids.begin(),ids.end(),order);
      for(int id:ids){
        auto a=input;auto t0=Clock::now();
        if(id==0)rune::sort(a);else rune_v16::sort(a);
        auto t1=Clock::now();
        if(a!=oracle){std::cerr<<"INCORRECT "<<modes[mi]<<" seed "<<seed<<"\n";return 3;}
        if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(t1-t0).count();
      }
    }
    for(int id=0;id<2;++id){
      auto t=times[id];std::sort(t.begin(),t.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','
        <<(id==0?"RUNE_v17":"RUNE_v16c")<<','
        <<t[2]<<','<<t.front()<<','<<t.back()<<",true\n";
    }
    std::cout.flush();
  }
}
