#include "detector.hpp"
#include "normalizer.hpp"
#include "word_box_assertions.hpp"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits>
using namespace oneocr_native;
int main() {
    try {
        std::filesystem::path directory=ONEOCR_BOUNDARY_CONTRACT;
        std::ifstream input(directory);nlohmann::json contract;input>>contract;
        for(const auto& record:contract.at("polynomials")) {
            auto actual=fit_line_polynomial(record.at("x").get<std::vector<double>>(),record.at("y").get<std::vector<double>>());
            const auto& expected=record.at(record.at("lu_success").get<bool>()?"lu":"svd");
            for(int i=0;i<3;i++)if(actual[i]!=expected[i].get<float>())throw std::runtime_error("Polynomial oracle differs at "+record.at("x").dump()+": "+std::to_string(actual[i]));
        }
        Image image{420,100,std::vector<float>(420*100*3)};
        for(int y=0;y<100;y++)for(int x=0;x<420;x++) {
            image.rgb[(y*420+x)*3]=(x+y)%256;
            image.rgb[(y*420+x)*3+1]=(x*3+y*2)%256;
            image.rgb[(y*420+x)*3+2]=(x*7+y*5)%256;
        }
        for(const auto& record:contract.at("pixels")) {
            const auto& item=contract.at("widths")[record.at("case").get<size_t>()];std::vector<Point> polygon;
            const auto& flat=item.at("polygon");for(size_t i=0;i<flat.size();i+=2)polygon.push_back({flat[i],flat[i+1]});
            PolygonMapping mapping;
            auto actual=normalize_polygon(image,polygon,item.at("maximum_width"),&mapping);
            if(actual.width!=record.at("width") || actual.height!=record.at("height"))throw std::runtime_error("Capped normalization dimensions differ");
            std::ifstream golden(directory.parent_path()/record.at("file").get<std::string>(),std::ios::binary);
            std::vector<unsigned char> expected((std::istreambuf_iterator<char>(golden)),{});
            if(expected.size()!=actual.rgb.size())throw std::runtime_error("Missing normalized pixel oracle");
            for(size_t i=0;i<expected.size();i++)if(actual.rgb[i]!=expected[i])throw std::runtime_error("Normalized pixel differs in case "+record.at("case").dump()+" at "+std::to_string(i));
            for(const auto& word:record.at("words")) {
                auto box=mapping.triangles.empty()?map_line_word(polygon,word.at("start"),word.at("end"),word.at("flipped")):
                    map_clipped_polygon_word(mapping,polygon,word.at("start"),word.at("end"),word.at("flipped"));
                Quad expected_box;
                for(int k=0;k<4;k++)expected_box[k]={word.at("box")[k*2].get<float>(),word.at("box")[k*2+1].get<float>()};
                box[0].x=std::numeric_limits<float>::quiet_NaN();
                oneocr_test::require_word_box_equal(box,expected_box);
            }
        }
        auto long_image=load_image(ONEOCR_BOUNDARY_LONG_IMAGE);
        auto resized=chw(resize_detector_pixels(long_image,1152,32),false);
        std::ifstream tensor(directory.parent_path()/"long-detector-input.f32",std::ios::binary);
        std::vector<float> expected(resized.size());
        if(!tensor.read(reinterpret_cast<char*>(expected.data()),std::streamsize(expected.size()*sizeof(float))))throw std::runtime_error("Missing long detector input oracle");
        for(size_t i=0;i<expected.size();i++)if(resized[i]!=expected[i])throw std::runtime_error("Long detector AREA input differs at "+std::to_string(i));
        std::cout<<"Curve fallback and clipped polygon DLL oracles: PASS\n";return 0;
    } catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
