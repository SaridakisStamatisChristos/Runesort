#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=uint64_t;
static U mask(unsigned bits,int pattern,std::mt19937_64&g){
 U m=0;
 if(pattern==0)for(unsigned i=0;i<bits;++i)m|=U(1)<<((i*29+7)&63);
 if(pattern==1)m=((U(1)<<bits)-1)<<22;
 if(pattern==2)m=((U(1)<<bits)-1)<<(64-bits);
 if(pattern==3)for(unsigned i=0;i<bits;++i)m|=U(1)<<((i*19+11)&63);
 if(pattern==4)while(rune::detail::popcount(m)<bits)m|=U(1)<<(g()%64);
 if(pattern==5){
  for(unsigned i=0;i<bits/2;++i)m|=U(1)<<i;
  for(unsigned i=bits/2;i<bits;++i)m|=U(1)<<(48+i-bits/2);
 }
 return m;
}
static size_t checked=0;
static void validate(std::vector<U> input){
 auto expected=input;
 std::sort(expected.begin(),expected.end());
 rune::sort(input);
 if(input!=expected){
  std::fprintf(stderr,"RUNE v14 spectrum incorrect n=%zu\n",input.size());
  std::exit(4);
 }
 ++checked;
}
int main(){
 std::mt19937_64 g(0x14A7C001ULL);
 for(size_t n:{size_t(0),size_t(1),size_t(63),size_t(64),
  size_t(4096),size_t(65536),size_t(262143),size_t(262144),
  size_t(524287),size_t(524288),size_t(524289),size_t(1000000)}){
  for(unsigned bits:{15u,16u,17u,20u,23u,24u}){
   for(int p=0;p<6;++p){
    U m=mask(bits,p,g),fixed=g()&~m;
    std::vector<U> a(n);
    for(auto&x:a)x=fixed|rune::detail::deposit(g(),m);
    validate(a);
    if(n<525000){
      std::sort(a.begin(),a.end());
      validate(a);
    }
   }
  }
 }
 // Exact fifth-tier helper must not mutate input on bounded rejection:
 // 100K different full-width values necessarily exceeds 81,920 keys.
 std::vector<U> many(100000);
 for(size_t i=0;i<many.size();++i)many[i]=i*0x9e3779b97f4a7c15ULL;
 auto before=many;
 if(rune::detail::fifth_distinct_reconstruct(many.data(),many.size())||many!=before){
  std::fputs("fifth tier did not reject without mutation\n",stderr);
  return 5;
 }
 std::printf("PASSED %zu v14 full-array spectrum cases plus exact fail-closed probe\n",checked);
}
