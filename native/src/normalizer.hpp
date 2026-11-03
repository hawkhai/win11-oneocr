#pragma once
#include "image.hpp"
#include <vector>
namespace oneocr_native {
Image resize_detector_pixels(const Image& image,int width,int height);
struct QuadNormalization {Image image;std::array<float,6> inverse;std::array<float,4> bounds;};
struct PolygonMapping {
    int width=0;
    std::vector<std::array<Point,3>> triangles;
    std::vector<std::array<float,10>> inverse;
};
QuadNormalization normalize_quad(const Image& image,const std::vector<Point>& polygon,int maximum_width);
Quad map_quad_word(const QuadNormalization& normalized,int start,int end,bool flipped);
// Piecewise affine normalization from the detector's paired curve edges.
Image normalize_polygon(const Image& image,const std::vector<Point>& polygon,int maximum_width,PolygonMapping* clipped_mapping=nullptr);
Quad map_clipped_polygon_word(const PolygonMapping&,const std::vector<Point>& polygon,int start,int end,bool flipped);
Image resize_line_pixels(const Image& image,int width,int height);
std::vector<std::array<int,2>> line_blank_intervals(const Image& image);
Quad map_line_word(const std::vector<Point>& polygon,int start,int end,bool flipped);
Quad orient_line_box(const Quad& box,const Quad& crop);
}
