#include "oneocr-fma.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>

static uint32_t state=0x31c5a73d;
static uint32_t random_bits(){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
static float value(uint32_t bits){float result;std::memcpy(&result,&bits,4);return result;}
static uint32_t bits(float number){uint32_t result;std::memcpy(&result,&number,4);return result;}
int main(){
    const uint32_t boundaries[]={0,0x80000000,1,0x80000001,0x7fffff,0x800000,0x80800000,
        0x3f000000,0x3f800000,0x3f800001,0x3f800002,0xbf800000,0x7f7fffff,0x7f800000,0xff800000,0x7fc00000};
    auto check=[](float weight,const float* a,const float* c){
        float result[4];wasm_v128_store(result,OneOcrFma4(wasm_v128_load(a),weight,wasm_v128_load(c)));
        for(int i=0;i<4;i++){
            float expected=std::fma(a[i],weight,c[i]);
            if(!(std::isnan(result[i]) && std::isnan(expected)) && bits(result[i])!=bits(expected)){
                std::cerr<<"FMA mismatch: "<<std::hex<<bits(a[i])<<' '<<bits(weight)<<' '<<bits(c[i])<<" actual "<<bits(result[i])<<" expected "<<bits(expected)<<'\n';return false;
            }
        }
        return true;
    };
    for(uint32_t w:boundaries)for(uint32_t x:boundaries)for(size_t i=0;i<16;i+=4){
        float a[4],c[4];for(int j=0;j<4;j++){a[j]=value(x);c[j]=value(boundaries[i+j]);}
        if(!check(value(w),a,c))return 1;
    }
    for(int n=0;n<250000;n++){
        float a[4],c[4];float w=value(random_bits());
        for(int i=0;i<4;i++){a[i]=value(random_bits());c[i]=value(random_bits());}
        if(!check(w,a,c))return 1;
    }
    std::cout<<"Wasm vector FMA vs scalar IEEE fmaf: boundary matrix and 1000000 random lanes PASS\n";return 0;
}
