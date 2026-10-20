// RUNE v13 research: external native CPU qualification; dependencies downloaded
// at pinned revisions by CI and remain under their own upstream licenses.
#include "runesort_v13/rune_sort.hpp"
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
#undef rune
#include <ips4o.hpp>
#include <hwy/contrib/sort/vqsort.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=std::uint64_t;
using Clock=std::chrono::steady_clock;
static std::vector<U> generate(size_t n,const std::string& mode,U seed){
  std::mt19937_64 g(seed);
  std::vector<U> a(n);
  if(mode=="uniform64"){for(auto&x:a)x=g();}
  else if(mode=="random32"){for(auto&x:a)x=g()&0xffffffffULL;}
  else if(mode.rfind("sparse",0)==0){
    unsigned bits=static_cast<unsigned>(std::stoul(mode.substr(6)));
    U mask=0;for(unsigned i=0;i<bits;++i)mask|=U(1)<<((i*29+7)&63);
    U common=0xF0F0A50A80000000ULL&~mask;
    for(auto&x:a)x=common|rune::detail::deposit(g(),mask);
  }else if(mode=="K81920"){
    std::vector<U> keys(81920);for(auto&x:keys)x=g();
    for(auto&x:a)x=keys[g()%keys.size()];
  }else std::abort();
  return a;
}
static void sorter(int id,std::vector<U>&a){
  switch(id){
    case 0:rune::sort(a);break;
    case 1:rune_v7::sort(a);break;
    case 2:hwy::VQSort(a.data(),a.size(),hwy::SortAscending{});break;
    case 3:ips4o::sort(a.begin(),a.end());break;
    default:std::abort();
  }
}
int main(int argc,char**argv){
  size_t n=argc>1?std::stoull(argv[1]):1000000;
  constexpr int reps=4;
  const std::array<U,5>seeds={290101,290107,290113,290129,290137};
  const std::array<std::string,8>modes={
    "uniform64","random32","sparse16","sparse17",
    "sparse20","sparse22","sparse23","K81920"};
  const std::array<const char*,4>names={"RUNE_v13","RUNE_v7","VQSort","IPS4o_seq"};
  std::cout<<"workload,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
  for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
    auto input=generate(n,modes[mi],seed+mi*271),oracle=input;
    std::sort(oracle.begin(),oracle.end());
    std::array<std::array<double,reps>,4>time{};
    for(int rep=0;rep<=reps;++rep){
      std::array<int,4>order{0,1,2,3};
      std::mt19937_64 ordergen(seed+mi*777+rep*1039);
      std::shuffle(order.begin(),order.end(),ordergen);
      for(int id:order){
        auto a=input;
        auto t0=Clock::now();sorter(id,a);auto t1=Clock::now();
        if(a!=oracle){std::cerr<<"INCORRECT "<<names[id]<<" "<<modes[mi]<<" "<<n<<"\n";return 3;}
        if(rep)time[id][rep-1]=std::chrono::duration<double,std::milli>(t1-t0).count();
      }
    }
    for(int id=0;id<4;++id){
      auto ts=time[id];std::sort(ts.begin(),ts.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<names[id]<<','
        <<(ts[1]+ts[2])*0.5<<','<<ts[0]<<','<<ts[3]<<",true\n";
    }
    std::cout.flush();
  }
}
