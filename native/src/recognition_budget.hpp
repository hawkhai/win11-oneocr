#pragma once
#include "ordering.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace oneocr_native {
// Photos 11.0.15: 180497500 / 180490ca0 / 1804a4c90. A line budget
// selects whole batches of opposite-axis overlap components, before NMS.
inline std::vector<std::vector<size_t>> recognition_budget_groups(
    const std::vector<Quad>& boxes,const std::vector<bool>& horizontal,
    const std::function<bool(size_t,size_t)>& overlap) {
    auto corner=[](const Quad& q){Point p=q[0];for(auto v:q){p.x=std::min(p.x,v.x);p.y=std::min(p.y,v.y);}return p;};
    auto order=native_sort_order(boxes.size(),[&](size_t a,size_t b){auto x=corner(boxes[a]),y=corner(boxes[b]);return x.y<y.y || (x.y==y.y && x.x<y.x);});
    std::vector<bool> visited(order.size(),false);std::vector<std::vector<size_t>> groups;
    for(size_t seed=0;seed<order.size();seed++)if(!visited[seed]) {
        std::vector<size_t> group{order[seed]};visited[seed]=true;
        for(size_t cursor=0;cursor<group.size();cursor++)for(size_t j=0;j<order.size();j++)
            if(!visited[j] && horizontal[group[cursor]]!=horizontal[order[j]] && overlap(group[cursor],order[j])){visited[j]=true;group.push_back(order[j]);}
        groups.push_back(std::move(group));
    }
    return groups;
}
inline size_t recognition_batch_target(size_t remaining) {
    // Initial reserve rounds, but the actual batch uses trunc(round(remaining)*1.1).
    return std::max(size_t(4),size_t(std::round(double(remaining))*1.1));
}
}
