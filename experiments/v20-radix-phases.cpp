// RUNE v20 observational diagnostic: clock boundaries inside a copied fixed8
// worker. This is INSTRUMENTED and must not be compared as a production sort.
#define rune rune_v17
#include "runesort_v17/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
using U=std::uint64_t;using Clock=std::chrono::steady_clock;
using Stamp=Clock::time_point;
static double ms(Stamp a, Stamp b) {
 return std::chrono::duration<double,std::milli>(b-a).count();
}
static std::vector<U> generate(std::size_t n,const std::string& mode,U seed){
 std::mt19937_64 g(seed);std::vector<U> x(n);
 if(mode=="uniform64")for(auto&v:x)v=g();
 else if(mode=="random32")for(auto&v:x)v=g()&UINT64_C(0xffffffff);
 else std::abort();
 return x;
}
int main(int argc,char**argv){
 const std::size_t n=argc>1?std::stoull(argv[1]):1000000;
 constexpr std::array<U,3> seeds{20261121,20261127,20261129};
 std::cout<<"mode,n,seed,phase,digit,ms,correct\n";
 for(const std::string mode:{"uniform64","random32"})for(U seed:seeds){
  auto input=generate(n,mode,seed),oracle=input;
  std::sort(oracle.begin(),oracle.end());
  auto a=input;
  auto t0=Clock::now();
  std::size_t h[8][256]{};
  for(std::size_t i=0;i<n;++i){
   U v=a[i];
   for(unsigned d=0;d<8;++d)++h[d][(v>>(8*d))&255u];
  }
  auto t1=Clock::now();
  for(unsigned d=0;d<8;++d){
   std::size_t p=0;
   for(unsigned k=0;k<256;++k){auto z=h[d][k];h[d][k]=p;p+=z;}
  }
  auto t2=Clock::now();
  std::unique_ptr<U[]> memory(new U[n]);
  auto t3=Clock::now();
  U *src=a.data(),*dst=memory.get();
  std::array<double,8> scatters{};
  for(unsigned d=0;d<8;++d){
   auto start=Clock::now();
   for(std::size_t i=0;i<n;++i){
    U v=src[i];dst[h[d][(v>>(d*8))&255u]++]=v;
   }
   auto stop=Clock::now();
   scatters[d]=ms(start,stop);
   std::swap(src,dst);
  }
  auto t4=Clock::now();
  if(src!=a.data())std::memcpy(a.data(),src,n*sizeof(U));
  auto t5=Clock::now();
  if(a!=oracle){std::cerr<<"phase diagnostic lost an item\n";return 4;}
  auto emit=[&](const char* name,int digit,double value){
   std::cout<<mode<<','<<n<<','<<seed<<','<<name<<','<<digit<<','<<value<<",true\n";
  };
  emit("histogram",-1,ms(t0,t1));
  emit("prefix",-1,ms(t1,t2));
  emit("scratch_alloc",-1,ms(t2,t3));
  for(unsigned d=0;d<8;++d)emit("scatter",int(d),scatters[d]);
  emit("scatter_total",-1,ms(t3,t4));
  emit("final_copy",-1,ms(t4,t5));
  emit("total",-1,ms(t0,t5));
  std::cout.flush();
 }
}
