#include "runesort_v13/rune_sort.hpp"
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
static U mask_for(const std::string&mode,std::mt19937_64&g){
 U m=0;
 if(mode.rfind("spread",0)==0){
   unsigned b=static_cast<unsigned>(std::stoul(mode.substr(6)));
   for(unsigned i=0;i<b;++i)m|=U(1)<<((i*19+11)&63);
 }else if(mode=="contig17high")m=((U(1)<<17)-1)<<40;
 else if(mode=="contig20mid")m=((U(1)<<20)-1)<<21;
 else if(mode=="contig23high")m=((U(1)<<23)-1)<<40;
 else if(mode=="contig23low")m=(U(1)<<23)-1;
 else if(mode=="block17")m= ((U(1)<<8)-1)|(((U(1)<<9)-1)<<50);
 else if(mode=="block20")m= ((U(1)<<10)-1)|(((U(1)<<10)-1)<<51);
 else if(mode=="nearbyte20")m=U(255)|(U(255)<<16)|(U(15)<<32);
 else if(mode=="stagger20")for(unsigned i=0;i<20;++i)m|=U(1)<<(2*i);
 else if(mode=="random20")while(rune::detail::popcount(m)<20)m|=U(1)<<(g()%64);
 else std::abort();
 return m;
}
int main(int argc,char**argv){
 size_t n=argc>1?std::stoull(argv[1]):1000000;
 constexpr size_t reps=4;
 const std::array<U,5>seeds={319001,319007,319021,319031,319037};
 const std::array<std::string,12>modes={
 "spread17","spread20","spread23","contig17high","contig20mid",
 "contig23high","contig23low","block17","block20",
 "nearbyte20","stagger20","random20"};
 std::cout<<"workload,n,seed,mask,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
   std::mt19937_64 g(seed+mi*317);
   U mask=mask_for(modes[mi],g);
   U constant=g()&~mask;
   std::vector<U>input(n);
   for(auto&x:input)x=constant|rune::detail::deposit(g(),mask);
   auto oracle=input;std::sort(oracle.begin(),oracle.end());
   std::array<std::array<double,reps>,2>times{};
   std::mt19937_64 ord(seed*65537+mi);
   for(size_t rep=0;rep<=reps;++rep){
     std::array<int,2>order{0,1};std::shuffle(order.begin(),order.end(),ord);
     for(int id:order){
       auto a=input;auto st=Clock::now();
       if(id==0)rune::sort(a);else rune_v7::sort(a);
       auto end=Clock::now();
       if(a!=oracle){std::cerr<<"INCORRECT "<<modes[mi]<<" n="<<n<<" seed="<<seed<<"\n";return 3;}
       if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(end-st).count();
     }
   }
   for(int id=0;id<2;++id){
      auto ts=times[id];std::sort(ts.begin(),ts.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<mask<<','
        <<(id==0?"RUNE_v13":"RUNE_v7")<<','
        <<(ts[1]+ts[2])*0.5<<','<<ts[0]<<','<<ts[3]<<",true\n";
   }std::cout.flush();
 }
}
