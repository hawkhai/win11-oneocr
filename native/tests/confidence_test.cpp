#include "confidence.hpp"
#include "json.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace oneocr_native;
int main() {
    try {
        std::ifstream distance_input(ONEOCR_DISTANCE_CONTRACT);nlohmann::json distance_contract;distance_input>>distance_contract;
        for(const auto& item:distance_contract.at("cases"))if(utf8_edit_distance(item.at("first"),item.at("second"))!=item.at("distance").get<int>())throw std::runtime_error("Native UTF-8 distance differs");
        std::ifstream input(ONEOCR_CTC_CONTRACT);nlohmann::json contract;input>>contract;
        int steps=contract.at("shape")[0],vocabulary=contract.at("shape")[1];
        CharacterInfo characters;
        characters.tokens=contract.at("characters").at("tokens").get<std::vector<std::string>>();
        characters.logical_indices=contract.at("characters").at("logical_indices").get<std::vector<int>>();
        characters.types=contract.at("characters").at("character_types").get<std::vector<int>>();
        for(int i=0;i<vocabulary;i++){if(characters.types[i]==4)characters.blank=i;if(characters.types[i]==5)characters.space=i;if(characters.types[i]==8)characters.trash=i;}
        std::vector<float> logits(size_t(steps)*vocabulary,-1000);
        for(int t=0;t<steps;t++)for(const auto& entry:contract.at("input")[t])logits[size_t(t)*vocabulary+entry[0].get<int>()]=entry[1].get<float>();
        auto frames=prepare_ctc_scores(logits.data(),steps,vocabulary,characters);
        for(int t=0;t<steps;t++) {
            if(frames[t].discarded!=contract.at("discarded")[t].get<int>())throw std::runtime_error("Native blank frame handling differs at "+std::to_string(t));
            for(bool logical:{false,true}) {
                const auto& expected=contract.at("expected")[logical?"logical":"physical"][t];
                const auto& actual=logical?frames[t].logical:frames[t].physical;
                if(actual.size()!=expected.size())throw std::runtime_error("Native sparse column size differs at "+std::to_string(t));
                for(size_t i=0;i<actual.size();i++)if(actual[i].id!=expected[i][0].get<int>() || std::abs(actual[i].value-expected[i][1].get<float>())>1e-5f)
                    throw std::runtime_error("Native sparse score differs at "+std::to_string(t));
            }
        }
        for(const auto& word:contract.at("words")) {
            std::vector<DecodedCharacter> decoded;
            for(const auto& character:word.at("characters"))decoded.push_back({character.at("id"),character.at("first"),character.at("last")});
            auto actual=word_confidence_features(frames,decoded,word.at("first_frame"),word.at("last_frame"),characters);
            for(int i=0;i<21;i++)if(std::abs(actual[i]-word.at("features")[i].get<float>())>1e-5f)
                throw std::runtime_error("Native confidence feature differs for "+word.at("text").get<std::string>()+" at "+std::to_string(i)+": "+std::to_string(actual[i]));
        }
        std::cout<<"Native sparse CTC and confidence feature contracts: PASS\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
