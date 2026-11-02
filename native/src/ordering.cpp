#include "ordering.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <functional>
#include <cstdint>
#include <list>
#include <map>

namespace oneocr_native {
std::vector<int32_t> native_group_order(const std::vector<int32_t>& keys) {
    // 1800FEB20 and 1800B8170: MSVC FNV-1a int keys, 1.0 load factor,
    // growth by eight below 512 buckets, and insertion before bucket front.
    std::list<int32_t> order;using Iterator=std::list<int32_t>::iterator;
    size_t buckets=8;std::vector<Iterator> first(buckets,order.end());std::map<int32_t,Iterator> nodes;
    auto hash=[](int32_t key){uint64_t value=0xcbf29ce484222325ULL;for(int i=0;i<4;i++){value^=(uint32_t(key)>>(8*i))&255;value*=0x100000001b3ULL;}return value;};
    for(int32_t key:keys) {
        if(nodes.count(key))continue;
        if(nodes.size()+1>buckets) {
            size_t needed=nodes.size()+1;buckets=buckets<512 && buckets*8>=needed?buckets*8:buckets*2;
            first.assign(buckets,order.end());
            std::vector<int32_t> existing(order.begin(),order.end());
            for(int32_t item:existing){size_t b=size_t(hash(item))&(buckets-1);auto node=nodes.at(item);if(first[b]!=order.end())order.splice(first[b],order,node);first[b]=node;}
        }
        size_t b=size_t(hash(key))&(buckets-1);auto node=order.insert(first[b],key);nodes.emplace(key,node);first[b]=node;
    }
    return {order.begin(),order.end()};
}
float dominant_angle(const std::vector<float>& angles) {
    // 180064410: most populated nearest-cardinal bin, then its upper median.
    std::array<std::vector<float>,4> bins;
    for(float angle:angles) {
        while(angle< -45)angle+=360;
        while(angle>=315)angle-=360;
        bins[size_t(int(angle+45)/90)].push_back(angle);
    }
    size_t best=0;
    for(size_t i=1;i<bins.size();i++)if(bins[i].size()>bins[best].size())best=i;
    auto& selected=bins[best];
    if(selected.empty())return 0;
    std::sort(selected.begin(),selected.end());
    float angle=selected[selected.size()/2];
    return angle<0?angle+360:angle;
}
Quad reading_coordinates(const Quad& box,float angle) {
    // 1800641f0: rotate each oriented corner into the page's reading coordinates.
    float radians=float(double(angle)*3.141592653589793/180.0);
    float cosine=std::cos(radians),sine=std::sin(radians);
    Quad rotated{};
    for(size_t i=0;i<4;i++)rotated[i]={cosine*box[i].x+sine*box[i].y,cosine*box[i].y-sine*box[i].x};
    return rotated;
}
bool reading_before(const Quad& a,const Quad& b,bool rtl) {
    // 180188600. Small horizontal overlap and vertical separation within one
    // text height identify a row; other pairs compare their reading-axis Y.
    float ax=rtl?a[1].x:a[0].x,ay=rtl?a[1].y:a[0].y;
    float bx=rtl?b[1].x:b[0].x,by=rtl?b[1].y:b[0].y;
    float intersection=std::max(0.f,std::min(a[1].x,b[1].x)-std::max(a[0].x,b[0].x));
    float span=std::abs(std::max(a[1].x,b[1].x)-std::min(a[0].x,b[0].x)+1);
    float height_a=rtl?a[2].y-a[1].y:a[3].y-a[0].y;
    float height_b=rtl?b[2].y-b[1].y:b[3].y-b[0].y;
    if(intersection/span<=.2f && std::abs(ay-by)<=std::min(std::abs(height_a),std::abs(height_b)))
        return rtl?bx<ax:ax<bx;
    return ay<by;
}
std::vector<size_t> native_sort_order(size_t count,const std::function<bool(size_t,size_t)>& before) {
    std::vector<size_t> result(count);std::iota(result.begin(),result.end(),0);
    // Original MSVC introsort sequence: equal keys and nontransitive geometry
    // must produce the same order on every target STL.
    auto less=[&](size_t a,size_t b){return before(result[a],result[b]);};
    auto median=[&](size_t a,size_t b,size_t c){if(less(b,a))std::swap(result[a],result[b]);if(less(c,b)){std::swap(result[b],result[c]);if(less(b,a))std::swap(result[a],result[b]);}};
    auto sift=[&](size_t base,size_t hole,size_t count,size_t value){
        size_t top=hole,bottom=(count-1)/2;
        while(hole<bottom){size_t right=hole*2+2,child=less(base+right,base+right-1)?right-1:right;result[base+hole]=result[base+child];hole=child;}
        if(hole==bottom && count%2==0){result[base+hole]=result[base+count-1];hole=count-1;}
        while(hole>top){size_t parent=(hole-1)/2;if(!before(result[base+parent],value))break;result[base+hole]=result[base+parent];hole=parent;}
        result[base+hole]=value;
    };
    auto partition=[&](size_t first,size_t last){
        size_t mid=first+(last-first)/2,end=last-1;
        if(end-first>40){size_t step=(end-first+1)/8;median(first,first+step,first+step*2);median(mid-step,mid,mid+step);median(end-step*2,end-step,end);median(first+step,mid,end-step);}else median(first,mid,end);
        size_t pivot_first=mid,pivot_end=mid+1;
        while(first<pivot_first && !less(pivot_first-1,pivot_first) && !less(pivot_first,pivot_first-1))--pivot_first;
        while(pivot_end<last && !less(pivot_end,pivot_first) && !less(pivot_first,pivot_end))++pivot_end;
        size_t right=pivot_end,left=pivot_first;
        for(;;){
            for(;right<last;right++){if(less(pivot_first,right))continue;if(less(right,pivot_first))break;if(pivot_end!=right)std::swap(result[pivot_end],result[right]);pivot_end++;}
            for(;first<left;left--){if(less(left-1,pivot_first))continue;if(less(pivot_first,left-1))break;--pivot_first;if(pivot_first!=left-1)std::swap(result[pivot_first],result[left-1]);}
            if(left==first && right==last)return std::array<size_t,2>{pivot_first,pivot_end};
            if(left==first){if(pivot_end!=right)std::swap(result[pivot_first],result[pivot_end]);pivot_end++;std::swap(result[pivot_first],result[right]);pivot_first++;right++;}
            else if(right==last){--left;--pivot_first;if(left!=pivot_first)std::swap(result[left],result[pivot_first]);std::swap(result[pivot_first],result[--pivot_end]);}
            else {std::swap(result[right],result[--left]);right++;}
        }
    };
    std::function<void(size_t,size_t,size_t)> sort=[&](size_t first,size_t last,size_t ideal){
        for(;;){
            if(last-first<=32){
                for(size_t i=first+1;i<last;i++){size_t value=result[i],position=i;
                    if(before(value,result[first])){while(position>first){result[position]=result[position-1];--position;}}
                    else while(position>first && before(value,result[position-1])){result[position]=result[position-1];--position;}
                    result[position]=value;
                }return;
            }
            if(!ideal){size_t count=last-first;for(size_t hole=count/2;hole;){--hole;sift(first,hole,count,result[first+hole]);}for(size_t count_left=count;count_left>1;--count_left){size_t value=result[first+count_left-1];result[first+count_left-1]=result[first];sift(first,0,count_left-1,value);}return;}
            auto pivot=partition(first,last);ideal=(ideal>>1)+(ideal>>2);
            if(pivot[0]-first<last-pivot[1]){sort(first,pivot[0],ideal);first=pivot[1];}else{sort(pivot[1],last,ideal);last=pivot[0];}
        }
    };
    if(!result.empty())sort(0,result.size(),result.size());
    return result;
}
std::vector<size_t> reading_order(const std::vector<Quad>& boxes,float angle,bool rtl) {
    std::vector<Quad> transformed;
    for(const auto& box:boxes)transformed.push_back(reading_coordinates(box,angle));
    return native_sort_order(boxes.size(),[&](size_t a,size_t b){return reading_before(transformed[a],transformed[b],rtl);});
}
}
