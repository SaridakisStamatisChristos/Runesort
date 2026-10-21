#include "runesort_v22/radix16.hpp"
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
static std::vector<U> generate(std::size_t n,const std::string& mode,U seed){
 std::mt19937_64 g(seed);std::vector<U> a(n);
 if(mode=="uniform64")for(auto&v:a)v=g();
 else if(mode=="random32")for(auto&v:a)v=g()&UINT64_C(0xffffffff);
 else if(mode=="K85000"){
   std::vector<U> keys(85000);for(auto&v:keys)v=g();
   for(auto&v:a)v=keys[g()%keys.size()];
 }else if(mode=="sorted"){
   for(auto&v:a)v=g();std::sort(a.begin(),a.end());
 }else if(mode=="alternating"){
   for(std::size_t i=0;i<n;++i)a[i]=(i&1)?0:UINT64_MAX;
 }else std::abort();
 return a;
}
static void sorter(unsigned id,std::vector<U>& a){
 switch(id){
  case 0:rune_v17::sort(a);break;
  case 1:rune_v17::detail::fixed_radix<8>(a.data(),a.size());break;
  case 2:rune_v22::fused16(a.data(),a.size());break;
  case 3:rune_v22::perpass16(a.data(),a.size());break;
  case 4:hwy::VQSort(a.data(),a.size(),hwy::SortAscending{});break;
  case 5:ips4o::sort(a.begin(),a.end());break;
  default:std::abort();
 }
}
int main(int argc,char**argv){
 const auto n=argc>1?std::stoull(argv[1]):1000000ULL;
 constexpr std::size_t R=3;
 const std::array<U,5> seeds{310901,310907,310913,310919,310927};
 const std::array<std::string,5> modes{"uniform64","random32","K85000","sorted","alternating"};
 const std::array<const char*,6> names{
   "RUNE17_full","RUNE17_fixed8","RUNE22_fused16","RUNE22_perpass16","VQSort","IPS4o_seq"};
 std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(std::size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
   auto original=generate(n,modes[mi],seed+mi*157),oracle=original;
   std::sort(oracle.begin(),oracle.end());
   std::array<std::array<double,R>,6> times{};
   std::mt19937_64 order(seed+mi*397);
   for(std::size_t rep=0;rep<=R;++rep){
     std::array<unsigned,6> ids{0,1,2,3,4,5};
     std::shuffle(ids.begin(),ids.end(),order);
     for(unsigned id:ids){
       auto a=original;
       auto t0=Clock::now();sorter(id,a);
       auto t1=Clock::now();
       if(a!=oracle){
         std::cerr<<"INCORRECT "<<names[id]<<" mode="<<modes[mi]<<" n="<<n<<" seed="<<seed<<"\n";
         return 5;
       }
       if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(t1-t0).count();
     }
   }
   for(unsigned id=0;id<6;++id){
     auto t=times[id];std::sort(t.begin(),t.end());
     std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<names[id]<<','
       <<std::fixed<<std::setprecision(5)<<t[1]<<','<<t.front()<<','
       <<t.back()<<",true\n";
   }
   std::cout.flush();
 }
}
