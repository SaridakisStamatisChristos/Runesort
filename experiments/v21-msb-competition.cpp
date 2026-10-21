// Direct same-run RUNE v21 vs frozen v17 and exact pinned independent competitors.
#include "runesort_v21/msb_partition.hpp"
#define rune rune_v17
#include "runesort_v17/rune_sort.hpp"
#undef rune
#include <hwy/contrib/sort/vqsort.h>
#include <ips4o.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=std::uint64_t;using Clock=std::chrono::steady_clock;
static std::vector<U> generate(size_t n,const std::string& mode,U seed){
  std::mt19937_64 rng(seed);std::vector<U> v(n);
  if(mode=="uniform64")for(auto&x:v)x=rng();
  else if(mode=="random32")for(auto&x:v)x=rng()&UINT64_C(0xffffffff);
  else if(mode=="K85000"){
    std::vector<U> keys(85000);for(auto&x:keys)x=rng();
    for(auto&x:v)x=keys[rng()%keys.size()];
  }else if(mode=="sorted"){
    for(auto&x:v)x=rng();std::sort(v.begin(),v.end());
  }else if(mode=="alternating"){
    for(size_t i=0;i<n;++i)v[i]=i&1?0:UINT64_MAX;
  }else std::abort();
  return v;
}
static void run(unsigned id,std::vector<U>& a){
  switch(id){
    case 0:rune_v17::sort(a);break;
    case 1:rune_v17::detail::fixed_radix<8>(a.data(),a.size());break;
    case 2:rune_v21::bucket_radix7(a.data(),a.size());break;
    case 3:rune_v21::bucket_stdsort(a.data(),a.size());break;
    case 4:hwy::VQSort(a.data(),a.size(),hwy::SortAscending{});break;
    case 5:ips4o::sort(a.begin(),a.end());break;
    case 6:std::sort(a.begin(),a.end());break;
    default:std::abort();
  }
}
int main(int argc,char**argv){
  const size_t n=argc>1?std::stoull(argv[1]):1000000;
  constexpr unsigned reps=3;
  const std::array<U,5> seeds{913103,913109,913117,913123,913129};
  const std::array<std::string,5> modes{"uniform64","random32","K85000","sorted","alternating"};
  const std::array<const char*,7> names{
    "RUNE17_full","RUNE17_fixed8","RUNE21_bucket_radix7",
    "RUNE21_bucket_stdsort","VQSort","IPS4o_seq","std_sort"};
  std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
  for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
    const auto original=generate(n,modes[mi],seed+mi*173);
    auto oracle=original;std::sort(oracle.begin(),oracle.end());
    std::array<std::array<double,reps>,7> timings{};
    std::mt19937_64 rng(seed+mi*271);
    for(unsigned rep=0;rep<=reps;++rep){
      std::array<unsigned,7> order{0,1,2,3,4,5,6};
      std::shuffle(order.begin(),order.end(),rng);
      for(unsigned id:order){
        auto a=original;
        const auto t0=Clock::now();run(id,a);const auto t1=Clock::now();
        if(a!=oracle){
          std::cerr<<"INCORRECT "<<names[id]<<" "<<modes[mi]<<" "<<n<<" "<<seed<<"\n";return 4;
        }
        if(rep)timings[id][rep-1]=
            std::chrono::duration<double,std::milli>(t1-t0).count();
      }
    }
    for(unsigned id=0;id<7;++id){
      auto t=timings[id];std::sort(t.begin(),t.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<names[id]<<','
        <<std::fixed<<std::setprecision(5)<<t[1]<<','<<t.front()<<','
        <<t.back()<<",true\n";
    }
    std::cout.flush();
  }
}
