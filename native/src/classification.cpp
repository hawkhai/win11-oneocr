#include "classification.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace oneocr_native {
namespace {
constexpr int script_ids[]={11,2,3,4,5,6,7,8,9,10};
int confident_script(const float* scores,const ClassificationSettings& settings) {
    int best=int(std::max_element(scores,scores+10)-scores),id=script_ids[best];
    if(id==11 && !settings.allow_nontext)return 0;
    double margin=id==11?settings.nontext_margin:settings.script_margin;
    for(int i=0;i<10;i++)if(i!=best && double(scores[best]-scores[i])<margin)return 0;
    return id;
}
int handwriting_category(float value,const ClassificationSettings& settings) {
    return double(value)<=settings.handwriting_min?1:double(value)<=settings.handwriting_max?0:2;
}
}
LineClassification classify_line(const float* scores,const std::vector<float>& script_frames,
    float handwriting,const std::vector<float>& handwriting_frames,float flip,bool horizontal,
    const ClassificationSettings& settings) {
    if(script_frames.size()%10 || handwriting_frames.size()%2)throw std::invalid_argument("Invalid classifier frame dimensions");
    LineClassification result;result.horizontal=horizontal;
    result.script=confident_script(scores,settings);
    if(!result.script) {
        std::vector<int> frames;for(size_t i=0;i<script_frames.size();i+=10)frames.push_back(confident_script(&script_frames[i],settings));
        int best=0;
        // The fixed model's EnumMapper iteration order determines equal-run ties.
        for(int script:{2,10,4,5,6,7,8,9}) {
            if(settings.script_consecutive[script]<0)continue;
            int run=0,longest=0;
            for(int frame:frames){run=frame==script?run+1:0;longest=std::max(longest,run);}
            int benefit=std::max(0,longest-settings.script_consecutive[script]);
            if(benefit>best){best=benefit;result.script=script;}
        }
    }
    float total=0;
    // Native softmax exponentiates the scores directly (without subtracting max).
    for(int i=0;i<10;i++){result.script_probabilities[i]=std::exp(scores[i]);total+=result.script_probabilities[i];}
    for(auto& value:result.script_probabilities)value/=total;
    result.style=handwriting_category(handwriting,settings);
    if(result.style==1) {
        int run=0,longest=0;
        for(size_t i=0;i<handwriting_frames.size();i+=2){run=handwriting_category(handwriting_frames[i]-handwriting_frames[i+1],settings)==2?run+1:0;longest=std::max(longest,run);}
        if(longest>settings.handwriting_consecutive)result.style=0;
    }
    result.handwriting_probability=1.f/(1.f+std::exp(-handwriting));
    result.flip=double(flip)<=settings.flip_min?2:double(flip)<=settings.flip_max?0:1;
    result.flip_probability=1.f/(1.f+std::exp(-flip));
    return result;
}
int resolve_script(const std::array<float,10>& probabilities,const std::array<size_t,12>& counts) {
    auto scores=probabilities;auto votes=counts;scores[0]=0;votes[11]=0;
    float denominator=float(std::max<size_t>(1,std::accumulate(votes.begin(),votes.end(),size_t(0))-votes[0]));
    for(int i=0;i<10;i++){float frequency=float(votes[script_ids[i]])/denominator;scores[i]=frequency+frequency+scores[i];}
    return script_ids[std::max_element(scores.begin(),scores.end())-scores.begin()];
}
void resolve_classifications(std::vector<LineClassification>& lines) {
    std::array<size_t,12> scripts{};std::array<std::array<size_t,3>,2> flips{};
    for(const auto& line:lines){scripts.at(line.script)++;if(line.script!=11 && line.flip)flips[line.horizontal][line.flip]++;}
    for(auto& line:lines) {
        if(line.script==11)continue;
        line.script_ambiguous=line.script==0;
        if(!line.script)line.script=resolve_script(line.script_probabilities,scripts);
        line.flip_ambiguous=line.flip==0;
        if(!line.flip) {
            float upright=float(flips[line.horizontal][1]),reverse=float(flips[line.horizontal][2]);
            float largest=std::max(upright,reverse),bias=.4f;
            if(largest>3)bias=(upright-reverse)/largest;
            line.flip=bias+(line.flip_probability-.5f)<0?2:1;
        }
    }
}
}
