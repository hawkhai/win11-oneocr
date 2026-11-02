#include "confidence.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <stdexcept>

namespace oneocr_native {
static float log_add(float a,float b) {
    if(a<b)std::swap(a,b);
    if(!std::isfinite(b))return a;
    return std::log(std::exp(b-a)+1.f)+a;
}
static float find_score(const std::vector<SparseScore>& scores,int id) {
    auto it=std::lower_bound(scores.begin(),scores.end(),id,[](auto score,int index){return score.id<index;});
    return it!=scores.end() && it->id==id?it->value:-std::numeric_limits<float>::infinity();
}
std::vector<CtcFrame> prepare_ctc_scores(const float* logits,int steps,int vocabulary,const CharacterInfo& characters) {
    if(!logits || steps<1 || vocabulary<2 || characters.logical_indices.size()!=size_t(vocabulary))throw std::invalid_argument("Invalid sparse CTC input");
    std::vector<CtcFrame> result;result.reserve(steps);
    const float log_minimum=std::log(.0001f),log_blank=std::log(.99f),log_character=std::log(.05f);
    int last_blank=-1,previous_peak=characters.blank;
    for(int t=0;t<steps;t++) {
        std::vector<SparseScore> raw;
        const float* source=logits+size_t(t)*vocabulary;
        for(int i=0;i<vocabulary;i++) {
            float probability=std::exp(source[i]);
            if(probability>.0001f)raw.push_back({i,float(std::log(double(probability)+1e-36))});
        }
        int peak=characters.blank;float best=-std::numeric_limits<float>::infinity();
        for(auto score:raw)if(score.value>best){best=score.value;peak=score.id;}
        if(characters.trash>=0) {
            if(peak==characters.trash) {
                // XnnResult marks this case for the non-text route as well.
                for(auto& score:raw)if(score.id==characters.trash)score.id=characters.space;
            } else raw.erase(std::remove_if(raw.begin(),raw.end(),[&](auto score){return score.id==characters.trash;}),raw.end());
        }
        CtcFrame frame;
        float blank=find_score(raw,characters.blank);
        bool keep=blank<=log_blank;
        if(keep)last_blank=-1;
        else {
            bool preserve=false;
            if(last_blank<0) {
                frame.discarded=-2;
                if(previous_peak==characters.blank)last_blank=t;
                else {
                    float preceding=find_score(raw,previous_peak);
                    if(preceding<=log_minimum)last_blank=t;
                    preserve=preceding>log_minimum;
                }
            }
            keep=preserve;
            if(!keep) {
                frame.logical.push_back({characters.blank,blank});
                frame.discarded=t-last_blank;
            }
        }
        if(keep) {
            std::map<int,float> combined;
            for(auto score:raw) {
                int id=characters.logical_indices.at(score.id);
                auto it=combined.find(id);
                if(it==combined.end())combined.emplace(id,score.value);else it->second=log_add(it->second,score.value);
            }
            for(auto [id,value]:combined)frame.logical.push_back({id,value});
            previous_peak=peak;
        }
        if(frame.discarded==0 || frame.discarded==-2)frame.physical.push_back({characters.blank,0});
        else if(frame.discarded<0)for(auto score:frame.logical)if(score.value>log_character)frame.physical.push_back(score);
        result.push_back(std::move(frame));
    }
    return result;
}
int utf8_edit_distance(const std::string& first,const std::string& second) {
    // Native 1800d45d0 compares UTF-8 string bytes. The feature denominator
    // remains the number of recognized characters.
    const auto& a=first;const auto& b=second;std::vector<int> row(b.size()+1);std::iota(row.begin(),row.end(),0);
    for(size_t i=0;i<a.size();i++) {
        int diagonal=row[0];row[0]=int(i+1);
        for(size_t j=0;j<b.size();j++){int previous=row[j+1];row[j+1]=std::min({row[j+1]+1,row[j]+1,diagonal+(a[i]!=b[j])});diagonal=previous;}
    }
    return row.back();
}
struct Path {float score;std::vector<int> labels;};
static std::vector<std::pair<std::string,float>> word_paths(const std::vector<CtcFrame>& frames,int first,int last,const CharacterInfo& characters) {
    std::vector<Path> paths{{0,{}}};
    for(int t=first;t<=last;t++) {
        const auto& scores=frames.at(t).physical;if(scores.empty())continue;
        std::vector<Path> next;
        for(auto value:scores)for(const auto& path:paths) {
            Path candidate{path.score+value.value,path.labels};candidate.labels.push_back(value.id);next.push_back(std::move(candidate));
        }
        std::stable_sort(next.begin(),next.end(),[](const auto& a,const auto& b){return a.score>b.score;});
        if(next.size()>20)next.resize(20);
        paths=std::move(next);
    }
    std::map<std::string,float> combined;
    for(const auto& path:paths) {
        std::string word;int previous=-1;
        for(int id:path.labels) {
            if(id!=characters.blank && id!=previous) {
                if(id==characters.space)word+=' ';
                else if(id!=characters.trash)word+=characters.tokens.at(id);
            }
            previous=id;
        }
        auto begin=word.find_first_not_of(' '),end=word.find_last_not_of(' ');
        word=begin==std::string::npos?std::string{}:word.substr(begin,end-begin+1);
        if(word.empty())continue;
        auto it=combined.find(word);if(it==combined.end())combined.emplace(std::move(word),path.score);else it->second=log_add(it->second,path.score);
    }
    std::vector<std::pair<std::string,float>> result(combined.begin(),combined.end());
    std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.second>b.second;});return result;
}
std::array<float,21> word_confidence_features(const std::vector<CtcFrame>& frames,const std::vector<DecodedCharacter>& word,int first_frame,int last_frame,const CharacterInfo& characters) {
    if(word.empty())throw std::invalid_argument("Empty confidence word");
    float count=float(word.size());std::array<float,21> features{};
    std::string decoded,best_characters;int previous_best=-1,previous_end=-2;
    float sum_log=0,sum_blank=0,sum_space=0,sum_alternative=0,sum_probability=0,minimum=1;
    std::array<float,10> histogram{};
    for(auto character:word) {
        int logical=characters.logical_indices.at(character.id);decoded+=characters.tokens.at(logical);
        std::map<int,float> averages;float length=float(character.last-character.first+1);
        for(int t=character.first;t<=character.last;t++)for(auto score:frames.at(t).logical)averages[score.id]+=std::exp(score.value)/length;
        float probability=averages[logical],blank=averages[characters.blank],space=averages[characters.space],alternative=0,best_probability=0;
        int best_id=-1;
        for(auto [id,value]:averages){if(value>best_probability){best_id=id;best_probability=value;}if(id!=logical)alternative=std::max(alternative,value);}
        if(best_id>=0 && best_id!=characters.blank && (best_id!=previous_best || character.first!=previous_end+1)) {
            if(best_id==characters.space)best_characters+=' ';else best_characters+=characters.tokens.at(best_id);
        }
        previous_best=best_id;previous_end=character.last;
        probability=std::max(probability,1e-6f);minimum=std::min(minimum,probability);sum_probability+=probability;
        histogram[std::clamp(int(double(probability)/.1),0,9)]++;
        float log_probability=std::log(probability);
        sum_log+=log_probability;
        sum_blank+=log_probability+(blank>1e-9f?std::log(1.f-blank):0.f);
        sum_space+=log_probability+(space>1e-9f?std::log(1.f-space):0.f);
        sum_alternative+=log_probability+(alternative>1e-9f?std::log(1.f-alternative):0.f);
    }
    features[0]=std::log(count);features[1]=-10;features[2]=1;features[3]=-float(utf8_edit_distance(best_characters,decoded))/count;features[4]=-10;
    auto candidates=word_paths(frames,first_frame,last_frame,characters);
    if(!candidates.empty()) {
        float chosen=candidates.back().second;
        for(const auto& candidate:candidates)if(candidate.first==decoded){chosen=candidate.second;break;}
        features[1]=chosen/count;features[2]=-float(utf8_edit_distance(candidates.front().first,decoded))/count;features[4]=(chosen-candidates.front().second)/count;
    }
    features[5]=sum_log/count;features[6]=sum_alternative/count;features[7]=sum_blank/count;features[8]=sum_space/count;
    features[9]=std::log(minimum);features[10]=std::log(sum_probability/count);
    for(int i=0;i<10;i++)features[11+i]=-histogram[i]/count;
    return features;
}
}
