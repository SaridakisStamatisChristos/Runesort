#include "rune_sort.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <numeric>
using U=uint64_t;
static size_t checks=0;
void verify(std::vector<U> a){auto expected=a;std::sort(expected.begin(),expected.end());rune::sort(a);if(a!=expected){std::cerr<<"Mismatch in adversarial check "<<checks<<"\n";std::exit(3);}++checks;}
int main(){
 std::mt19937_64 g(0xCAFEF00D);
 for(size_t n:{64u,65u,127u,128u,511u,512u,8192u,16384u,65536u}){
  for(size_t q=0;q<400;++q){
   std::vector<U>a(n);
   switch(q%10){
   case 0:for(size_t i=0;i<n;++i)a[i]=i;break;
   case 1:for(size_t i=0;i<n;++i)a[i]=n-i;break;
   case 2:for(size_t i=0;i<n;++i)a[i]=17;break;
   case 3:for(size_t i=0;i<n;++i)a[i]=U(i%2)?~U(0):0;break;
   default:for(size_t i=0;i<n;++i)a[i]=g();break;
   }
   if(q%10==0 || q%10==1 || q%10==2)for(size_t i=64;i<n;++i)a[i]=g();
   if(q%10==3)for(size_t i=64;i<n;++i)a[i]=g();
   if(q%10==4)for(size_t i=0;i<64;++i)a[i]=i;
   if(q%10==5)for(size_t i=0;i<64;++i)a[i]=~U(0)-i;
   if(q%10==6)for(size_t i=0;i<64;++i)a[i]=g()%2;
   if(q%10==7)for(size_t i=0;i<n;++i)a[i]=g()%17;
   if(q%10==8){std::sort(a.begin(),a.end());for(size_t i=0;i<n/100;++i)std::swap(a[g()%n],a[g()%n]);}
   if(q%10==9){std::sort(a.begin(),a.end());std::rotate(a.begin(),a.begin()+(g()%n),a.end());}
   if(q%13==0)a.back()=0;
   if(q%17==0)a.back()=~U(0);
   verify(std::move(a));
  }
 }
 for(int len=0;len<=8;++len){
  uint64_t options=U(1)<<(2*len);
  for(uint64_t code=0;code<options;++code){
   std::vector<U> a(64,0);
   for(int j=0;j<len;++j)a[64-len+j]=(code>>(2*j))&3;
   verify(std::move(a));
  }
 }
 for(int j=0;j<2000;++j){
  size_t n=16384;std::vector<U>a(n);
  U offset=g();
  for(size_t i=0;i<n;++i)a[i]=(U(i/16)<<32)|U(i%16);
  if(j%3==0)for(size_t z=0;z<n/100;++z)std::swap(a[g()%n],a[g()%n]);
  if(j%3==1)std::rotate(a.begin(),a.begin()+(g()%n),a.end());
  if(j%3==2)for(U&x:a)x^=offset;
  verify(std::move(a));
 }
 std::cout<<"PASSED "<<checks<<" adversarial cases.\n";
}
