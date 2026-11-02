#include "ordering.hpp"
#include "json.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace oneocr_native;
static Quad quad(const nlohmann::json& value) {
    Quad result{};for(int i=0;i<4;i++)result[i]={value.at(i*2).get<float>(),value.at(i*2+1).get<float>()};return result;
}
int main() {
    try {
        std::ifstream group_input(ONEOCR_GROUP_ORDER_CONTRACT);nlohmann::json group_contract;group_input>>group_contract;
        for(const auto& record:group_contract.at("cases")){auto first=native_group_order(record.at("keys").get<std::vector<int32_t>>());if(first!=record.at("first").get<std::vector<int32_t>>() || native_group_order(first)!=record.at("second").get<std::vector<int32_t>>())throw std::runtime_error("Native group hash iteration differs");}
        std::ifstream input(ONEOCR_ORDER_CONTRACT);nlohmann::json contract;input>>contract;
        for(const auto& record:contract.at("comparisons"))
            if(reading_before(quad(record.at("first")),quad(record.at("second")),record.at("rtl").get<bool>())!=record.at("before").get<bool>())
                throw std::runtime_error("Native reading-order comparison differs");
        for(const auto& record:contract.at("rotations")) {
            auto actual=reading_coordinates(quad(record.at("source")),record.at("angle").get<float>()),expected=quad(record.at("target"));
            for(int i=0;i<4;i++)if(std::abs(actual[i].x-expected[i].x)>1e-4f || std::abs(actual[i].y-expected[i].y)>1e-4f)
                throw std::runtime_error("Native reading-coordinate rotation differs");
        }
        if(dominant_angle({359,1,90,180})!=1)throw std::runtime_error("Cardinal bin upper median differs");
        nlohmann::json sorts;std::ifstream(ONEOCR_SORT_CONTRACT)>>sorts;
        for(const auto& record:sorts.at("cases")) {
            std::vector<Quad> boxes;for(const auto& value:record.at("boxes"))boxes.push_back(quad(value));
            auto actual=reading_order(boxes,record.at("angle"),record.at("rtl"));
            if(actual!=record.at("order").get<std::vector<size_t>>())throw std::runtime_error("Native complete reading sort differs for "+std::to_string(boxes.size())+" lines");
        }
        nlohmann::json detector;std::ifstream(ONEOCR_DETECTOR_ORDER_CONTRACT)>>detector;
        auto scores=detector.at("scores").get<std::vector<float>>();
        if(native_sort_order(scores.size(),[&](size_t a,size_t b){return scores[a]>scores[b];})!=detector.at("order").get<std::vector<size_t>>())throw std::runtime_error("Native detector equal-score ordering differs");
        std::cout<<"Native reading-order contracts: PASS\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
