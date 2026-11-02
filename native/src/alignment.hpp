#pragma once
#include <array>
#include <vector>
namespace oneocr_native {
struct WordFrames {int first,last,start=0,end=0;};
void align_word_frames(std::vector<WordFrames>& words,int stride,int pixels_per_space,int start_shift,int end_shift,int steps,const std::vector<std::array<int,2>>& blanks);
}
