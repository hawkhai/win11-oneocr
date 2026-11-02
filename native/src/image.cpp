#include "image.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MAX_DIMENSIONS 32768
#include "stb_image.h"

namespace oneocr_native {
static void dimensions(int w,int h) {
    if(w<=0 || h<=0 || w>32768 || h>32768 || uint64_t(w)*uint64_t(h)>100000000)
        throw std::invalid_argument("Image dimensions exceed supported limits");
}
Image load_image(const char* path) {
    std::ifstream f(std::filesystem::u8path(path),std::ios::binary|std::ios::ate);
    if(!f) throw ImageError("Cannot open image");
    auto length=f.tellg();
    if(length<=0 || length>std::numeric_limits<int>::max()) throw ImageError("Invalid image file size");
    std::vector<uint8_t> bytes(static_cast<size_t>(length));
    f.seekg(0); f.read(reinterpret_cast<char*>(bytes.data()),length);
    if(!f) throw ImageError("Cannot read image");
    int w=0,h=0,c=0;
    if(!stbi_info_from_memory(bytes.data(),static_cast<int>(bytes.size()),&w,&h,&c)) throw ImageError("Unsupported or corrupt image");
    dimensions(w,h);
    // The original wrapper accepts gray, RGB and RGBA, but rejects gray+alpha.
    if(c!=1 && c!=3 && c!=4) throw UnsupportedImageError("Unsupported image channel count");
    unsigned char* data=stbi_load_from_memory(bytes.data(),static_cast<int>(bytes.size()),&w,&h,&c,4);
    if(!data) throw ImageError("Image decoder failed");
    struct Release { unsigned char* p; ~Release(){ stbi_image_free(p); } } guard{data};
    Image out{w,h,std::vector<float>(size_t(w)*h*3)};
    for(size_t i=0;i<size_t(w)*h;i++) for(int ch=0;ch<3;ch++) {
        out.rgb[i*3+ch]=data[i*4+ch]; // Match legacy BGRA: alpha is ignored.
    }
    return out;
}
Image from_bgra(const uint8_t* data,size_t bytes,int w,int h,int64_t stride) {
    dimensions(w,h);
    if(!data || stride<int64_t(w)*4 || uint64_t(w)*4>bytes) throw std::invalid_argument("Invalid BGRA pointer, byte count or stride");
    if(uint64_t(h-1) && uint64_t(stride)>(uint64_t(bytes)-uint64_t(w)*4)/uint64_t(h-1)) throw std::invalid_argument("BGRA buffer is too small");
    uint64_t required=uint64_t(h-1)*uint64_t(stride)+uint64_t(w)*4;
    if(required>bytes || required>size_t(-1)) throw std::invalid_argument("BGRA buffer is too small");
    Image out{w,h,std::vector<float>(size_t(w)*h*3)};
    for(int y=0;y<h;y++) for(int x=0;x<w;x++) {
        const uint8_t* p=data+size_t(y)*size_t(stride)+size_t(x)*4;
        for(int c=0;c<3;c++) out.rgb[(size_t(y)*w+x)*3+c]=p[2-c];
    }
    return out;
}
std::array<float,3> Image::sample(float x,float y) const {
    x=std::clamp(x,0.f,float(width-1)); y=std::clamp(y,0.f,float(height-1));
    int x0=int(x),y0=int(y),x1=std::min(x0+1,width-1),y1=std::min(y0+1,height-1);
    float dx=x-x0,dy=y-y0;
    std::array<float,3> out{};
    for(int c=0;c<3;c++) {
        float a=rgb[(size_t(y0)*width+x0)*3+c],b=rgb[(size_t(y0)*width+x1)*3+c];
        float d=rgb[(size_t(y1)*width+x0)*3+c],e=rgb[(size_t(y1)*width+x1)*3+c];
        out[c]=(a+(b-a)*dx)*(1-dy)+(d+(e-d)*dx)*dy;
    }
    return out;
}
Image resize(const Image& in,int w,int h) {
    dimensions(w,h);
    Image out{w,h,std::vector<float>(size_t(w)*h*3)};
    for(int y=0;y<h;y++) for(int x=0;x<w;x++) {
        auto value=in.sample((x+.5f)*in.width/w-.5f,(y+.5f)*in.height/h-.5f);
        for(int c=0;c<3;c++) out.rgb[(size_t(y)*w+x)*3+c]=value[c];
    }
    return out;
}
std::array<int,2> detector_dimensions(int width,int height,int limit_width,int limit_height) {
    dimensions(width,height);
    if(limit_width<1 || limit_height<1)throw std::invalid_argument("Invalid detector limits");
    // OneOCR 9.0.103: 180113400 -> 1801b2810 -> 1801b2250.
    // Limits apply to the short/long edge; enlargement is capped at 1.5x.
    // Preserve integer truncation at each resize step, then round UP to stride 32.
    int short_edge=std::min(width,height),long_edge=std::max(width,height);
    int short_limit=std::min(std::min(limit_width,limit_height),int(short_edge*1.5f+.5f));
    int long_limit=std::min(std::max(limit_width,limit_height),int(long_edge*1.5f+.5f));
    if(short_edge<short_limit) {long_edge=int(int64_t(short_limit)*long_edge/short_edge);short_edge=short_limit;}
    if(long_limit<long_edge) {short_edge=int(int64_t(long_limit)*short_edge/long_edge);long_edge=long_limit;}
    if(short_limit<short_edge) {long_edge=int(int64_t(short_limit)*long_edge/short_edge);short_edge=short_limit;}
    short_edge=std::max(1,(short_edge+31)/32)*32;
    long_edge=std::max(1,(long_edge+31)/32)*32;
    return width>=height?std::array<int,2>{long_edge,short_edge}:std::array<int,2>{short_edge,long_edge};
}
Image warp_quad(const Image& in,const Quad& q,int w,int h) {
    dimensions(w,h);
    Image out{w,h,std::vector<float>(size_t(w)*h*3)};
    // Projective mapping from the unit square to the detected quadrilateral.
    double dx1=q[1].x-q[2].x,dx2=q[3].x-q[2].x;
    double dy1=q[1].y-q[2].y,dy2=q[3].y-q[2].y;
    double dx3=q[0].x-q[1].x+q[2].x-q[3].x;
    double dy3=q[0].y-q[1].y+q[2].y-q[3].y;
    double det=dx1*dy2-dx2*dy1,g=0,hp=0;
    if(std::abs(det)>1e-12) { g=(dx3*dy2-dx2*dy3)/det; hp=(dx1*dy3-dx3*dy1)/det; }
    double a=q[1].x-q[0].x+g*q[1].x,b=q[3].x-q[0].x+hp*q[3].x;
    double d=q[1].y-q[0].y+g*q[1].y,e=q[3].y-q[0].y+hp*q[3].y;
    for(int y=0;y<h;y++) for(int x=0;x<w;x++) {
        double u=w>1?double(x)/(w-1):0,v=h>1?double(y)/(h-1):0,den=g*u+hp*v+1;
        auto value=in.sample(float((a*u+b*v+q[0].x)/den),float((d*u+e*v+q[0].y)/den));
        for(int c=0;c<3;c++) out.rgb[(size_t(y)*w+x)*3+c]=value[c];
    }
    return out;
}
Image rotate_quarter(const Image& in,int turns) {
    turns=((turns%4)+4)%4;
    if(!turns) return in;
    int w=turns%2?in.height:in.width,h=turns%2?in.width:in.height;
    Image out{w,h,std::vector<float>(size_t(w)*h*3)};
    for(int y=0;y<h;y++) for(int x=0;x<w;x++) {
        Point p=map_quarter_back({float(x),float(y)},turns,in.width,in.height);
        for(int c=0;c<3;c++) out.rgb[(size_t(y)*w+x)*3+c]=in.rgb[(size_t(p.y)*in.width+size_t(p.x))*3+c];
    }
    return out;
}
Point map_quarter_back(Point p,int t,int w,int h) {
    switch(t%4) {
    case 1:return {p.y,float(h-1)-p.x};
    case 2:return {float(w-1)-p.x,float(h-1)-p.y};
    case 3:return {float(w-1)-p.y,p.x};
    default:return p;
    }
}
float distance(Point a,Point b) { return std::hypot(a.x-b.x,a.y-b.y); }
float quad_area(const Quad& q) {
    float area=0;
    for(int i=0;i<4;i++) area+=q[i].x*q[(i+1)%4].y-q[i].y*q[(i+1)%4].x;
    return std::abs(area)*.5f;
}
static float cross(Point o,Point a,Point b) { return (a.x-o.x)*(b.y-o.y)-(a.y-o.y)*(b.x-o.x); }
float quad_iou(const Quad& first,const Quad& second) {
    float winding=0;
    for(int i=0;i<4;i++)winding+=second[i].x*second[(i+1)%4].y-second[(i+1)%4].x*second[i].y;
    float sign=winding>=0?1.f:-1.f;
    std::vector<Point> polygon(first.begin(),first.end());
    for(int edge=0;edge<4 && !polygon.empty();edge++) {
        Point a=second[edge],b=second[(edge+1)%4];
        std::vector<Point> clipped;
        Point previous=polygon.back();float previous_side=sign*cross(a,b,previous);
        for(auto point:polygon) {
            float side=sign*cross(a,b,point);
            if((side>=0)!=(previous_side>=0)) {
                float ratio=previous_side/(previous_side-side);
                clipped.push_back({previous.x+ratio*(point.x-previous.x),previous.y+ratio*(point.y-previous.y)});
            }
            if(side>=0)clipped.push_back(point);
            previous=point;previous_side=side;
        }
        polygon=std::move(clipped);
    }
    float twice_area=0;
    for(size_t i=0;i<polygon.size();i++) {
        Point a=polygon[i],b=polygon[(i+1)%polygon.size()];twice_area+=a.x*b.y-b.x*a.y;
    }
    float intersection=std::abs(twice_area)*.5f,area=quad_area(first)+quad_area(second)-intersection;
    return area>0?intersection/area:0;
}
Quad minimum_rectangle(std::vector<Point> p) {
    if(p.empty()) throw std::invalid_argument("Empty polygon");
    std::sort(p.begin(),p.end(),[](Point a,Point b){return a.x<b.x || (a.x==b.x && a.y<b.y);});
    std::vector<Point> hull;
    for(auto v:p) { while(hull.size()>1 && cross(hull[hull.size()-2],hull.back(),v)<=0) hull.pop_back(); hull.push_back(v); }
    size_t lower=hull.size();
    for(auto it=p.rbegin()+1;it!=p.rend();++it) { while(hull.size()>lower && cross(hull[hull.size()-2],hull.back(),*it)<=0) hull.pop_back(); hull.push_back(*it); }
    if(hull.size()>1) hull.pop_back();
    double best=std::numeric_limits<double>::infinity(); Quad result{};
    for(size_t i=0;i<hull.size();i++) {
        Point a=hull[i],b=hull[(i+1)%hull.size()];
        double angle=std::atan2(b.y-a.y,b.x-a.x),cs=std::cos(angle),sn=std::sin(angle);
        double minx=1e30,miny=1e30,maxx=-1e30,maxy=-1e30;
        for(auto v:hull) { double x=cs*v.x+sn*v.y,y=-sn*v.x+cs*v.y; minx=std::min(minx,x);maxx=std::max(maxx,x);miny=std::min(miny,y);maxy=std::max(maxy,y); }
        double area=(maxx-minx)*(maxy-miny);
        if(area<best) {
            best=area;
            std::array<std::array<double,2>,4> corners{{{minx,miny},{maxx,miny},{maxx,maxy},{minx,maxy}}};
            for(int j=0;j<4;j++) result[j]={float(cs*corners[j][0]-sn*corners[j][1]),float(sn*corners[j][0]+cs*corners[j][1])};
        }
    }
    // Begin at the top-left, keeping clockwise winding in image coordinates.
    int start=0;
    for(int i=1;i<4;i++) if(result[i].x+result[i].y<result[start].x+result[start].y) start=i;
    Quad normalized{};
    for(int i=0;i<4;i++) normalized[i]=result[(start+i)%4];
    return normalized;
}
std::vector<float> chw(const Image& image,bool bgr,float scale) {
    size_t n=size_t(image.width)*image.height; std::vector<float> out(n*3);
    for(int c=0;c<3;c++) for(size_t i=0;i<n;i++) out[size_t(c)*n+i]=image.rgb[i*3+(bgr?2-c:c)]*scale;
    return out;
}
}
