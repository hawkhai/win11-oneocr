#include "nms.hpp"
#include "unicode.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <cmath>
using namespace oneocr_native;
int main() {
    nlohmann::json contract;
    size_t failures=0,polygon_probes=0;float maximum=0;
    for(const char* filename:{ONEOCR_NMS_CONTRACT,ONEOCR_OVERVIEW_NMS_CONTRACT}) {
    std::ifstream(filename)>>contract;
    std::vector<std::vector<Point>> polygons;std::vector<bool> horizontal;
    for(const auto& p:contract.at("polygons")){std::vector<Point> points;auto v=p.at("points").get<std::vector<float>>();for(size_t i=0;i<v.size();i+=2)points.push_back({v[i],v[i+1]});polygons.push_back(std::move(points));horizontal.push_back(p.at("horizontal"));}
    for(const auto& probe:contract.at("comparisons")) {
        polygon_probes++;
        size_t a=probe.at("first"),b=probe.at("second");
        float area=line_polygon_intersection(polygons[a],polygons[b]),expected=probe.at("intersection");maximum=std::max(maximum,std::abs(area-expected));
        bool covered=line_polygon_covered(polygons[a],horizontal[a],polygons[b],horizontal[b]);
        if(line_polygons_joinable(polygons[a],horizontal[a],polygons[b],horizontal[b])!=probe.at("joinable").get<bool>()){std::cerr<<"Join mismatch "<<a<<","<<b<<"\n";failures++;}
        if(covered!=probe.at("covered").get<bool>()){std::cerr<<"Coverage mismatch "<<a<<","<<b<<"\n";failures++;}
        if(std::abs(area-expected)>.01f){std::cerr<<"Intersection mismatch "<<a<<","<<b<<": "<<area<<" versus "<<expected<<"\n";failures++;}
    }
    }
    std::ifstream(ONEOCR_RECOGNITION_NMS_CONTRACT)>>contract;
    size_t case_index=0;
    for(const auto& sample:contract.at("cases")) {
        std::vector<RecognitionCandidate> candidates;
        for(const auto& line:sample.at("lines")) {
            RecognitionCandidate value;auto points=line.at("polygon").get<std::vector<float>>();
            for(size_t i=0;i<points.size();i+=2)value.polygon.push_back({points[i],points[i+1]});
            value.text=unicode_characters(line.at("text"));value.horizontal=line.at("horizontal");value.min_rectangle=line.at("min_rectangle");
            value.script=line.at("script");value.style=line.at("style");value.language_score=line.at("language_score");value.confidence_score=line.at("confidence_score");candidates.push_back(std::move(value));
        }
        auto suppressed=recognition_suppressed(candidates);std::vector<size_t> kept;
        for(size_t i=0;i<suppressed.size();i++)if(!suppressed[i])kept.push_back(i);
        if(kept!=sample.at("kept").get<std::vector<size_t>>()){std::cerr<<"Recognition NMS mismatch at "<<case_index<<"\n";failures++;}
        case_index++;
    }
    std::cout<<polygon_probes<<" native polygon probes and "<<case_index<<" recognition NMS cases; maximum area error "<<maximum<<"\n";return failures?1:0;
}
