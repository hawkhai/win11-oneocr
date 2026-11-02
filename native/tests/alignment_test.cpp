#include "alignment.hpp"
#include "normalizer.hpp"
#include "json.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace oneocr_native;
int main() {
    try {
        std::ifstream input(ONEOCR_ALIGNMENT_CONTRACT);nlohmann::json contract;input>>contract;
        for(const auto& record:contract.at("records")) {
            std::vector<WordFrames> frames;
            for(const auto& word:record.at("words"))frames.push_back({word.at("start"),word.at("end")});
            auto blanks=record.at("blank_intervals").get<std::vector<std::array<int,2>>>();
            align_word_frames(frames,record.at("stride"),record.at("pixels_per_space"),record.at("start_shift"),record.at("end_shift"),record.at("steps"),blanks);
            std::vector<Point> polygon;const auto& values=record.at("polygon");
            for(size_t i=0;i<values.size();i+=2)polygon.push_back({values[i].get<float>(),values[i+1].get<float>()});
            for(size_t i=0;i<frames.size();i++) {
                const auto& expected=record.at("words")[i];
                if(frames[i].start!=expected.at("pixel_start") || frames[i].end!=expected.at("pixel_end"))throw std::runtime_error("Native word pixel interval differs");
                auto box=map_line_word(polygon,frames[i].start,frames[i].end,false);
                for(int j=0;j<4;j++)if(std::abs(box[j].x-expected.at("bounding_box")[j*2].get<float>())>1e-3f || std::abs(box[j].y-expected.at("bounding_box")[j*2+1].get<float>())>1e-3f)
                    throw std::runtime_error("Native inverse triangle word mapping differs");
            }
        }
        std::cout<<"Native word alignment contracts: PASS\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
