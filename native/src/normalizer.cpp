#include "normalizer.hpp"
#include "trace.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace oneocr_native {
Image resize_detector_pixels(const Image& image,int width,int height) {
    cv::Mat source(image.height,image.width,CV_32FC3,const_cast<float*>(image.rgb.data())),target;
    // ImageUtility's area-ratio map chooses AREA at <= 0.04, LINEAR above.
    // The 10000-pixel long-line fixture crosses this existing DLL boundary.
    double ratio=(double(width)*height)/(double(image.width)*image.height);
    cv::resize(source,target,cv::Size(width,height),0,0,ratio<=.04?cv::INTER_AREA:cv::INTER_LINEAR);
    Image result{width,height,std::vector<float>(size_t(width)*height*3)};
    std::copy_n(target.ptr<float>(),result.rgb.size(),result.rgb.begin());return result;
}
QuadNormalization normalize_quad(const Image& image,const std::vector<Point>& polygon,int maximum_width) {
    if(polygon.size()!=4)throw std::invalid_argument("Quadrilateral normalization requires four points");
    float minx=polygon[0].x,miny=polygon[0].y,maxx=minx,maxy=miny;
    for(auto p:polygon){minx=std::min(minx,p.x);miny=std::min(miny,p.y);maxx=std::max(maxx,p.x);maxy=std::max(maxy,p.y);}
    int ox=int(std::floor(minx)),oy=int(std::floor(miny)),right=int(std::ceil(maxx)),bottom=int(std::ceil(maxy));
    cv::Rect original_roi(ox,oy,right-ox+1,bottom-oy+1);original_roi&=cv::Rect(0,0,image.width,image.height);
    cv::Mat original_float(image.height,image.width,CV_32FC3,const_cast<float*>(image.rgb.data())),pixels;
    original_float(original_roi).convertTo(pixels,CV_8UC3);
    std::vector<cv::Point2f> points;for(auto p:polygon)points.emplace_back(std::floor(p.x)-original_roi.x,std::floor(p.y)-original_roi.y);
    double first=std::atan2(double(points[1].y-points[0].y),double(points[1].x-points[0].x));
    double second=std::atan2(double(points[2].y-points[3].y),double(points[2].x-points[3].x));
    double angle=(first+second)*.5*(180/3.14159265358979323846);
    cv::Mat transform=cv::Mat::eye(2,3,CV_32F),rotated=pixels;
    if(angle!=0 && std::min(pixels.cols,pixels.rows)>2) {
        auto matrix=cv::getRotationMatrix2D(cv::Point2f((pixels.cols-1)*.5f,(pixels.rows-1)*.5f),angle,1);matrix.convertTo(transform,CV_32F);
        auto bounds=cv::RotatedRect(cv::Point2f(0,0),cv::Size2f(float(pixels.cols),float(pixels.rows)),float(angle)).boundingRect2f();
        transform.at<float>(0,2)+=(bounds.width-float(pixels.cols))*.5f;transform.at<float>(1,2)+=(bounds.height-float(pixels.rows))*.5f;
        cv::warpAffine(pixels,rotated,transform,{cvRound(bounds.width),cvRound(bounds.height)},cv::INTER_LINEAR,cv::BORDER_CONSTANT,cv::Scalar::all(255));
    }
    int left=INT_MAX,top=INT_MAX,last_x=-1,last_y=-1;
    for(auto p:points){int x=int(transform.at<float>(0,0)*p.x+transform.at<float>(0,1)*p.y+transform.at<float>(0,2)),y=int(transform.at<float>(1,0)*p.x+transform.at<float>(1,1)*p.y+transform.at<float>(1,2));left=std::min(left,x);top=std::min(top,y);last_x=std::max(last_x,x);last_y=std::max(last_y,y);}
    cv::Rect roi(left,top,std::max(1,last_x-left),std::max(1,last_y-top));roi&=cv::Rect(0,0,rotated.cols,rotated.rows);
    if(roi.empty())throw std::invalid_argument("Quadrilateral normalization crop is empty");
    double scale=60./roi.height;int width=std::min(maximum_width,std::max(1,int(roi.width*scale)));
    TensorTrace trace("quad");trace.document("normalization",{{"source_roi",{original_roi.x,original_roi.y,original_roi.width,original_roi.height}},{"rotated",{rotated.cols,rotated.rows}},{"crop",{roi.x,roi.y,roi.width,roi.height}},{"width",width},{"angle",angle}});
    cv::Mat resized;cv::resize(rotated(roi),resized,{width,60},0,0,cv::INTER_LINEAR);
    transform.at<float>(0,2)-=float(left);transform.at<float>(1,2)-=float(top);
    cv::Mat forward(2,3,CV_32F);
    for(int y=0;y<2;y++)for(int x=0;x<3;x++)forward.at<float>(y,x)=transform.at<float>(y,x)*float(scale);
    cv::Mat inverse;cv::invertAffineTransform(forward,inverse);
    QuadNormalization result{{width,60,std::vector<float>(size_t(width)*60*3)},{},{float(ox),float(oy),float(right),float(bottom)}};
    for(int i=0;i<6;i++)result.inverse[i]=inverse.ptr<float>()[i];
    result.inverse[2]+=float(original_roi.x);result.inverse[5]+=float(original_roi.y);
    for(int y=0;y<60;y++)for(int x=0;x<width;x++)for(int c=0;c<3;c++)result.image.rgb[(size_t(y)*width+x)*3+c]=resized.at<cv::Vec3b>(y,x)[c];
    return result;
}
Quad map_quad_word(const QuadNormalization& normalized,int start,int end,bool flipped) {
    Quad q{{{float(start),0},{float(end),0},{float(end),60},{float(start),60}}};
    if(flipped)for(int k=0;k<4;k++){q[k].x=float(normalized.image.width-((k==0 || k==3)?1:0))-q[k].x;q[k].y=60-q[k].y;}
    const auto& m=normalized.inverse;for(auto& p:q){float x=m[0]*p.x+m[1]*p.y+m[2],y=m[3]*p.x+m[4]*p.y+m[5];p={std::clamp(x,normalized.bounds[0],normalized.bounds[2]),std::clamp(y,normalized.bounds[1],normalized.bounds[3])};}return q;
}
std::vector<std::array<int,2>> line_blank_intervals(const Image& image) {
    cv::Mat source_float(image.height,image.width,CV_32FC3,const_cast<float*>(image.rgb.data())),pixels,gray,binary;
    source_float.convertTo(pixels,CV_8UC3);
    cv::cvtColor(pixels,gray,cv::COLOR_RGB2GRAY);
    cv::adaptiveThreshold(gray,binary,255,cv::ADAPTIVE_THRESH_MEAN_C,cv::THRESH_BINARY_INV,61,10);
    std::vector<std::array<int,2>> intervals;
    int first=0;
    for(int x=0;x<image.width;x++) {
        bool constant=true;auto value=binary.at<uint8_t>(5,x);
        for(int y=6;y<image.height-5;y++)if(binary.at<uint8_t>(y,x)!=value){constant=false;break;}
        if(!constant) {
            if(x-first>11)intervals.push_back({first,x});
            first=x+1;
        }
    }
    return intervals;
}
static std::vector<float> polygon_positions(const std::vector<Point>& polygon) {
    const size_t half=polygon.size()/2;
    float height=0;
    for(size_t i=0;i<half;i++) {
        float x=polygon[i].x-polygon[polygon.size()-1-i].x,y=polygon[i].y-polygon[polygon.size()-1-i].y;
        height+=std::sqrt(x*x+y*y);
    }
    height/=float(half);float scale=60.f/height;
    std::vector<float> positions{0};
    for(size_t i=0;i<half-1;i++) {
        float x=polygon[i].x-polygon[i+1].x,y=polygon[i].y-polygon[i+1].y;
        positions.push_back(std::sqrt(x*x+y*y)*scale+positions.back());
    }
    return positions;
}
Quad map_line_word(const std::vector<Point>& polygon,int start,int end,bool flipped) {
    auto positions=polygon_positions(polygon);size_t half=positions.size();
    int width=int(positions.back()+1);
    Quad normalized{{{float(start),0},{float(end),0},{float(end),60},{float(start),60}}};
    if(flipped)for(int k=0;k<4;k++) {
        auto& p=normalized[k];p.x=float(width-((k==0 || k==3)?1:0))-p.x;p.y=60-p.y;
    }
    Point minimum=polygon[0],maximum=polygon[0];
    for(auto p:polygon){minimum.x=std::min(minimum.x,p.x);minimum.y=std::min(minimum.y,p.y);maximum.x=std::max(maximum.x,p.x);maximum.y=std::max(maximum.y,p.y);}
    minimum={std::floor(minimum.x),std::floor(minimum.y)};maximum={std::ceil(maximum.x),std::ceil(maximum.y)};
    Quad result;
    for(int k=0;k<4;k++) {
        Point p=normalized[k];size_t i=0;
        while(i+2<half && positions[i+1]<p.x)i++;
        bool upper=p.y<=59.f*(positions[i+1]-p.x)/(positions[i+1]-positions[i]);
        size_t indices[3]={upper?i:polygon.size()-1-i,upper?i+1:polygon.size()-2-i,upper?polygon.size()-1-i:i+1};
        std::vector<cv::Point2f> src,dst;
        for(size_t j:indices) {
            bool top=j<half;size_t n=top?j:polygon.size()-1-j;
            src.emplace_back(positions[n],top?0.f:59.f);dst.emplace_back(polygon[j].x,polygon[j].y);
        }
        auto matrix=cv::getAffineTransform(src,dst);
        float x=float(matrix.at<double>(0,0))*p.x+float(matrix.at<double>(0,1))*p.y+float(matrix.at<double>(0,2));
        float y=float(matrix.at<double>(1,0))*p.x+float(matrix.at<double>(1,1))*p.y+float(matrix.at<double>(1,2));
        result[k]={std::clamp(x,minimum.x,maximum.x),std::clamp(y,minimum.y,maximum.y)};
    }
    return result;
}
Image resize_line_pixels(const Image& image,int width,int height) {
    cv::Mat source_float(image.height,image.width,CV_32FC3,const_cast<float*>(image.rgb.data())),pixels,resized;
    source_float.convertTo(pixels,CV_8UC3);
    cv::resize(pixels,resized,{width,height},0,0,cv::INTER_LINEAR);
    Image result{width,height,std::vector<float>(size_t(width)*height*3)};
    for(int y=0;y<height;y++)for(int x=0;x<width;x++)for(int c=0;c<3;c++)
        result.rgb[(size_t(y)*width+x)*3+c]=resized.at<cv::Vec3b>(y,x)[c];
    return result;
}
Image normalize_polygon(const Image& image,const std::vector<Point>& polygon,int maximum_width,PolygonMapping* clipped_mapping) {
    if(polygon.size()<4 || polygon.size()%2 || maximum_width<2)throw std::invalid_argument("Invalid normalization polygon");
    size_t half=polygon.size()/2;
    float thickness=0;
    for(size_t i=0;i<half;i++) {
        float x=polygon[i].x-polygon[polygon.size()-1-i].x,y=polygon[i].y-polygon[polygon.size()-1-i].y;
        thickness+=std::sqrt(x*x+y*y);
    }
    thickness/=float(half);
    if(!(thickness>0))throw std::invalid_argument("Degenerate normalization polygon");
    std::vector<float> positions{0};
    float cumulative=0,scale=60.f/thickness;size_t active=half;
    for(size_t i=0;i<half-1;i++) {
        float x=polygon[i].x-polygon[i+1].x,y=polygon[i].y-polygon[i+1].y;
        cumulative=std::sqrt(x*x+y*y)*scale+cumulative;
        positions.push_back(std::min(cumulative,float(maximum_width-1)));
        if(cumulative>maximum_width-1){active=i+1;break;}
    }
    int width=std::max(1,int(positions.back()+1));
    if(clipped_mapping){*clipped_mapping={};clipped_mapping->width=width;}
    std::vector<cv::Point2f> source,target;
    for(auto p:polygon)source.emplace_back(p.x,p.y);
    // The original keeps the source polygon and zero-initialized target tail.
    // The capped endpoint controls image width but is not an active vertex.
    target.resize(polygon.size());
    for(size_t i=0;i<active;i++){target[i]={positions[i],0};target[polygon.size()-1-i]={positions[i],59};}
    cv::Mat input(image.height,image.width,CV_32FC3,const_cast<float*>(image.rgb.data()));
    cv::Mat output=cv::Mat::zeros(60,width,CV_32FC3);
    // 1800dc510 -> 1800defd0 -> 1801b1920. The two triangles in each
    // strip use separate affine transforms and a rasterized convex mask.
    for(size_t i=0;i<half-1;i++)for(int side=0;side<2;side++) {
        const size_t indices[2][3]={{i,i+1,polygon.size()-1-i},{polygon.size()-1-i,polygon.size()-2-i,i+1}};
        std::vector<cv::Point2f> src,dst;
        for(size_t index:indices[side]){src.push_back(source[index]);dst.push_back(target[index]);}
        if(clipped_mapping && active!=half)clipped_mapping->triangles.push_back({Point{dst[0].x,dst[0].y},Point{dst[1].x,dst[1].y},Point{dst[2].x,dst[2].y}});
        auto sr=cv::boundingRect(src),dr=cv::boundingRect(dst);
        sr.width=std::min(sr.width+1,image.width-sr.x);sr.height=std::min(sr.height+1,image.height-sr.y);
        dr.width=std::min(dr.width+1,width-dr.x);dr.height=std::min(dr.height+1,60-dr.y);
        if(sr.x<0 || sr.y<0 || sr.width<1 || sr.height<1 || dr.x<0 || dr.y<0 || dr.width<1 || dr.height<1)
            throw std::invalid_argument("Normalization triangle outside image");
        for(auto& p:src){p.x-=float(sr.x);p.y-=float(sr.y);}
        for(auto& p:dst){p.x-=float(dr.x);p.y-=float(dr.y);}
        auto affine=cv::getAffineTransform(src,dst);
        if(clipped_mapping && active!=half) {
            cv::Mat inverse;cv::invertAffineTransform(affine,inverse);
            clipped_mapping->inverse.push_back({float(inverse.at<double>(0,0)),float(inverse.at<double>(0,1)),
                float(inverse.at<double>(1,0)),float(inverse.at<double>(1,1)),float(affine.at<double>(0,2)),float(affine.at<double>(1,2)),
                float(sr.x),float(sr.y),float(dr.x),float(dr.y)});
        }
        cv::Mat patch;
        cv::warpAffine(input(sr),patch,affine,dr.size(),cv::INTER_LINEAR,cv::BORDER_REFLECT_101);
        cv::Mat mask=cv::Mat::zeros(dr.size(),CV_32FC3);
        std::vector<cv::Point> integer_dst;
        for(auto p:dst)integer_dst.emplace_back(int(p.x),int(p.y));
        cv::fillConvexPoly(mask,integer_dst,cv::Scalar::all(1),cv::LINE_AA);
        cv::multiply(output(dr),cv::Scalar::all(1)-mask,output(dr));
        cv::multiply(patch,mask,patch);
        cv::add(output(dr),patch,output(dr));
    }
    Image result{width,60,std::vector<float>(size_t(width)*60*3)};
    cv::Mat quantized;output.convertTo(quantized,CV_8UC3);
    for(int y=0;y<60;y++)for(int x=0;x<width;x++)for(int c=0;c<3;c++)
        result.rgb[(size_t(y)*width+x)*3+c]=quantized.at<cv::Vec3b>(y,x)[c];
    return result;
}
Quad map_clipped_polygon_word(const PolygonMapping& mapping,const std::vector<Point>& polygon,int start,int end,bool flipped) {
    if(mapping.triangles.empty() || mapping.triangles.size()!=mapping.inverse.size())throw std::invalid_argument("Missing clipped polygon mapping");
    Quad result{{{float(start),0},{float(end),0},{float(end),60},{float(start),60}}};
    Point minimum=polygon[0],maximum=polygon[0];
    for(auto p:polygon){minimum.x=std::min(minimum.x,p.x);minimum.y=std::min(minimum.y,p.y);maximum.x=std::max(maximum.x,p.x);maximum.y=std::max(maximum.y,p.y);}
    minimum={std::floor(minimum.x),std::floor(minimum.y)};maximum={std::ceil(maximum.x),std::ceil(maximum.y)};
    for(int k=0;k<4;k++) {
        auto& p=result[k];
        if(flipped){p.x=float(mapping.width-((k==0 || k==3)?1:0))-p.x;p.y=60-p.y;}
        size_t index=mapping.triangles.size()>1?mapping.triangles.size()-2:0;
        for(size_t i=0;i<mapping.triangles.size();i++)if(p.x<=mapping.triangles[i][1].x){index=i;break;}
        const auto& t=mapping.triangles[index];
        // Preserve the original NaN comparison for zero-width target triangles.
        if(!(p.y<=(t[2].y-t[0].y)*(t[1].x-p.x)/(t[1].x-t[0].x)))index++;
        if(index>=mapping.inverse.size())throw std::invalid_argument("Clipped polygon triangle index outside mapping");
        const auto& m=mapping.inverse[index];float x=(p.x-m[8])-m[4],y=(p.y-m[9])-m[5];
        p={std::clamp(y*m[1]+x*m[0]+m[6],minimum.x,maximum.x),std::clamp(y*m[3]+x*m[2]+m[7],minimum.y,maximum.y)};
    }
    return result;
}
Quad orient_line_box(const Quad& box,const Quad& crop) {
    Point direction{crop[1].x-crop[0].x,crop[1].y-crop[0].y};
    int best=0;float best_cos=-2;
    for(int i=0;i<4;i++) {
        Point edge{box[(i+1)%4].x-box[i].x,box[(i+1)%4].y-box[i].y};
        float length=std::sqrt(edge.x*edge.x+edge.y*edge.y);
        if(length>0) {
            float value=(edge.x*direction.x+edge.y*direction.y)/length;
            if(value>best_cos){best_cos=value;best=i;}
        }
    }
    return {box[best],box[(best+1)%4],box[(best+2)%4],box[(best+3)%4]};
}
}
