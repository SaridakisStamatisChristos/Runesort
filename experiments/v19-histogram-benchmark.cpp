#include "runesort_v19/compact_histogram.hpp"
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
static std::vector<U> generate(size_t n,const std::string& mode,U seed){
 std::mt19937_64 rng(seed);std::vector<U> v(n);
 if(mode=="uniform64")for(auto&x:v)x=rng();
 else if(mode=="random32")for(auto&x:v)x=rng()&0xffffffffULL;
 else if(mode=="K85000"){std::vector<U> keys(85000);for(auto&x:keys)x=rng();for(auto&x:v)x=keys[rng()%keys.size()];}
 else if(mode=="sorted"){for(auto&x:v)x=rng();std::sort(v.begin(),v.end());}
 else if(mode=="alternating"){for(size_t i=0;i<n;++i)v[i]=(i&1)?0:UINT64_MAX;}
 else std::abort();
 return v;
}
static void run(int id,U* a,size_t n){
 if(id==0)rune_v17::detail::fixed_radix<8>(a,n);
 else if(id==1)rune_v19::fused32(a,n);
 else if(id==2)rune_v19::perpass32(a,n);
 else std::abort();
}
int main(int argc,char**argv){
 const size_t n=argc>1?std::stoull(argv[1]):1000000;
 constexpr int reps=3;
 const std::array<U,5> seeds{82103,82111,82117,82129,82133};
 const std::array<std::string,5> modes{"uniform64","random32","K85000","sorted","alternating"};
 const std::array<const char*,3> names{"v17_fixed8","fused32","perpass32"};
 std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
  auto input=generate(n,modes[mi],seed+mi*137),oracle=input;
  std::sort(oracle.begin(),oracle.end());
  std::array<std::array<double,reps>,3> times{};
  std::mt19937_64 gen(seed+mi*139);
  for(int rep=0;rep<=reps;++rep){
   std::array<int,3> ids{0,1,2};std::shuffle(ids.begin(),ids.end(),gen);
   for(int id:ids){
    auto a=input;auto t0=Clock::now();
    run(id,a.data(),a.size());
    auto t1=Clock::now();
    if(a!=oracle){std::cerr<<"INCORRECT "<<modes[mi]<<' '<<n<<' '<<seed<<' '<<id<<"\n";return 4;}
    if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(t1-t0).count();
   }
  }
  for(int id=0;id<3;++id){
   auto t=times[id];std::sort(t.begin(),t.end());
   std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<names[id]<<','
       <<t[1]<<','<<t.front()<<','<<t.back()<<",true\n";
  }
  std::cout.flush();
 }
}
