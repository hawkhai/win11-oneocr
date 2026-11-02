#include <wasm_simd128.h>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/intrin.hpp>
#include <array>
#include <climits>
#include <iostream>
#include <limits>

int main() {
    const float nan=std::numeric_limits<float>::quiet_NaN(),infinity=std::numeric_limits<float>::infinity();
    const std::array<float,24> input{-2.5f,-1.5f,-.5f,.5f,1.5f,2.5f,3.5f,4.5f,
        126.5f,127.5f,128.5f,129.5f,2147483648.f,-2147483648.f,nan,infinity,
        -infinity,.49999997f,.50000006f,-.50000006f,254.5f,255.5f,-126.5f,-127.5f};
    const std::array<int,24> expected{-2,-2,0,0,2,2,4,4,126,128,128,130,
        INT_MIN,INT_MIN,INT_MIN,INT_MIN,INT_MIN,0,1,-1,254,256,-126,-128};
    for(size_t i=0;i<input.size();i+=4) {
        std::array<int,4> output;
        cv::v_store(output.data(),cv::v_round(cv::v_load(input.data()+i)));
        for(size_t j=0;j<4;j++)if(output[j]!=expected[i+j]) {
            std::cerr<<"SIMD nearest-even mismatch at "<<i+j<<": "<<output[j]<<" expected "<<expected[i+j]<<'\n';return 1;
        }
    }
    std::array<float,32> pixels;std::array<unsigned char,32> quantized;
    for(int i=0;i<32;i++)pixels[i]=float(i)+.5f;
    for(int length:{1,3,4,7,8,15,16,17,31,32}) {
        cv::Mat source(1,length,CV_32F,pixels.data()),target(1,length,CV_8U,quantized.data());source.convertTo(target,CV_8U);
        for(int i=0;i<length;i++)if(quantized[i]!=i+(i&1)){std::cerr<<"Pixel rounding mismatch at length "<<length<<", index "<<i<<'\n';return 1;}
    }
    std::cout<<"Wasm SIMD nearest-even, overflow sentinel and pixel SIMD/tail boundaries: PASS\n";return 0;
}
