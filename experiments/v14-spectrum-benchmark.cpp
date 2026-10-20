// Frozen v13 vs PR#5 sparse16 vs combined v14 vs v7. No external sources required.
#include "runesort_v14/rune_sort.hpp"
#define rune rune_v13
#include "runesort_v13/rune_sort.hpp"
#undef rune
#define rune rune_arm16
#include "experiments/v14_baselines/arm16/rune_sort.hpp"
#undef rune
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
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
using U=uint64_t;
using Clock=std::chrono::steady_clock;
static U make_mask(const std::string& s,std::mt19937_64&g){
 U m=0;
 if(s=="spread16")for(unsigned i=0;i<16;++i)m|=U(1)<<((i*29+7)&63);
 else if(s=="spread16b")for(unsigned i=0;i<16;++i)m|=U(1)<<((i*19+11)&63);
 else if(s=="contig16mid")m=((U(1)<<16)-1)<<22;
 else if(s=="contig16high")m=((U(1)<<16)-1)<<48;
 else if(s=="split16")m=U(255)|(U(255)<<48);
 else if(s=="fourbytes16")m=U(15)|(U(15)<<16)|(U(15)<<32)|(U(15)<<48);
 else if(s=="random16")while(rune::detail::popcount(m)<16)m|=U(1)<<(g()%64);
 else if(s=="sparse17")for(unsigned i=0;i<17;++i)m|=U(1)<<((i*29+7)&63);
 else if(s=="sparse20")for(unsigned i=0;i<20;++i)m|=U(1)<<((i*29+7)&63);
 else if(s=="sparse23")for(unsigned i=0;i<23;++i)m|=U(1)<<((i*29+7)&63);
 return m;
}
static std::vector<U> generate(size_t n,const std::string&s,U seed){
 std::mt19937_64 g(seed);
 std::vector<U> a(n);
 if(s=="uniform64"){for(auto&x:a)x=g();return a;}
 if(s=="random32"){for(auto&x:a)x=g()&0xffffffffULL;return a;}
 if(s=="K65536"){
  std::vector<U> keys(65536);
  for(auto&x:keys)x=g();
  for(auto&x:a)x=keys[g()%keys.size()];
  return a;
 }
 U m=make_mask(s,g);
 if(!m)std::abort();
 U fixed=g()&~m;
 for(auto&x:a)x=fixed|rune::detail::deposit(g(),m);
 return a;
}
static void sort_variant(int id,std::vector<U>&a){
 if(id==0)rune::sort(a);
 else if(id==1)rune_v13::sort(a);
 else if(id==2)rune_arm16::sort(a);
 else rune_v7::sort(a);
}
int main(int argc,char**argv){
 size_t n=argc>1?std::stoull(argv[1]):1000000;
 constexpr size_t reps=4;
 const std::array<U,5>seeds={412001,412007,412009,412019,412021};
 const std::array<std::string,13>modes={
  "spread16","spread16b","random16","contig16mid","contig16high",
  "split16","fourbytes16","sparse17","sparse20","sparse23",
  "K65536","random32","uniform64"};
 const std::array<const char*,4>labels={"RUNE_v14","RUNE_v13","RUNE_arm16","RUNE_v7"};
 std::cout<<"workload,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
  auto input=generate(n,modes[mi],seed+mi*271),oracle=input;
  std::sort(oracle.begin(),oracle.end());
  std::array<std::array<double,reps>,4> times{};
  std::mt19937_64 ordergen(seed*65537+mi);
  for(size_t rep=0;rep<=reps;++rep){
   std::array<int,4>order{0,1,2,3};
   std::shuffle(order.begin(),order.end(),ordergen);
   for(int id:order){
    auto a=input;
    auto start=Clock::now();sort_variant(id,a);auto end=Clock::now();
    if(a!=oracle){
     std::cerr<<"INVALID n="<<n<<" mode="<<modes[mi]<<" seed="<<seed<<" id="<<id<<"\n";
     return 3;
    }
    if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(end-start).count();
   }
  }
  for(int id=0;id<4;++id){
   auto z=times[id];std::sort(z.begin(),z.end());
   std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<labels[id]<<','
      <<(z[1]+z[2])*0.5<<','<<z.front()<<','<<z.back()<<",true\n";
  }
  std::cout.flush();
 }
}
