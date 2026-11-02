#include "oneocr_portable.h"
#include "image.hpp"
#include "sha256.hpp"
#include "json.hpp"
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <stdexcept>

using nlohmann::json;
using namespace oneocr_native;
namespace fs=std::filesystem;
namespace {
constexpr const char* capabilities=R"({"backend":"native-onnx-cpu","parity":"incomplete","word_confidence":"uncalibrated_ctc","style_confidence":"experimental_classifier_calibrated","geometry":"experimental_fpn_quadrilaterals"})";
Ort::Env& environment() { static Ort::Env env(ORT_LOGGING_LEVEL_ERROR,"oneocr-portable");return env; }
fs::path asset(const fs::path& root,const std::string& relative) {
    fs::path p=fs::u8path(relative);
    if(p.empty() || p.is_absolute()) throw std::invalid_argument("Model asset path must be relative");
    for(const auto& part:p) if(part=="..") throw std::invalid_argument("Model asset path escapes model directory");
    return root/p;
}
json read_json(const fs::path& path) {
    std::ifstream f(path);
    if(!f) throw std::runtime_error("Cannot open model manifest or vocabulary: "+path.string());
    json value; f>>value; return value;
}
std::unique_ptr<Ort::Session> session(const fs::path& path) {
    Ort::SessionOptions options;
    options.SetIntraOpNumThreads(1);
    options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    return std::make_unique<Ort::Session>(environment(),path.c_str(),options);
}
std::vector<Ort::Value> run_image(Ort::Session& s,const Image& image,bool bgr,float scale,const std::vector<const char*>& outputs) {
    auto data=chw(image,bgr,scale);
    std::array<int64_t,4> shape{1,3,image.height,image.width};
    auto memory=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
    auto value=Ort::Value::CreateTensor<float>(memory,data.data(),data.size(),shape.data(),shape.size());
    const char* name="data";
    return s.Run(Ort::RunOptions{nullptr},&name,&value,1,outputs.data(),outputs.size());
}
std::vector<float> quad_json(const Quad& q) {std::vector<float> values;for(auto p:q){values.push_back(p.x);values.push_back(p.y);}return values;}
Point interpolate(Point a,Point b,float t) {return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};}
Quad span(const Quad& q,float start,float end) {return {interpolate(q[0],q[1],start),interpolate(q[0],q[1],end),interpolate(q[3],q[2],end),interpolate(q[3],q[2],start)};}
struct Region {Quad box;float score;bool vertical;};
float overlap(const Quad& a,const Quad& b) {
    auto bounds=[](const Quad& q){std::array<float,4> r{1e30f,1e30f,-1e30f,-1e30f};for(auto p:q){r[0]=std::min(r[0],p.x);r[1]=std::min(r[1],p.y);r[2]=std::max(r[2],p.x);r[3]=std::max(r[3],p.y);}return r;};
    auto x=bounds(a),y=bounds(b);
    float intersection=std::max(0.f,std::min(x[2],y[2])-std::max(x[0],y[0]))*std::max(0.f,std::min(x[3],y[3])-std::max(x[1],y[1]));
    float aa=(x[2]-x[0])*(x[3]-x[1]),bb=(y[2]-y[0])*(y[3]-y[1]);
    return intersection/std::max(1.f,std::min(aa,bb));
}
oneocr_options resolve_options(const oneocr_options* options) {
    oneocr_options o=options?*options:oneocr_options{};
    if(o.max_lines==0) o.max_lines=1000;
    if(o.max_lines<1 || o.max_lines>1000) throw std::invalid_argument("max_lines must be 1..1000");
    if(o.resize_width==0 && o.resize_height==0) {o.resize_width=1152;o.resize_height=768;}
    if(o.resize_width<32 || o.resize_height<32 || o.resize_width>8192 || o.resize_height>8192) throw std::invalid_argument("Both resize dimensions must be 32..8192");
    return o;
}
char* copy_string(const std::string& s) {
    auto p=static_cast<char*>(std::malloc(s.size()+1));
    if(!p) throw std::bad_alloc();
    std::memcpy(p,s.c_str(),s.size()+1);return p;
}
void error_string(char** out,const char* message) noexcept {
    if(!out) return;
    size_t length=std::strlen(message);
    *out=static_cast<char*>(std::malloc(length+1));
    if(*out) std::memcpy(*out,message,length+1);
}
template<class F> int boundary(char** error,F&& f) noexcept {
    if(error) *error=nullptr;
    try {f();return ONEOCR_OK;}
    catch(const std::bad_alloc&) {error_string(error,"Memory allocation failed");return ONEOCR_ALLOCATION_ERROR;}
    catch(const std::invalid_argument& e) {error_string(error,e.what());return ONEOCR_INVALID_ARGUMENT;}
    catch(const ImageError& e) {error_string(error,e.what());return ONEOCR_IMAGE_ERROR;}
    catch(const Ort::Exception& e) {error_string(error,e.what());return ONEOCR_RUNTIME_ERROR;}
    catch(const std::exception& e) {error_string(error,e.what());return ONEOCR_MODEL_ERROR;}
    catch(...) {error_string(error,"Unknown native OCR error");return ONEOCR_RUNTIME_ERROR;}
}
}

struct oneocr_context {
    std::mutex mutex;
    fs::path root;
    std::unique_ptr<Ort::Session> detector,classifier;
    std::map<std::string,std::unique_ptr<Ort::Session>> recognizers;
    std::map<std::string,std::vector<std::string>> vocabularies;
    std::vector<std::array<float,2>> handwritten_calibration;
    explicit oneocr_context(const char* directory):root(fs::u8path(directory)) {
        if(std::string(OrtGetApiBase()->GetVersionString())!="1.23.2") throw std::runtime_error("ONNX Runtime v1.23.2 is required");
        auto manifest=read_json(root/"manifest.json");
        if(manifest.at("schema_version")!=1) throw std::runtime_error("Unsupported model manifest version");
        auto verify=[&](const json& record){auto file=asset(root,record.at("file").get<std::string>());if(hash_file(file)!=record.at("sha256").get<std::string>()) throw std::runtime_error("Model checksum mismatch: "+file.string());};
        for(const auto& record:manifest.at("models")) verify(record);
        verify(manifest.at("portable_detector"));
        for(const auto& record:manifest.at("vocabularies")) verify(record);
        verify(manifest.at("handwritten_calibration"));
        for(const auto& record:manifest.value("portable_auxiliary",json::array())) verify(record);
        handwritten_calibration=read_json(asset(root,manifest.at("handwritten_calibration").at("file").get<std::string>())).get<std::vector<std::array<float,2>>>();
        if(handwritten_calibration.empty()) throw std::runtime_error("Handwritten calibration table is empty");
        detector=session(asset(root,manifest.at("portable_detector").at("file").get<std::string>()));
        classifier=session(root/"classifier/script_classifier.onnx");
        const std::map<std::string,size_t> sizes{{"cjk",32632},{"cyrillic",415},{"latin",548},{"arabic",221},{"devanagari",237},{"greek",244},{"thai",199},{"hebrew",201},{"tamil",179}};
        for(const auto& [name,count]:sizes) {
            auto entries=read_json(root/"vocab"/("vocab_"+name+".json")).get<std::vector<std::string>>();
            if(entries.size()!=count || entries[0]!=" " || !entries.back().empty()) throw std::runtime_error("Invalid vocabulary contract: "+name);
            vocabularies.emplace(name,std::move(entries));
        }
    }
    std::vector<Region> detect(const Image& original,const oneocr_options& o) {
        int w=std::max(32,std::min(original.width,o.resize_width)/32*32);
        int h=std::max(32,std::min(original.height,o.resize_height)/32*32);
        auto image=resize(original,w,h);
        auto data=chw(image,false);
        float info[]={float(h),float(w),1};
        int64_t shape[]={1,3,h,w},info_shape[]={1,3};
        auto memory=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
        std::vector<Ort::Value> inputs;
        inputs.push_back(Ort::Value::CreateTensor<float>(memory,data.data(),data.size(),shape,4));
        inputs.push_back(Ort::Value::CreateTensor<float>(memory,info,3,info_shape,2));
        const char* input_names[]={"data","im_info"};
        std::vector<std::string> output_storage;
        for(int level=2;level<=4;level++) for(auto direction:{"hori","vert"}) for(auto kind:{"scores_","bbox_deltas_","link_scores_"}) output_storage.push_back(std::string(kind)+direction+"_fpn"+std::to_string(level));
        std::vector<const char*> names;for(auto& name:output_storage) names.push_back(name.c_str());
        auto outputs=detector->Run(Ort::RunOptions{nullptr},input_names,inputs.data(),2,names.data(),names.size());
        std::vector<Region> regions;
        constexpr int dx[]={-1,0,1,1,1,0,-1,-1},dy[]={-1,-1,-1,0,1,1,1,0};
        for(int level=0;level<3;level++) for(int direction=0;direction<2;direction++) {
            int base=level*6+direction*3;
            auto dims=outputs[base].GetTensorTypeAndShapeInfo().GetShape();
            int mh=int(dims[2]),mw=int(dims[3]);size_t n=size_t(mh)*mw;
            const float* scores=outputs[base].GetTensorData<float>();
            const float* deltas=outputs[base+1].GetTensorData<float>();
            const float* links=outputs[base+2].GetTensorData<float>();
            std::vector<uint8_t> visited(n,0);
            for(size_t seed=0;seed<n;seed++) {
                if(visited[seed] || scores[seed]<.7f) continue;
                std::vector<size_t> group{seed};visited[seed]=1;
                for(size_t next=0;next<group.size();next++) {
                    size_t cell=group[next];int x=int(cell%mw),y=int(cell/mw);
                    for(int k=0;k<8;k++) {
                        int nx=x+dx[k],ny=y+dy[k];
                        if(nx<0 || ny<0 || nx>=mw || ny>=mh) continue;
                        size_t other=size_t(ny)*mw+nx;
                        if(!visited[other] && scores[other]>=.7f && links[size_t(k)*n+cell]>.8f) {visited[other]=1;group.push_back(other);}
                    }
                }
                if(group.size()<3) continue;
                std::vector<Point> points;float score=0;
                float stride=float(4<<level),anchor=float(24<<level);
                for(auto cell:group) {
                    score+=scores[cell];
                    float x=float(cell%mw)*stride,y=float(cell/mw)*stride;
                    for(int k=0;k<4;k++) points.push_back({(x+deltas[size_t(k*2)*n+cell]*anchor)*original.width/w,(y+deltas[size_t(k*2+1)*n+cell]*anchor)*original.height/h});
                }
                auto box=minimum_rectangle(std::move(points));
                if(quad_area(box)<16 || score/group.size()<(level==0?.7f:.8f)) continue;
                for(auto& p:box) {p.x=std::clamp(p.x,0.f,float(original.width-1));p.y=std::clamp(p.y,0.f,float(original.height-1));}
                regions.push_back({box,score/float(group.size()),direction==1});
            }
        }
        std::stable_sort(regions.begin(),regions.end(),[](const Region& a,const Region& b){return a.score>b.score;});
        std::vector<Region> kept;
        for(const auto& r:regions) if(std::none_of(kept.begin(),kept.end(),[&](const Region& k){return overlap(r.box,k.box)>.65f;})) kept.push_back(r);
        std::stable_sort(kept.begin(),kept.end(),[](const Region& a,const Region& b){
            float ay=0,by=0;for(auto p:a.box)ay+=p.y;for(auto p:b.box)by+=p.y;
            return ay<by || (ay==by && a.box[0].x<b.box[0].x);
        });
        return kept;
    }
    json recognize(const Image& image,const oneocr_options& options,const std::string& file,int64_t step) {
        auto regions=detect(image,options);
        json lines=json::array();std::vector<float> angles;
        constexpr const char* scripts[]={"latin","cjk","latin","cyrillic","arabic","devanagari","greek","thai","hebrew","tamil"};
        for(const auto& region:regions) {
            if(lines.size()>=size_t(options.max_lines)) break;
            auto q=region.box;
            if(region.vertical) q={q[1],q[2],q[3],q[0]};
            float width=(distance(q[0],q[1])+distance(q[3],q[2]))*.5f;
            float height=(distance(q[0],q[3])+distance(q[1],q[2]))*.5f;
            if(width<2 || height<2) continue;
            int target_width=std::clamp(int(width*60/height),16,16384);
            auto normalized=warp_quad(image,q,target_width,60);
            for(auto& value:normalized.rgb) value=std::round(value);
            auto cls=run_image(*classifier,normalized,true,1.f/255,{"script_id_score","hwpc_score","flip_score"});
            const float* scores=cls[0].GetTensorData<float>();
            int script=int(std::max_element(scores,scores+10)-scores);
            if(script==0) continue; // Non-text class; verified by native probes.
            float hwpc=cls[1].GetTensorData<float>()[0];
            float flip=cls[2].GetTensorData<float>()[0];
            if(flip<0) {normalized=rotate_quarter(normalized,2);q={q[2],q[3],q[0],q[1]};}
            std::string language=scripts[script];
            int frame_stride=(language=="cjk" || language=="cyrillic")?8:4;
            int aligned_width=(target_width+frame_stride-1)/frame_stride*frame_stride;
            if(aligned_width!=target_width) {
                normalized=resize(normalized,aligned_width,60);
                for(auto& value:normalized.rgb) value=std::round(value);
                target_width=aligned_width;
            }
            if(!recognizers.count(language)) recognizers[language]=session(root/"recognizers"/("recognizer_"+language+".onnx"));
            auto input=chw(normalized,true,1.f/255);
            int32_t length=target_width/frame_stride;
            int64_t shape[]={1,3,60,target_width},length_shape[]={1};
            auto memory=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
            std::vector<Ort::Value> tensors;
            tensors.push_back(Ort::Value::CreateTensor<float>(memory,input.data(),input.size(),shape,4));
            tensors.push_back(Ort::Value::CreateTensor<int32_t>(memory,&length,1,length_shape,1));
            const char* names[]={"data","seq_lengths"};const char* out="logsoftmax";
            auto result=recognizers[language]->Run(Ort::RunOptions{nullptr},names,tensors.data(),2,&out,1);
            auto dims=result[0].GetTensorTypeAndShapeInfo().GetShape();
            int steps=int(dims.at(0)),v=int(dims.at(2));
            const auto& vocab=vocabularies.at(language);
            if(dims.at(1)!=1 || size_t(v)!=vocab.size()) throw std::runtime_error("Recognizer/vocabulary mismatch");
            const float* logits=result[0].GetTensorData<float>();
            struct Token {std::string text;int start,end;float confidence;};
            std::vector<Token> tokens;int previous=-1;
            for(int t=0;t<steps;t++) {
                const float* row=logits+size_t(t)*v;
                int index=int(std::max_element(row,row+v)-row);
                if(index!=previous && index!=v-1 && !vocab[index].empty()) tokens.push_back({vocab[index],t,t+1,std::exp(row[index])});
                else if(index==previous && index!=v-1 && !tokens.empty()) tokens.back().end=t+1;
                previous=index;
            }
            json words=json::array();std::string text,current;float sum=0;int count=0,start=0,end=0;
            auto emit=[&](){if(current.empty())return;auto box=span(q,float(start)/steps,float(end)/steps);words.push_back({{"index",words.size()},{"text",current},{"bounding_box",quad_json(box)},{"confidence",sum/count}});current.clear();sum=0;count=0;};
            for(const auto& token:tokens) {
                if(token.text==" ") {emit();if(!text.empty() && text.back()!=' ') text+=' ';continue;}
                bool individual=language=="cjk" && (static_cast<unsigned char>(token.text[0])>=0xE0 || (token.text.size()==1 && std::strchr(":;!?()[]{}",token.text[0])));
                if(individual) emit();
                if(current.empty()) start=token.start;
                current+=token.text;text+=token.text;end=token.end;sum+=token.confidence;count++;
                if(individual) emit();
            }
            emit();
            while(!text.empty() && text.back()==' ') text.pop_back();
            if(text.empty()) continue;
            float confidence=1.f/(1.f+std::exp(-hwpc));
            auto calibration=std::lower_bound(handwritten_calibration.begin(),handwritten_calibration.end(),confidence,[](const auto& pair,float value){return pair[0]<value;});
            float calibrated=calibration==handwritten_calibration.end()?handwritten_calibration.back()[1]:(*calibration)[1];
            lines.push_back({{"index",lines.size()},{"text",text},{"bounding_box",quad_json(q)},{"style",{{"type",confidence>.5f?"handwritten":"printed"},{"confidence",calibrated}}},{"word_count",words.size()},{"words",words}});
            float angle=std::atan2(q[1].y-q[0].y,q[1].x-q[0].x)*180.f/3.14159265358979323846f;
            angles.push_back(angle);
        }
        float angle=0;
        if(!angles.empty()) {std::sort(angles.begin(),angles.end());angle=angles[angles.size()/2];}
        angle=std::fmod(angle+360.f,360.f);
        return {{"file",file},{"image",{{"width",image.width},{"height",image.height},{"step",step}}},{"image_angle",angle},{"line_count",lines.size()},{"lines",lines},{"engine",json::parse(capabilities)}};
    }
};

extern "C" {
int oneocr_create(const char* models,oneocr_context** context,char** error) {
    if(context) *context=nullptr;
    return boundary(error,[&](){if(!models || !*models || !context) throw std::invalid_argument("Model directory and context output are required");auto p=std::make_unique<oneocr_context>(models);*context=p.release();});
}
int oneocr_recognize_file(oneocr_context* context,const char* path,const oneocr_options* options,char** output,char** error) {
    if(output) *output=nullptr;
    return boundary(error,[&](){if(!context || !path || !*path || !output) throw std::invalid_argument("Context, image path and JSON output are required");auto o=resolve_options(options);auto image=load_image(path);std::lock_guard lock(context->mutex);*output=copy_string(context->recognize(image,o,path,int64_t(image.width)*4).dump(2));});
}
int oneocr_recognize_bgra(oneocr_context* context,const uint8_t* data,size_t bytes,int32_t w,int32_t h,int64_t stride,const oneocr_options* options,char** output,char** error) {
    if(output) *output=nullptr;
    return boundary(error,[&](){if(!context || !output) throw std::invalid_argument("Context and JSON output are required");auto o=resolve_options(options);auto image=from_bgra(data,bytes,w,h,stride);std::lock_guard lock(context->mutex);*output=copy_string(context->recognize(image,o,"",stride).dump(2));});
}
void oneocr_destroy(oneocr_context* context) {delete context;}
void oneocr_free(void* p) {std::free(p);}
const char* oneocr_capabilities(void) {return capabilities;}
}
