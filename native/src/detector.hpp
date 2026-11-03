#pragma once
#include "image.hpp"
#include <cstddef>
#include <vector>

namespace oneocr_native {
struct Segment { size_t cell;float score;Quad box; };
struct SegmentGroup { size_t root;std::vector<Segment> segments; };
std::vector<std::vector<size_t>> fragment_chains(const std::vector<Segment>& segments,bool horizontal);
struct LineCurve {
    bool horizontal;
    std::array<float,3> upper,lower; // Constant, linear, quadratic.
    Quad corners;
    float score;
};
// Recovered from the fixed 9.0.103 sample's XYSeglinkProposalsOpBase.
// This covers anchor decoding, undirected link components and fragment NMS.
// The later geometric splitting/joining of a component remains separate.
std::vector<SegmentGroup> detector_segments(const float* scores,const float* deltas,const float* links,int width,int height,int level,bool horizontal);
LineCurve fit_line_curve(const std::vector<Segment>& segments,bool horizontal,float scale_x,float scale_y);
std::array<float,3> fit_line_polynomial(const std::vector<double>& x,const std::vector<double>& y);
LineCurve refit_line_polygons(const std::vector<Point>& first,const std::vector<Point>& second,bool horizontal,float score);
std::vector<Point> curve_polygon(const LineCurve& curve);
// TextLine geometry uses a covariance eigenvector, then floor and AABB clamp.
// It is not the minimum-area rectangle of the polygon.
Quad line_polygon_box(const std::vector<Point>& polygon,bool curved=true);
float line_quad_angle(const Quad& quad);
}
