// RUNE v20: honest exact same-array comparison against pinned external sorters.
// External sources are fetched at immutable git revisions by CI, not vendored.
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
 std::mt19937_64 g(seed);std::vector<U> a(n);
 if(mode=="uniform64"){for(auto&x:a)x=g();}
 else if(mode=="random32"){for(auto&x:a)x=g() & UINT64_C(0xffffffff);}
 else if(mode=="K85000"){
  std::vector<U> keys(85000);for(auto&x:keys)x=g();
  for(auto&x:a)x=keys[g()%keys.size()];
 }else if(mode=="sorted"){for(auto&x:a)x=g();std::sort(a.begin(),a.end());}
 else if(mode=="alternating"){for(size_t i=0;i<n;++i)a[i]=(i&1)?0:UINT64_MAX;}
 else std::abort();
 return a;
}
static void sorter(unsigned id,std::vector<U>& a){
 switch(id){
  case 0:rune_v17::sort(a);break;
  case 1:rune_v17::detail::fixed_radix<8>(a.data(),a.size());break;
  case 2:hwy::VQSort(a.data(),a.size(),hwy::SortAscending{});break;
  case 3:ips4o::sort(a.begin(),a.end());break;
  case 4:std::sort(a.begin(),a.end());break;
  default:std::abort();
 }
}
int main(int argc,char**argv){
 const size_t n=argc>1?std::stoull(argv[1]):1000000;
 constexpr unsigned reps=3;
 const std::array<U,5> seeds{20261121,20261127,20261129,20261147,20261153};
 const std::array<std::string,5> modes{"uniform64","random32","K85000","sorted","alternating"};
 const std::array<const char*,5> names{"RUNE17_full","RUNE17_fixed8","VQSort","IPS4o_seq","std_sort"};
 std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
  auto input=generate(n,modes[mi],seed+mi*191),oracle=input;
  std::sort(oracle.begin(),oracle.end());
  std::array<std::array<double,reps>,5> times{};
  std::mt19937_64 order_rng(seed+mi*373);
  for(unsigned rep=0;rep<=reps;++rep){
   std::array<unsigned,5> order{0,1,2,3,4};
   std::shuffle(order.begin(),order.end(),order_rng);
   for(unsigned id:order){
    auto a=input;
    auto t0=Clock::now();
    sorter(id,a);
    auto t1=Clock::now();
    if(a!=oracle){
     std::cerr<<"INCORRECT "<<names[id]<<" "<<modes[mi]<<" "<<n<<" "<<seed<<"\n";
     return 3;
    }
    if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(t1-t0).count();
   }
  }
  for(unsigned id=0;id<5;++id){
   auto z=times[id];std::sort(z.begin(),z.end());
   std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<names[id]<<','
    <<std::fixed<<std::setprecision(5)<<z[1]<<','<<z.front()<<','
    <<z.back()<<",true\n";
  }
  std::cout.flush();
 }
}
