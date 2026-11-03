#include "word_box_assertions.hpp"
#include <iostream>
#include <limits>
using namespace oneocr_native;
using oneocr_test::require_word_box_equal;
int main() {
    try {
        Quad golden{{{10,20},{30,20},{30,40},{10,40}}};
        require_word_box_equal(golden,golden);
        int rejected=0;
        const float invalid[]={std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()};
        for(int side=0;side<2;side++)for(int corner=0;corner<4;corner++)for(int axis=0;axis<2;axis++)for(float value:invalid) {
            auto actual=golden,expected=golden;auto& point=(side==0?actual:expected)[corner];
            (axis==0?point.x:point.y)=value;
            bool failed=false;
            try {require_word_box_equal(actual,expected);}catch(const std::runtime_error&){failed=true;}
            if(!failed)throw std::runtime_error("Nonfinite word box was accepted");
            rejected++;
        }
        for(int corner=0;corner<4;corner++)for(int axis=0;axis<2;axis++) {
            auto actual=golden;auto& coordinate=axis==0?actual[corner].x:actual[corner].y;
            coordinate+=.00005f;require_word_box_equal(actual,golden);
            coordinate+=.01f;bool failed=false;
            try {require_word_box_equal(actual,golden);}catch(const std::runtime_error&){failed=true;}
            if(!failed)throw std::runtime_error("Out-of-tolerance word box was accepted");
        }
        std::cout<<"Final word boxes: "<<rejected<<" nonfinite negatives, 8 finite out-of-tolerance negatives and finite controls PASS\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
