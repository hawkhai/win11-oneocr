#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>

namespace oneocr_native {
class ImageError : public std::runtime_error { public: using std::runtime_error::runtime_error; };
struct Point { float x=0,y=0; };
using Quad=std::array<Point,4>;
struct Image {
    int width=0,height=0;
    std::vector<float> rgb;
    std::array<float,3> sample(float x,float y) const;
};
Image load_image(const char* utf8);
Image from_bgra(const uint8_t*,size_t,int,int,int64_t);
Image resize(const Image&,int,int);
Image warp_quad(const Image&,const Quad&,int,int);
Image rotate_quarter(const Image&,int);
Point map_quarter_back(Point,int,int,int);
Quad minimum_rectangle(std::vector<Point>);
float distance(Point,Point);
float quad_area(const Quad&);
std::vector<float> chw(const Image&,bool bgr,float scale=1);
}
