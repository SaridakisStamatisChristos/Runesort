// Mechanism attribution: identical inputs, direct radix8 vs direct radix11 vs full dispatch.
#include "runesort_v15/rune_sort.hpp"
#define rune rune_v14
#include "runesort_v14/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
using U=std::uint64_t;
using Clock=std::chrono::steady_clock;
int main(int argc,char**argv){
 const size_t n=argc>1?std::stoull(argv[1]):1000000;
 const std::array<U,5>seeds={77531,77541,77549,77551,77557};
 const char*names[]={"v14_dispatch","v15_dispatch","direct_8byte","direct_11bit"};
 std::cout<<"n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(U seed:seeds){
  std::mt19937_64 gen(seed);
  std::vector<U>a(n);
  for(auto&x:a)x=gen();
  auto expected=a;std::sort(expected.begin(),expected.end());
  std::array<std::array<double,4>,4>t{};
  for(unsigned rep=0;rep<=4;++rep){
   std::array<int,4>order{0,1,2,3};
   std::mt19937_64 shuffler(seed+rep*7919);
   std::shuffle(order.begin(),order.end(),shuffler);
   for(int id:order){
    auto b=a;
    auto st=Clock::now();
    switch(id){
     case 0:rune_v14::sort(b);break;
     case 1:rune::sort(b);break;
     case 2:rune::detail::fixed_radix<8>(b.data(),b.size());break;
     case 3:rune::detail::radix11_full(b.data(),b.size());break;
    }
    auto en=Clock::now();
    if(b!=expected){std::cerr<<"BAD "<<names[id]<<" seed "<<seed<<"\n";return 5;}
    if(rep)t[id][rep-1]=std::chrono::duration<double,std::milli>(en-st).count();
   }
  }
  for(int id=0;id<4;++id){
   auto z=t[id];std::sort(z.begin(),z.end());
   std::cout<<n<<','<<seed<<','<<names[id]<<','
    <<std::fixed<<std::setprecision(6)<<(z[1]+z[2])*0.5<<','
    <<z.front()<<','<<z.back()<<",true\n";
  }
  std::cout.flush();
 }
}
