#include "alignment.hpp"
#include <algorithm>
#include <limits>
namespace oneocr_native {
void align_word_frames(std::vector<WordFrames>& words,int stride,int pixels_per_space,int start_shift,int end_shift,int steps,const std::vector<std::array<int,2>>& blanks) {
    // OneOCR 9.0.103, 1800a1bb0. Blank intervals come from 1800a8f60.
    size_t blank_index=0;
    for(size_t i=0;i<words.size();i++) {
        auto& word=words[i];
        int start=std::max(i?words[i-1].end:0,stride*word.first+start_shift);
        word.start=start;
        int maximum=i+1==words.size()?(steps-1)*stride:stride*words[i+1].first-pixels_per_space+start_shift;
        word.end=std::max(start+stride,std::min(stride*word.last+end_shift,maximum));
        if(i==0) {
            if(blanks.empty() || start<blanks.front()[0])word.start=0;
        } else {
            auto& previous=words[i-1];int first=std::numeric_limits<int>::max(),last=0;
            while(blank_index<blanks.size()) {
                auto blank=blanks[blank_index];
                if(start<=blank[0])break;
                int center=(blank[0]+blank[1])/2;
                if(previous.end<=center && center<=start){last=blank[1];if(first==std::numeric_limits<int>::max())first=blank[0];}
                blank_index++;
            }
            if(last>0 && first<std::numeric_limits<int>::max() && previous.start+stride<first && last<word.end-stride && (start-previous.end)/(last-first)<3) {
                word.start=last;previous.end=first;
            }
        }
        if(i+1==words.size() && (blanks.empty() || blanks.back()[1]<word.end))word.end=stride*steps;
    }
}
}
