#include "detector.hpp"
#include "json.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace oneocr_native;
int main() {
    try {
        std::ifstream chains_input(ONEOCR_FRAGMENT_CONTRACT);nlohmann::json chains_contract;chains_input>>chains_contract;
        size_t chain_index=0;
        for(const auto& record:chains_contract.at("records")) {
            std::vector<Segment> segments;
            for(const auto& source:record.at("segments")){Segment s{source.at("cell"),source.at("score"),{}};for(int k=0;k<4;k++)s.box[k]={source.at("box")[2*k],source.at("box")[2*k+1]};segments.push_back(s);}
            if(fragment_chains(segments,record.at("horizontal"))!=record.at("chains").get<std::vector<std::vector<size_t>>>())throw std::runtime_error("Native fragment chain differs at "+std::to_string(chain_index));
            chain_index++;
        }
        std::ifstream input(ONEOCR_GEOMETRY_CONTRACT);nlohmann::json contract;input>>contract;
        for(const auto& record:contract.at("records")) {
            std::vector<Point> polygon;
            for(auto edge:{"upper","lower"}) {
                const auto& values=record.at(edge);
                for(size_t i=0;i<values.size();i+=2)polygon.push_back({values[i].get<float>(),values[i+1].get<float>()});
            }
            auto box=line_polygon_box(polygon);
            for(int i=0;i<4;i++)if(box[i].x!=record.at("box")[i*2].get<float>() || box[i].y!=record.at("box")[i*2+1].get<float>())
                throw std::runtime_error("Polygon PCA rectangle differs from native probe");
            if(std::abs(line_quad_angle(box)-record.at("angle").get<float>())>1e-4f)
                throw std::runtime_error("Native line angle differs");
        }
        std::cout<<"Native polygon geometry contracts: PASS\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
