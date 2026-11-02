#include "classification.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace oneocr_native;
int main() {
    try {
        std::ifstream input(ONEOCR_CLASSIFICATION_CONTRACT);nlohmann::json contract;input>>contract;
        ClassificationSettings settings;
        for(size_t i=0;i<contract.at("cases").size();i++) {
            const auto& sample=contract.at("cases")[i];auto scores=sample.at("scores").get<std::array<float,10>>();
            auto line=classify_line(scores.data(),sample.at("script_frames").get<std::vector<float>>(),sample.at("handwriting"),sample.at("handwriting_frames").get<std::vector<float>>(),10,true,settings);
            if(line.script!=sample.at("script") || line.style!=sample.at("style"))throw std::runtime_error("Native classifier decision differs at "+std::to_string(i)+": script="+std::to_string(line.script)+", style="+std::to_string(line.style));
        }
        for(const auto& sample:contract.at("global_cases"))if(resolve_script(sample.at("probabilities").get<std::array<float,10>>(),sample.at("counts").get<std::array<size_t,12>>())!=sample.at("script"))throw std::runtime_error("Native global script vote differs");
        std::cout<<"201 native classifier and 80 global script decisions: PASS\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
