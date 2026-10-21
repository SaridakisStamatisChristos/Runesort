#include "runesort_v25/simd_msd.hpp"
#include "runesort_v24/rune_sort.hpp"
#include "experiments/v25-simd-msd/prototype_cache.hpp"
#include <random>
#include <chrono>
#include <iostream>
using U=std::uint64_t;
void call(unsigned k,std::vector<U>&a){switch(k){case 0:rune_v17::sort(a);break;case 1:rune_v24::sort(a);break;case 2:rune_v25::kernel<true>(a.data(),a.size());break;case 3:rune_v25::kernel<false>(a.data(),a.size());break;case 4:rune25_prototype::cache_radix<8>(a.data(),a.size());break;case 5:rune25_prototype::cache_radix<11>(a.data(),a.size());break;case 6:rune25_prototype::cache_radix<12>(a.data(),a.size());}}
int main(){
 std::mt19937_64 g(980003);
 for(size_t n=0;n<600;++n){std::vector<U>a(n);for(auto&x:a)x=g();auto ref=a;std::sort(ref.begin(),ref.end());for(unsigned k=2;k<7;++k){auto v=a;call(k,v);if(v!=ref){std::cerr<<"FAIL "<<n<<" "<<k;return 4;}}}
 std::cout<<"mode,n,algorithm,median_ms,trial0_ms,trial1_ms,trial2_ms,correct\n";
 for(size_t n:{1000000u,4000000u})for(unsigned mode=0;mode<2;++mode){
 std::vector<U>source(n);for(auto&x:source)x=g();if(mode)for(auto&x:source)x&=1;
 auto ref=source;std::sort(ref.begin(),ref.end());double t[7][3]{};
 std::vector<unsigned>ids{0,1,2,3,4,5,6};
 for(unsigned rep=0;rep<4;++rep){std::shuffle(ids.begin(),ids.end(),g);for(auto k:ids){auto v=source;auto s=std::chrono::steady_clock::now();call(k,v);auto e=std::chrono::steady_clock::now();if(v!=ref){std::cerr<<"FAIL "<<n<<" "<<k;return 4;}if(rep)t[k][rep-1]=std::chrono::duration<double,std::milli>(e-s).count();}}
 for(unsigned k=0;k<7;++k){std::vector<double>ts(t[k],t[k]+3);std::sort(ts.begin(),ts.end());std::cout<<(mode?"binary":"uniform64")<<','<<n<<','<<k<<','<<ts[1]<<','<<t[k][0]<<','<<t[k][1]<<','<<t[k][2]<<",true\n";}std::cout.flush();
 }
}
