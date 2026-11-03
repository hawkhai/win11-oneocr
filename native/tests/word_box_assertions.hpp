#pragma once
#include "image.hpp"
#include <cmath>
#include <stdexcept>

namespace oneocr_test {
inline void require_word_box_equal(const oneocr_native::Quad& actual,const oneocr_native::Quad& expected) {
    for(size_t k=0;k<4;k++) {
        const float a[]={actual[k].x,actual[k].y},e[]={expected[k].x,expected[k].y};
        for(int axis=0;axis<2;axis++) {
            if(!std::isfinite(a[axis]) || !std::isfinite(e[axis]))
                throw std::runtime_error("Nonfinite final word box coordinate");
            if(std::abs(a[axis]-e[axis])>1e-4f)
                throw std::runtime_error("Clipped inverse word mapping differs");
        }
    }
}
}
