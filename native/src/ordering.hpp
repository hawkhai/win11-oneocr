#pragma once
#include "image.hpp"
#include <vector>
#include <functional>

namespace oneocr_native {
std::vector<int32_t> native_group_order(const std::vector<int32_t>& keys);
float dominant_angle(const std::vector<float>& angles);
Quad reading_coordinates(const Quad&,float angle);
bool reading_before(const Quad& first,const Quad& second,bool right_to_left);
std::vector<size_t> reading_order(const std::vector<Quad>& boxes,float angle,bool right_to_left);
std::vector<size_t> native_sort_order(size_t count,const std::function<bool(size_t,size_t)>& before);
}
