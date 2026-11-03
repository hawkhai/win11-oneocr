#include "nms.hpp"
#include "unicode.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>
#include <string>
using namespace oneocr_native;
struct IntersectionCase {const char* name;float actual,expected;bool reject;};
static const IntersectionCase intersection_cases[]={
    {"at-positive-tolerance",.01f,0,false},
    {"at-negative-tolerance",-.01f,0,false},
    {"equal-large-finite",std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),false},
    {"over-positive-tolerance",std::nextafter(.01f,std::numeric_limits<float>::infinity()),0,true},
    {"over-negative-tolerance",-std::nextafter(.01f,std::numeric_limits<float>::infinity()),0,true},
    {"review-large-delta",0,1e9f,true},
    {"actual-nan",std::numeric_limits<float>::quiet_NaN(),0,true},
    {"actual-positive-infinity",std::numeric_limits<float>::infinity(),0,true},
    {"actual-negative-infinity",-std::numeric_limits<float>::infinity(),0,true},
    {"expected-nan",0,std::numeric_limits<float>::quiet_NaN(),true},
    {"expected-positive-infinity",0,std::numeric_limits<float>::infinity(),true},
    {"expected-negative-infinity",0,-std::numeric_limits<float>::infinity(),true},
    {"both-nan",std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN(),true},
    {"same-positive-infinity",std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),true},
    {"same-negative-infinity",-std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),true},
    {"finite-subtraction-overflow",std::numeric_limits<float>::max(),std::numeric_limits<float>::lowest(),true}
};
int main(int argc,char** argv) {
    const IntersectionCase* injected=nullptr;
    if(argc!=1) {
        if(argc!=3 || std::string(argv[1])!="--photos-intersection-case") {std::cerr<<"Invalid NMS test arguments\n";return 1;}
        for(const auto& sample:intersection_cases)if(sample.name==std::string(argv[2]))injected=&sample;
        if(!injected){std::cerr<<"Unknown Photos intersection case\n";return 1;}
    }
    bool injection_applied=false;
    nlohmann::json contract;
    size_t failures=0,intersection_failures=0,polygon_probes=0;float maximum=0;
    auto check_intersection=[&](float actual,float expected,const std::string& location) {
        // Preserve the existing .01f polygon-area contract. In particular,
        // abs(NaN)>tolerance is false, so finiteness must be checked explicitly.
        float error=std::abs(actual-expected);
        if(std::isfinite(error))maximum=std::max(maximum,error);
        if(!std::isfinite(actual) || !std::isfinite(expected) || !std::isfinite(error) || error>.01f) {
            std::cerr<<"Intersection mismatch "<<location<<": "<<actual<<" versus "<<expected<<"\n";
            failures++;intersection_failures++;
        }
    };
    for(const char* filename:{ONEOCR_NMS_CONTRACT,ONEOCR_OVERVIEW_NMS_CONTRACT}) {
    std::ifstream(filename)>>contract;
    std::vector<std::vector<Point>> polygons;std::vector<bool> horizontal;
    for(const auto& p:contract.at("polygons")){std::vector<Point> points;auto v=p.at("points").get<std::vector<float>>();for(size_t i=0;i<v.size();i+=2)points.push_back({v[i],v[i+1]});polygons.push_back(std::move(points));horizontal.push_back(p.at("horizontal"));}
    for(const auto& probe:contract.at("comparisons")) {
        polygon_probes++;
        size_t a=probe.at("first"),b=probe.at("second");
        float area=line_polygon_intersection(polygons[a],polygons[b]),expected=probe.at("intersection");
        check_intersection(area,expected,std::string(filename)+" "+std::to_string(a)+","+std::to_string(b));
        bool covered=line_polygon_covered(polygons[a],horizontal[a],polygons[b],horizontal[b]);
        if(probe.contains("primary_possible") && line_polygon_primary_possible(polygons[a],polygons[b],double(.2f))!=probe.at("primary_possible").get<bool>()){std::cerr<<"Primary gate mismatch "<<a<<","<<b<<"\n";failures++;}
        if(line_polygons_joinable(polygons[a],horizontal[a],polygons[b],horizontal[b])!=probe.at("joinable").get<bool>()){std::cerr<<"Join mismatch "<<a<<","<<b<<"\n";failures++;}
        if(covered!=probe.at("covered").get<bool>()){std::cerr<<"Coverage mismatch "<<a<<","<<b<<"\n";failures++;}
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
    nlohmann::json observed;std::ifstream(std::string(ONEOCR_OPTIONS_ORACLES)+"/index.json")>>observed;
    for(const auto& entry:observed.at("files")) {
        nlohmann::json oracle;std::ifstream(std::string(ONEOCR_OPTIONS_ORACLES)+"/"+entry.at("file").get<std::string>())>>oracle;
        for(const auto& call:oracle.at("nms")) {
            std::vector<std::vector<Point>> polygons;std::vector<bool> horizontal;
            for(const auto& region:call.at("after")) {
                auto values=region.at("upper").get<std::vector<float>>(),lower=region.at("lower").get<std::vector<float>>();values.insert(values.end(),lower.begin(),lower.end());
                std::vector<Point> points;for(size_t i=0;i<values.size();i+=2)points.push_back({values[i],values[i+1]});polygons.push_back(std::move(points));horizontal.push_back(region.at("horizontal"));
            }
            for(const auto& pair:call.at("pairs")) {
                size_t a=pair.at("first"),b=pair.at("second");polygon_probes++;
                if(line_polygon_primary_possible(polygons[a],polygons[b],double(.3f))!=pair.at("primary_possible").get<bool>()){std::cerr<<"Photos coarse gate differs\n";failures++;}
                if(line_polygon_covered(polygons[a],horizontal[a],polygons[b],horizontal[b])!=pair.at("covered").get<bool>()){std::cerr<<"Photos coverage differs\n";failures++;}
                float area=line_polygon_intersection(polygons[a],polygons[b]),expected=pair.at("intersection");
                // Inject only into a scalar assertion on the first real Photos
                // pair. Geometry and the frozen oracle files remain untouched.
                if(injected && !injection_applied){area=injected->actual;expected=injected->expected;injection_applied=true;}
                check_intersection(area,expected,"Photos "+entry.at("file").get<std::string>()+" "+std::to_string(a)+","+std::to_string(b));
            }
        }
        for(const auto& call:oracle.at("choices")) {
            std::vector<RecognitionCandidate> candidates;std::vector<size_t> source;
            for(const auto& line:call.at("lines")) {
                RecognitionCandidate value;auto p=line.at("polygon").get<std::vector<float>>();for(size_t i=0;i<p.size();i+=2)value.polygon.push_back({p[i],p[i+1]});
                value.text=unicode_characters(line.at("text"));value.horizontal=line.at("horizontal");value.min_rectangle=line.at("ambiguous");
                value.script=line.at("script");value.style=line.at("style");value.confidence_score=line.at("confidence_score");value.language_score=line.at("language_score");
                candidates.push_back(std::move(value));source.push_back(line.at("source_index"));
            }
            auto suppressed=recognition_suppressed(candidates);std::vector<size_t> kept;
            for(size_t i=0;i<suppressed.size();i++)if(!suppressed[i])kept.push_back(source[i]);
            if(kept!=call.at("kept_source_indices").get<std::vector<size_t>>()){std::cerr<<"Actual Photos recognition choice differs\n";failures++;}
            case_index++;
        }
    }
    std::cout<<polygon_probes<<" native polygon probes and "<<case_index<<" recognition NMS cases; maximum area error "<<maximum<<"\n";
    if(injected) {
        size_t expected_failures=injected->reject?1:0;
        if(!injection_applied || failures!=expected_failures || intersection_failures!=expected_failures) {
            std::cerr<<"Photos intersection assertion did not enforce the injected contract\n";return 1;
        }
        std::cout<<"Photos intersection case "<<injected->name<<": PASS ("<<expected_failures<<" expected rejection)\n";return 0;
    }
    return failures?1:0;
}
