#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace oneocr_native {
// Script IDs are the original TextScript enum: 0 ambiguous, 2 CJK,
// 3 Latin ... 10 Tamil, 11 non-text. They are not network output indices.
struct ClassificationSettings {
    double script_margin=4.1,nontext_margin=5;
    double handwriting_min=-2,handwriting_max=0,flip_min=-5,flip_max=5;
    int handwriting_consecutive=3;
    std::array<int,12> script_consecutive{-1,-1,3,-1,1,-1,2,3,3,3,3,-1};
    bool allow_nontext=true;
};
struct LineClassification {
    int script=0,style=0,flip=0;
    bool horizontal=true,flip_ambiguous=false,script_ambiguous=false;
    float handwriting_probability=0,flip_probability=0;
    std::array<float,10> script_probabilities{};
};
LineClassification classify_line(const float* scores,const std::vector<float>& script_frames,
    float handwriting,const std::vector<float>& handwriting_frames,float flip,bool horizontal,
    const ClassificationSettings& settings);
void resolve_classifications(std::vector<LineClassification>& lines);
int resolve_script(const std::array<float,10>& probabilities,const std::array<size_t,12>& counts);
}
