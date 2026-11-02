// Research-only child process: decode with the same stb implementation as CLI.
#include "image.hpp"
#include <fstream>
#include <iostream>
#include <cstdint>
int main(int argc,char** argv) {
    if(argc!=3)return 2;
    try {
        auto image=oneocr_native::load_image(argv[1]);std::ofstream output(argv[2],std::ios::binary);
        int32_t header[]={image.width,image.height};output.write(reinterpret_cast<const char*>(header),sizeof(header));
        for(size_t i=0;i<size_t(image.width)*image.height;i++){uint8_t pixel[]={uint8_t(image.rgb[i*3+2]),uint8_t(image.rgb[i*3+1]),uint8_t(image.rgb[i*3]),255};output.write(reinterpret_cast<const char*>(pixel),4);}
        return output?0:3;
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
