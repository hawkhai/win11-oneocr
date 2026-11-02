#include "image.hpp"
#include "sha256.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace oneocr_native;
static void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
int main() {
    try {
        Sha256 h;const auto* abc=reinterpret_cast<const uint8_t*>("abc");h.update(abc,3);
        require(h.finish()=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad","SHA-256 known vector");
        require(detector_dimensions(1000,220,1152,768)==std::array<int,2>{1152,256},"native landscape detector dimensions");
        require(detector_dimensions(220,1000,1152,768)==std::array<int,2>{256,1152},"native portrait detector dimensions");
        require(detector_dimensions(320,180,1152,768)==std::array<int,2>{480,288},"native 1.5x enlargement limit");
        require(detector_dimensions(1123,781,1152,768)==std::array<int,2>{1120,768},"native integer aspect scaling");
        uint8_t pixels[]={0,0,255,255,0,255,0,255,1,2,3,4,255,0,0,255,255,255,255,255};
        auto image=from_bgra(pixels,sizeof(pixels),2,2,12);
        require(image.rgb[0]==255 && image.rgb[1]==0 && image.rgb[2]==0,"BGRA channels");
        require(image.rgb[6]==0 && image.rgb[8]==255,"padded stride");
        auto rotated=rotate_quarter(rotate_quarter(image,1),3);
        require(rotated.width==image.width && rotated.height==image.height && rotated.rgb==image.rgb,"rotation inverse");
        auto q=minimum_rectangle({{1,1},{5,1},{5,3},{1,3},{3,2}});
        require(std::abs(quad_area(q)-8)<1e-4,"minimum rectangle area");
        Quad skewed{{{0,100},{600,0},{606,36},{6,136}}},parallel{{{0,160},{600,60},{606,96},{6,196}}};
        require(quad_iou(skewed,parallel)==0,"disjoint skewed lines must not suppress each other");
        require(std::abs(quad_iou(skewed,skewed)-1)<1e-6,"quad self IoU");
        bool failed=false;try{from_bgra(pixels,4,2,2,12);}catch(const std::invalid_argument&){failed=true;}
        require(failed,"short buffer rejected");
        failed=false;try{from_bgra(pixels,sizeof(pixels),2,2,INT64_MAX);}catch(const std::invalid_argument&){failed=true;}
        require(failed,"overflowing stride rejected");
        require(from_bgra(pixels,4,1,1,INT64_MAX).rgb[0]==255,"single row does not read stride padding");
        std::cout<<"Hash, geometry, pixels and stride: PASS\n";return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
