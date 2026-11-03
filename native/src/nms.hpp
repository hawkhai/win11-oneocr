#pragma once
#include "image.hpp"
#include <string>
namespace oneocr_native {
float line_polygon_intersection(const std::vector<Point>& first,const std::vector<Point>& second);
float line_polygon_area(const std::vector<Point>& polygon);
bool line_polygon_primary_possible(const std::vector<Point>& first,const std::vector<Point>& second,double threshold);
bool line_polygon_covered(const std::vector<Point>& first,bool first_horizontal,const std::vector<Point>& second,bool second_horizontal,float threshold=.4f);
bool line_polygons_joinable(const std::vector<Point>& first,bool first_horizontal,const std::vector<Point>& second,bool second_horizontal,double width_divisor=4,double thickness_ratio=2);
struct RecognitionCandidate {
    std::vector<Point> polygon;
    std::u32string text; // Concatenated recognized words, before visual-to-logical conversion.
    bool horizontal=true,min_rectangle=false;
    int script=0,style=0;
    float language_score=0,confidence_score=0;
};
std::vector<bool> recognition_suppressed(const std::vector<RecognitionCandidate>& lines);
}
