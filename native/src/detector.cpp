#include "detector.hpp"
#include <opencv2/core.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>

namespace oneocr_native {
static std::array<float,4> bounds(const Quad& quad) {
    std::array<float,4> result{quad[0].x,quad[0].y,quad[0].x,quad[0].y};
    for(auto point:quad) {
        result[0]=std::min(result[0],point.x);result[1]=std::min(result[1],point.y);
        result[2]=std::max(result[2],point.x);result[3]=std::max(result[3],point.y);
    }
    return result;
}
static float inclusive_iou(const Quad& first,const Quad& second) {
    auto a=bounds(first),b=bounds(second);
    if(b[2]<a[0] || b[3]<a[1] || a[2]<b[0] || a[3]<b[1])return 0;
    float width=std::max(0.f,std::min(a[2],b[2])-std::max(a[0],b[0])+1);
    float height=std::max(0.f,std::min(a[3],b[3])-std::max(a[1],b[1])+1);
    float intersection=width*height;
    return intersection/((a[2]-a[0]+1)*(a[3]-a[1]+1)+(b[2]-b[0]+1)*(b[3]-b[1]+1)-intersection);
}
std::vector<SegmentGroup> detector_segments(const float* scores,const float* deltas,const float* links,int width,int height,int level,bool horizontal) {
    if(!scores || !deltas || !links || width<=0 || height<=0 || level<2 || level>4)
        throw std::invalid_argument("Invalid detector tensor contract");
    size_t count=size_t(width)*height;
    std::vector<int64_t> parents(count,-2);
    for(size_t i=0;i<count;i++)if(scores[i]>=.7f)parents[i]=-1;
    auto find=[&](size_t index) {
        while(parents[index]>=0) {
            size_t parent=size_t(parents[index]);
            if(parents[parent]>=0)parents[index]=parents[parent];
            index=parent;
        }
        return index;
    };
    // 18015bdb0: row-major neighbours; a directed positive link unions BOTH cells.
    constexpr int dx[]={-1,0,1,-1,1,-1,0,1},dy[]={-1,-1,-1,0,0,1,1,1};
    for(size_t i=0;i<count;i++) {
        if(parents[i]==-2)continue;
        int x=int(i%width),y=int(i/width);
        for(int k=0;k<8;k++) {
            int nx=x+dx[k],ny=y+dy[k];
            if(nx<0 || nx>=width || ny<0 || ny>=height)continue;
            size_t other=size_t(ny)*width+nx;
            if(parents[other]!=-2 && links[size_t(k)*count+i]>=.8f) {
                size_t first=find(i),second=find(other);
                if(first!=second)parents[first]=int64_t(second);
            }
        }
    }
    int stride=1<<level;float anchor=float(32*(1<<(level-2))-1),center=(stride-1)*.5f;
    std::map<size_t,std::vector<Segment>> components;
    std::vector<size_t> component_order;
    for(size_t i=0;i<count;i++) {
        if(parents[i]==-2)continue;
        Segment segment{i,scores[i],{}};
        float x=float(i%width)*stride+center,y=float(i/width)*stride+center;
        // 180162830 -> 180158c40 -> 18015b100: anchor centre + delta*(anchor size-1).
        for(int k=0;k<4;k++)segment.box[k]={
            std::clamp(x+deltas[size_t(k*2)*count+i]*anchor,0.f,float(width*stride-1)),
            std::clamp(y+deltas[size_t(k*2+1)*count+i]*anchor,0.f,float(height*stride-1))};
        size_t root=find(i);
        if(!components.count(root))component_order.push_back(root);
        components[root].push_back(segment);
    }
    std::vector<SegmentGroup> groups;
    for(size_t root:component_order) {
        auto& segments=components.at(root);
        // 18015c390: descending confidence, ascending cell index for exact ties.
        std::sort(segments.begin(),segments.end(),[](const Segment& a,const Segment& b){return a.score==b.score?a.cell<b.cell:a.score>b.score;});
        SegmentGroup group{root,{}};
        for(const auto& segment:segments)
            if(std::none_of(group.segments.begin(),group.segments.end(),[&](const Segment& selected){return inclusive_iou(segment.box,selected.box)>=.5f;}))
                group.segments.push_back(segment);
        for(const auto& chain:fragment_chains(group.segments,horizontal)) {
            SegmentGroup connected{root,{}};for(size_t index:chain)connected.segments.push_back(group.segments[index]);
            groups.push_back(std::move(connected));
        }
    }
    return groups;
}
std::array<float,3> fit_line_polynomial(const std::vector<double>& x,const std::vector<double>& y) {
    if(x.empty() || x.size()!=y.size())throw std::invalid_argument("Invalid polynomial samples");
    // 1800e4ea0 -> e7000/e6a50/e7210/e74a0. Preserve the native
    // multiplication, LU factorization and triangular solve sequence.
    int degree=std::min(2,int(x.size())-1),count=degree+1;
    double system[3][3]{},rhs[3]{};
    for(size_t i=0;i<x.size();i++) {
        double basis[3]={1,x[i],x[i]*x[i]};
        for(int row=0;row<count;row++) {
            rhs[row]+=basis[row]*y[i];
            for(int column=0;column<count;column++)system[row][column]+=basis[row]*basis[column];
        }
    }
    int pivots[3]={0,1,2};
    for(int column=0;column<count;column++) {
        int pivot=column;
        for(int row=column+1;row<count;row++)if(std::abs(system[row][column])>std::abs(system[pivot][column]))pivot=row;
        if(system[pivot][column]==0) {
            // 9.0.103: e61f0 -> e48d0; Photos 11.0.15: 58b070 -> 58b580.
            // Only an exactly zero LU pivot triggers the SVD normal-equation
            // solver. Near-singular systems retain the original LU result.
            cv::Mat design(int(x.size()),count,CV_64F),samples(int(y.size()),1,CV_64F),a,b,solution;
            for(size_t i=0;i<x.size();i++){double power=1;for(int j=0;j<count;j++){design.at<double>(int(i),j)=power;power*=x[i];}samples.at<double>(int(i))=y[i];}
            cv::gemm(design,design,1,cv::Mat(),0,a,cv::GEMM_1_T);
            cv::gemm(design,samples,1,cv::Mat(),0,b,cv::GEMM_1_T);
            if(!cv::solve(a,b,solution,cv::DECOMP_SVD))throw std::runtime_error("Line curve SVD solve failed");
            std::array<float,3> result{};for(int i=0;i<count;i++)result[i]=float(solution.at<double>(i));return result;
        }
        pivots[column]=pivot;
        for(int k=0;k<count;k++)std::swap(system[column][k],system[pivot][k]);
        double reciprocal=1./system[column][column];
        for(int row=column+1;row<count;row++)system[row][column]*=reciprocal;
        for(int row=column+1;row<count;row++)for(int k=column+1;k<count;k++)system[row][k]-=system[row][column]*system[column][k];
    }
    for(int i=0;i<count;i++)std::swap(rhs[i],rhs[pivots[i]]);
    for(int i=0;i<count;i++)if(rhs[i]!=0)for(int j=i+1;j<count;j++)rhs[j]-=rhs[i]*system[j][i];
    std::array<float,3> result{};
    for(int i=count-1;i>=0;i--){rhs[i]/=system[i][i];if(rhs[i]!=0)for(int j=i-1;j>=0;j--)rhs[j]-=rhs[i]*system[j][i];}
    for(int i=0;i<count;i++)result[i]=float(rhs[i]);
    return result;
}
static float evaluate(const std::array<float,3>& coefficients,float x) {
    return (x*x)*coefficients[2]+x*coefficients[1]+coefficients[0];
}
static float evaluate_corner(const std::array<float,3>& coefficients,float x) {
    // 1800e57d0 evaluates endpoints with Horner's method. Interior samples
    // in 180112be0 use the expanded polynomial, a distinct float sequence.
    return (coefficients[2]*x+coefficients[1])*x+coefficients[0];
}
LineCurve refit_line_polygons(const std::vector<Point>& first,const std::vector<Point>& second,bool horizontal,float score) {
    std::vector<double> ux,uy,lx,ly;
    for(const auto* polygon:{&first,&second}) {
        size_t n=polygon->size()/2;
        for(size_t i=0;i<polygon->size();i++){auto p=(*polygon)[i];auto& x=i<n?ux:lx;auto& y=i<n?uy:ly;x.push_back(horizontal?p.x:p.y);y.push_back(horizontal?p.y:p.x);}
    }
    LineCurve curve{horizontal,fit_line_polynomial(ux,uy),fit_line_polynomial(lx,ly),{},score};
    auto upper=std::minmax_element(ux.begin(),ux.end()),lower=std::minmax_element(lx.begin(),lx.end());
    const float positions[]={float(*upper.first),float(*upper.second),float(*lower.second),float(*lower.first)};
    for(int i=0;i<4;i++){float dependent=evaluate_corner(i<2?curve.upper:curve.lower,positions[i]);curve.corners[i]=horizontal?Point{positions[i],dependent}:Point{dependent,positions[i]};}
    if(!horizontal)curve.corners={curve.corners[3],curve.corners[0],curve.corners[1],curve.corners[2]};return curve;
}
LineCurve fit_line_curve(const std::vector<Segment>& segments,bool horizontal,float scale_x,float scale_y) {
    if(segments.empty() || !(scale_x>0) || !(scale_y>0))throw std::invalid_argument("Invalid line curve input");
    std::vector<double> upper_x,upper_y,lower_x,lower_y;
    float score=0;
    for(const auto& segment:segments) {
        Quad quad=segment.box;
        for(auto& point:quad){point.x/=scale_x;point.y/=scale_y;}
        const int upper[]={horizontal?0:1,horizontal?1:2},lower[]={horizontal?2:3,horizontal?3:0};
        for(int k:upper){upper_x.push_back(horizontal?quad[k].x:quad[k].y);upper_y.push_back(horizontal?quad[k].y:quad[k].x);}
        for(int k:lower){lower_x.push_back(horizontal?quad[k].x:quad[k].y);lower_y.push_back(horizontal?quad[k].y:quad[k].x);}
        score+=segment.score;
    }
    LineCurve curve{horizontal,fit_line_polynomial(upper_x,upper_y),fit_line_polynomial(lower_x,lower_y),{},score/float(segments.size())};
    auto upper=std::minmax_element(upper_x.begin(),upper_x.end()),lower=std::minmax_element(lower_x.begin(),lower_x.end());
    const float positions[]={float(*upper.first),float(*upper.second),float(*lower.second),float(*lower.first)};
    for(int i=0;i<4;i++) {
        float dependent=evaluate_corner(i<2?curve.upper:curve.lower,positions[i]);
        curve.corners[i]=horizontal?Point{positions[i],dependent}:Point{dependent,positions[i]};
    }
    if(!horizontal)curve.corners={curve.corners[3],curve.corners[0],curve.corners[1],curve.corners[2]};
    return curve;
}
std::vector<Point> curve_polygon(const LineCurve& curve) {
    // 180112e30 -> 180112be0: at most eight samples per long edge.
    Quad q=curve.corners;
    if(!curve.horizontal)q={q[1],q[2],q[3],q[0]};
    float thickness=distance(q[0],q[3]),length=distance(q[0],q[1]);
    int samples=thickness>.5f?std::min(8,int(length/thickness)):0;
    std::vector<Point> points;
    for(int edge=0;edge<2;edge++) {
        int first=edge*2,last=first+1;
        points.push_back(q[first]);
        const auto& coefficients=edge?curve.lower:curve.upper;
        float start=curve.horizontal?q[first].x:q[first].y,end=curve.horizontal?q[last].x:q[last].y;
        for(int i=1;i<samples-1;i++) {
            float independent=float(i)*((end-start)/float(samples-1))+start;
            float dependent=evaluate(coefficients,independent);
            points.push_back(curve.horizontal?Point{independent,dependent}:Point{dependent,independent});
        }
        points.push_back(q[last]);
    }
    return points;
}
float line_quad_angle(const Quad& q) {
    constexpr double pi=3.14159265358979323846;
    float first=std::atan2(q[1].y-q[0].y,q[1].x-q[0].x);
    float second=std::atan2(q[2].y-q[3].y,q[2].x-q[3].x);
    double angle=double((first+second)*.5f);
    if(double(std::abs(first-second))>pi)angle+=pi;
    angle=angle/pi*180;
    if(angle<0)angle+=360;
    return float(angle);
}
Quad line_polygon_box(const std::vector<Point>& polygon,bool curved) {
    if(polygon.empty())throw std::invalid_argument("Empty line polygon");
    if(!curved){size_t n=polygon.size()/2;Quad q{polygon.front(),polygon[n-1],polygon[n],polygon.back()};for(auto& p:q){p.x=std::floor(p.x);p.y=std::floor(p.y);}return q;}
    double mean_x=0,mean_y=0;
    float left=polygon[0].x,top=polygon[0].y,right=left,bottom=top;
    for(auto p:polygon) {
        mean_x+=p.x;mean_y+=p.y;
        left=std::min(left,p.x);right=std::max(right,p.x);
        top=std::min(top,p.y);bottom=std::max(bottom,p.y);
    }
    mean_x/=polygon.size();mean_y/=polygon.size();
    double xx=0,xy=0,yy=0;
    for(auto p:polygon) {double x=p.x-mean_x,y=p.y-mean_y;xx+=x*x;xy+=x*y;yy+=y*y;}
    // The native Jacobi eigensolver's principal vector has positive x when
    // x variance dominates, and negative y when y variance dominates.
    double principal=.5*std::atan2(2*xy,xx-yy);
    if(yy>xx && std::sin(principal)>0)principal-=3.14159265358979323846;
    float angle=float(principal),cs=std::cos(angle),sn=std::sin(angle);
    left=std::floor(left);top=std::floor(top);right=std::ceil(right);bottom=std::ceil(bottom);
    float min_x=std::numeric_limits<float>::max(),min_y=min_x,max_x=-min_x,max_y=-min_x;
    for(auto p:polygon) {
        float x=(p.x-left)*cs+sn*(p.y-top),y=cs*(p.y-top)-sn*(p.x-left);
        min_x=std::min(min_x,x);max_x=std::max(max_x,x);min_y=std::min(min_y,y);max_y=std::max(max_y,y);
    }
    float inverse=1.f/(cs*cs+sn*sn),ic=cs*inverse,is=sn*inverse;
    const Point corners[]={{min_x,min_y},{max_x,min_y},{max_x,max_y},{min_x,max_y}};
    Quad box;
    for(int i=0;i<4;i++) {
        auto p=corners[i];
        box[i]={std::clamp(std::floor(-is*p.y+ic*p.x+left),left,right),std::clamp(std::floor(p.y*ic+p.x*is+top),top,bottom)};
    }
    if(angle>1.5707963267948966)box={box[2],box[3],box[0],box[1]};
    return box;
}
}
