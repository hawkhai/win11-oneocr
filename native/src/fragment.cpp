#include "detector.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace oneocr_native {
std::vector<std::vector<size_t>> fragment_chains(const std::vector<Segment>& segments,bool horizontal) {
    // 18015d380 / 18015d8a0: successor selection and predecessor conflict repair.
    const size_t n=segments.size();if(!n)return {};if(n==1)return {{0}};
    std::vector<Point> centers(n);std::vector<float> slopes(n),keys(n),areas(n);
    std::vector<std::array<float,4>> bounds(n);
    auto midpoint=[](Point a,Point b){return Point{(a.x+b.x)*.5f,(a.y+b.y)*.5f};};
    auto slope=[&](Point a,Point b){float denominator=horizontal?a.x-b.x:a.y-b.y;return denominator!=0?(horizontal?a.y-b.y:a.x-b.x)/denominator:std::numeric_limits<float>::max();};
    for(size_t i=0;i<n;i++) {
        const auto& q=segments[i].box;
        centers[i]={(q[0].x+q[1].x+q[2].x+q[3].x)*.25f,(q[0].y+q[1].y+q[2].y+q[3].y)*.25f};
        slopes[i]=horizontal?slope(midpoint(q[1],q[2]),midpoint(q[0],q[3])):slope(midpoint(q[2],q[3]),midpoint(q[0],q[1]));
        keys[i]=horizontal?(q[0].x+q[3].x)*.5f:(q[0].y+q[1].y)*.5f;
        bounds[i]={q[0].x,q[0].y,q[0].x,q[0].y};
        double area=0;for(size_t k=0;k<4;k++){auto a=q[k],b=q[(k+1)%4];area+=double(a.x)*b.y-double(b.x)*a.y;bounds[i][0]=std::min(bounds[i][0],a.x);bounds[i][1]=std::min(bounds[i][1],a.y);bounds[i][2]=std::max(bounds[i][2],a.x);bounds[i][3]=std::max(bounds[i][3],a.y);}areas[i]=float(std::abs(area)*.5);
    }
    auto overlap=[&](size_t i,size_t j){
        auto a=bounds[i],b=bounds[j];if(!(a[0]<b[2] && a[1]<b[3] && b[0]<a[2] && b[1]<a[3]))return 0.f;
        auto triangles=[&](size_t index){const auto& q=segments[index].box;return std::array<std::vector<cv::Point2f>,2>{std::vector<cv::Point2f>{{q[0].x,q[0].y},{q[1].x,q[1].y},{q[2].x,q[2].y}},std::vector<cv::Point2f>{{q[0].x,q[0].y},{q[2].x,q[2].y},{q[3].x,q[3].y}}};};
        float intersection=0;std::vector<cv::Point2f> output;
        for(const auto& left:triangles(i))for(const auto& right:triangles(j)){float area=cv::intersectConvexConvex(left,right,output,true);intersection+=cv::contourArea(left,true)*cv::contourArea(right,true)>=0?area:-area;}
        return std::abs(intersection)>=1e-8f?intersection/(areas[j]+areas[i]-intersection):0.f;
    };
    std::vector<size_t> order(n);std::iota(order.begin(),order.end(),0);std::stable_sort(order.begin(),order.end(),[&](size_t a,size_t b){return keys[a]<keys[b];});
    std::vector<int64_t> next(n,-1),previous(n,-1);
    for(size_t position=0;position<n;position++) {
        size_t source=order[position];int64_t best=-1,second=-1;float best_score=-std::numeric_limits<float>::max(),second_score=best_score;
        for(size_t p=position+1;p<n;p++){size_t target=order[p];float score=overlap(source,target);if(score<=best_score){if(second_score<score){second=int64_t(target);second_score=score;}}else{second=best;second_score=best_score;best=int64_t(target);best_score=score;}}
        if(best<0)continue;int64_t target=best;
        if(second>=0) {
            float first_slope=slope(centers[source],centers[size_t(best)]),second_slope=slope(centers[source],centers[size_t(second)]);
            if(best_score<.15f || second_score<.15f || best_score-second_score>=.15f) {
                if(best_score>=.01f){if(!(best_score>0))continue;}
                else {
                    bool choose_second;
                    if(std::abs(first_slope-second_slope)>.15f)choose_second=std::abs(slopes[source]-second_slope)<std::abs(slopes[source]-first_slope);
                    else {auto square_distance=[&](size_t t){float y=centers[source].y-centers[t].y,x=centers[source].x-centers[t].x;return std::pow(y,2.f)+std::pow(x,2.f);};choose_second=square_distance(size_t(second))<square_distance(size_t(best));}
                    target=choose_second?second:best;
                }
            } else target=std::abs(slopes[source]-first_slope)<=std::abs(slopes[source]-second_slope)?best:second;
        }
        size_t t=size_t(target);
        if(previous[t]<0){next[source]=target;previous[t]=int64_t(source);continue;}
        size_t old=size_t(previous[t]);float old_slope=slope(centers[old],centers[t]),new_slope=slope(centers[source],centers[t]);
        if(std::abs(slopes[t]-new_slope)<=std::abs(slopes[t]-old_slope)) {
            next[source]=target;previous[t]=int64_t(source);
            if(std::abs(slopes[t]-old_slope)<.15f && previous[source]<0){next[old]=int64_t(source);previous[source]=int64_t(old);}else next[old]=-1;
        }
    }
    std::vector<std::vector<size_t>> result;
    for(size_t i=0;i<n;i++)if(previous[i]<0 && next[i]>=0){std::vector<size_t> chain;for(int64_t j=int64_t(i);j>=0;j=next[size_t(j)]){chain.push_back(size_t(j));if(chain.size()>n)throw std::runtime_error("Invalid detector fragment cycle");}result.push_back(std::move(chain));}
    return result;
}
}
