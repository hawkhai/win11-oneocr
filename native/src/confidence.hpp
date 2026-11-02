#pragma once
#include <array>
#include <string>
#include <vector>
namespace oneocr_native {
struct CharacterInfo {
    std::vector<int> logical_indices,types;
    std::vector<std::string> tokens;
    int blank=-1,space=-1,trash=-1;
};
struct DecodedCharacter {int id,first,last;};
struct SparseScore {int id;float value;};
struct CtcFrame {std::vector<SparseScore> logical,physical;int discarded=-1;};
int utf8_edit_distance(const std::string& first,const std::string& second);
std::vector<CtcFrame> prepare_ctc_scores(const float* logits,int steps,int vocabulary,const CharacterInfo& characters);
std::array<float,21> word_confidence_features(const std::vector<CtcFrame>& frames,const std::vector<DecodedCharacter>& word,int first_frame,int last_frame,const CharacterInfo& characters);
}
