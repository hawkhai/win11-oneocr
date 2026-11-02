#include "nms.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
namespace oneocr_native {
static bool cjk_character(char32_t c) {
    for(auto interval:{std::pair{0x4e00,0x9fff},std::pair{0x3400,0x4dbf},std::pair{0x20000,0x2a6df},std::pair{0x2a700,0x2ebef},std::pair{0x30000,0x3134f},std::pair{0x1100,0x11ff},std::pair{0x2e80,0x2fdf},std::pair{0x3000,0x318f},std::pair{0x31a0,0x33ff},std::pair{0xac00,0xd7ff},std::pair{0xff00,0xffef}})if(c>=char32_t(interval.first) && c<=char32_t(interval.second))return true;
    return false;
}
static float polygon_area(const std::vector<Point>& p) {
    double sum=0;for(size_t i=0;i<p.size();i++){auto a=p[i],b=p[(i+1)%p.size()];sum+=double(a.x)*b.y-double(b.x)*a.y;}return float(std::abs(sum)*.5);
}
float line_polygon_area(const std::vector<Point>& p) {
    // 1801b4880: float accumulation around paired edges, including final terms.
    if(p.size()<3)return 0;
    float sum=0;
    for(size_t i=1;i+1<p.size();i++)sum+=(p[i+1].y-p[i-1].y)*p[i].x;
    return std::abs(((p[1].y-p.back().y)*p[0].x+sum+(p[0].y-p[p.size()-2].y)*p.back().x)*.5f);
}
struct Triangle {std::array<Point,3> points;bool positive;};
static Triangle triangle(Point a,Point b,Point c) {
    float cross=(c.y-b.y)*(b.x-a.x)-(b.y-a.y)*(c.x-b.x);
    bool positive=std::abs(cross)<1e-8f || cross>=0;
    if(!positive)std::swap(b,c);
    return {{a,b,c},positive};
}
static float triangle_intersection(const Triangle& a,const Triangle& b) {
    // 1801b4990: preserve float determinant rounding at original coordinates.
    std::vector<Point> points(b.points.begin(),b.points.end()),next;
    for(int edge=0;edge<3;edge++) {
        Point l=a.points[edge],r=a.points[(edge+1)%3];float dx=r.x-l.x,dy=r.y-l.y;
        auto side=[&](Point p){float cross=dx*(p.y-l.y)-(p.x-l.x)*dy;return double(dx*dx+dy*dy)*1e-6<=double(cross*cross)?(cross>0?1:-1):0;};
        next.clear();
        for(size_t i=0;i<points.size();i++) {
            Point p=points[i],q=points[(i+1)%points.size()];int ps=side(p),qs=side(q);
            if(ps>=0)next.push_back(p);
            if(ps*qs<0) {
                float y=p.y-q.y,x=q.x-p.x,det=x*(l.y-r.y)-y*dx;
                if(std::abs(det)<1e-8f)throw std::runtime_error("Polygon intersection has parallel crossing edges");
                float ld=r.y*l.x-l.y*r.x,pd=p.x*q.y-p.y*q.x;
                next.push_back({(pd*dx-ld*x)/det,(ld*y-pd*(l.y-r.y))/det});
            }
        }
        points.swap(next);
    }
    return line_polygon_area(points);
}
float line_polygon_intersection(const std::vector<Point>& a,const std::vector<Point>& b) {
    // 1800e2f60: paired strips, signed triangle intersections, native sum order.
    float sum=0;
    for(size_t i=0;i+1<a.size()/2;i++) {
        Triangle left[]={triangle(a[i],a[i+1],a[a.size()-2-i]),triangle(a[a.size()-2-i],a[a.size()-1-i],a[i])};
        for(size_t j=0;j+1<b.size()/2;j++) {
            Triangle right[]={triangle(b[j],b[j+1],b[b.size()-2-j]),triangle(b[b.size()-2-j],b[b.size()-1-j],b[j])};
            for(const auto& r:right)for(const auto& l:left) {
                float value=triangle_intersection(l,r);
                sum=l.positive==r.positive?value+sum:sum-value;
            }
        }
    }
    return sum;
}
std::vector<bool> recognition_suppressed(const std::vector<RecognitionCandidate>& lines) {
    const size_t count=lines.size();std::vector<std::vector<size_t>> adjacent(count);
    std::vector<float> coverage(count);std::array<size_t,2> reliable{};size_t handwritten_cjk=0;
    for(const auto& line:lines){if(line.text.size()>=2 && line.polygon.size()>=6 && !line.min_rectangle)reliable[line.horizontal]++;if(line.script==2 && line.style==2)handwritten_cjk++;}
    for(size_t i=0;i<count;i++)for(size_t j=0;j<count;j++) {
        if(!lines[i].horizontal || lines[j].horizontal)continue;
        float overlap=line_polygon_intersection(lines[i].polygon,lines[j].polygon);
        float a=overlap/polygon_area(lines[i].polygon),b=overlap/polygon_area(lines[j].polygon);
        coverage[i]+=a;coverage[j]+=b;
        float ratio=std::max(a,b);bool shares=false;
        for(auto character:lines[i].text)if(lines[j].text.find(character)!=std::u32string::npos){shares=true;break;}
        bool linked=std::abs(ratio-.2f)<=.1f?shares:ratio>.2f;
        if(linked){adjacent[i].push_back(j);adjacent[j].push_back(i);}
    }
    std::vector<bool> visited(count),suppressed(count);
    auto relative=[](float a,float b){float largest=std::max(a,b);return double(largest)>1e-6? (a-b)/largest:0.f;};
    for(size_t seed=0;seed<count;seed++) {
        if(visited[seed])continue;
        std::vector<size_t> component{seed};visited[seed]=true;
        for(size_t i=0;i<component.size();i++)for(size_t neighbor:adjacent[component[i]])if(!visited[neighbor]){visited[neighbor]=true;component.push_back(neighbor);}
        std::array<size_t,2> counts{},characters{};std::array<float,2> language{},confidence{};size_t short_or_rect=0,cjk=0;
        for(size_t index:component){const auto& line=lines[index];size_t orientation=line.horizontal;counts[orientation]++;characters[orientation]+=line.text.size();language[orientation]+=line.language_score;confidence[orientation]+=line.confidence_score;short_or_rect+=line.polygon.size()<6 || line.text.size()==1;cjk+=std::any_of(line.text.begin(),line.text.end(),cjk_character);}
        if(!counts[0] || !counts[1])continue;
        for(int orientation=0;orientation<2;orientation++){language[orientation]/=float(counts[orientation]);confidence[orientation]/=float(counts[orientation]);}
        float score=-relative(language[1],language[0])+relative(float(characters[1]),float(characters[0]))+relative(confidence[1],confidence[0]);
        if(float(count)*.1f<=float(handwritten_cjk))score+=relative(float(reliable[1]),float(reliable[0]))*.5f;
        bool horizontal=reliable[0]<=reliable[1];
        if(cjk>=component.size()/2 || short_or_rect<=component.size()/2)horizontal=score>0;
        for(size_t index:component)if(lines[index].horizontal!=horizontal)suppressed[index]=coverage[index]>=.4f;
    }
    return suppressed;
}
static std::array<float,4> bounds(const std::vector<Point>& p) {
    std::array<float,4> r{p[0].x,p[0].y,p[0].x,p[0].y};for(auto v:p){r[0]=std::min(r[0],v.x);r[1]=std::min(r[1],v.y);r[2]=std::max(r[2],v.x);r[3]=std::max(r[3],v.y);}for(int i=0;i<4;i++)r[i]=i<2?std::floor(r[i]):std::ceil(r[i]);return r;
}
bool line_polygons_joinable(const std::vector<Point>& a,bool ah,const std::vector<Point>& b,bool bh,double divisor,double ratio) {
    // 1800e1ea0: overlap, an endpoint midpoint inside the other curve,
    // compatible thickness, and reciprocal distances to the center chords.
    if(ah!=bh)return false;auto ab=bounds(a),bb=bounds(b);
    if(ab[0]>bb[2] || ab[1]>bb[3] || bb[0]>ab[2] || bb[1]>ab[3])return false;
    if(line_polygon_intersection(a,b)<.01f)return false;
    const auto* left=&a;const auto* right=&b;
    if((ah?bb[0]:bb[3])<=(ah?ab[0]:ab[3])){left=&b;right=&a;}
    auto midpoint=[](Point x,Point y){return Point{(x.x+y.x)*.5f,(x.y+y.y)*.5f};};
    auto end=[&](const std::vector<Point>& p){return midpoint(p[p.size()/2-1],p[p.size()/2]);};
    auto start=[&](const std::vector<Point>& p){return midpoint(p.front(),p.back());};
    Point le=end(*left),rs=start(*right);
    auto inside=[](const std::vector<Point>& p,Point point){std::vector<cv::Point2f> contour;for(auto v:p)contour.emplace_back(v.x,v.y);return cv::pointPolygonTest(contour,cv::Point2f(point.x,point.y),false)>=0;};
    if(!inside(*left,rs) && !inside(*right,le))return false;
    float lh=distance((*left)[left->size()/2-1],(*left)[left->size()/2]),rh=distance(right->front(),right->back());
    if(double(std::max(lh,rh))>=double(std::min(lh,rh))*ratio || divisor==0)return false;
    double tolerance=double(std::min(lh,rh))/divisor;
    auto near=[&](const std::vector<Point>& polygon,Point p){auto s=start(polygon),e=end(polygon);float dx=e.x-s.x,dy=e.y-s.y;float numerator=ah?(e.y-s.y)*p.x+(s.x-e.x)*p.y+(e.x*s.y-e.y*s.x):(e.x-s.x)*p.y+(s.y-e.y)*p.x+(e.y*s.x-e.x*s.y);return double(std::abs(numerator)/std::sqrt(dx*dx+dy*dy))<tolerance;};
    return near(*left,rs) && near(*right,le);
}
static std::array<float,3> center_fit(const std::vector<Point>& polygon,bool horizontal) {
    int n=int(polygon.size()/2);
    if(n==2) {
        auto a=polygon[0],b=polygon[3],c=polygon[1],d=polygon[2];
        float x0=horizontal?(a.x+b.x)*.5f:(a.y+b.y)*.5f,x1=horizontal?(c.x+d.x)*.5f:(c.y+d.y)*.5f;
        float y0=horizontal?(a.y+b.y)*.5f:(a.x+b.x)*.5f,y1=horizontal?(c.y+d.y)*.5f:(c.x+d.x)*.5f;
        float slope=x1!=x0?(y1-y0)/(x1-x0):0;return {y0-slope*x0,slope,0};
    }
    cv::Mat system=cv::Mat::zeros(3,3,CV_64F),rhs=cv::Mat::zeros(3,1,CV_64F),coefficients;
    for(int i=0;i<n;i++) {
        auto a=polygon[i],b=polygon[polygon.size()-1-i];float cx=(a.x+b.x)*.5f,cy=(a.y+b.y)*.5f;
        double x=horizontal?cx:cy,y=horizontal?cy:cx,powers[]={1,x,x*x,x*x*x,x*x*x*x};
        for(int row=0;row<3;row++){rhs.at<double>(row)+=powers[row]*y;for(int col=0;col<3;col++)system.at<double>(row,col)+=powers[row+col];}
    }
    if(!cv::solve(system,rhs,coefficients,cv::DECOMP_LU)) {
        // A two-point line has no unique quadratic: its linear solution is exact.
        std::vector<cv::Point2f> points;for(int i=0;i<n;i++){auto a=polygon[i],b=polygon[polygon.size()-1-i];points.emplace_back((a.x+b.x)*.5f,(a.y+b.y)*.5f);}
        float x0=horizontal?points.front().x:points.front().y,x1=horizontal?points.back().x:points.back().y;
        float y0=horizontal?points.front().y:points.front().x,y1=horizontal?points.back().y:points.back().x;
        float slope=x1!=x0?(y1-y0)/(x1-x0):0;return {y0-slope*x0,slope,0};
    }
    return {float(coefficients.at<double>(0)),float(coefficients.at<double>(1)),float(coefficients.at<double>(2))};
}
bool line_polygon_covered(const std::vector<Point>& a,bool ah,const std::vector<Point>& b,bool bh,float threshold) {
    // 1800e2380, including its early TRUE branches recovered from assembly.
    if(ah!=bh)return false;auto ab=bounds(a),bb=bounds(b);int along=ah?0:1,across=1-along;
    float center=0;for(auto p:a)center+=ah?p.x:p.y;center/=a.size();
    if(center<bb[along] || center>bb[along+2])return false;
    float area=polygon_area(a);if(area<=0)return false;
    float coverage=line_polygon_intersection(a,b)/area;
    if(coverage<=threshold)return false;if(coverage>=.99f)return true;
    float side_overlap=std::max(0.f,std::min(ab[across+2],bb[across+2])-std::max(ab[across],bb[across]));
    float side_span=std::max(ab[across+2],bb[across+2])-std::min(ab[across],bb[across]);
    float along_overlap=std::max(0.f,std::min(ab[along+2],bb[along+2])-std::max(ab[along],bb[along]));
    if(side_span>0 && side_overlap/side_span>.7f && along_overlap/(ab[along+2]-ab[along])<.6f)return false;
    if(ab[along+2]-ab[along]<(bb[along+2]-bb[along])*.2f)return true;
    auto coefficients=center_fit(b,bh);
    auto thickness=[](const std::vector<Point>& p){float sum=0;size_t n=p.size()/2;for(size_t i=0;i<n;i++)sum+=distance(p[i],p[p.size()-1-i]);return sum/float(n);};
    float tolerance=std::min(thickness(a),thickness(b))*.2f;bool above=false,below=false;
    for(auto p:a){float x=bh?p.x:p.y,y=bh?p.y:p.x,curve=coefficients[0]+x*coefficients[1]+(x*x)*coefficients[2];above|=y-curve>tolerance;below|=curve-y>tolerance;}
    return above && below;
}
}
