#include "oneocr_portable.h"
#include "image.hpp"
#include "sha256.hpp"
#include "trace.hpp"
#include "detector.hpp"
#include "ordering.hpp"
#include "normalizer.hpp"
#include "alignment.hpp"
#include "confidence.hpp"
#include "unicode.hpp"
#include "nms.hpp"
#include "classification.hpp"
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
#ifdef __EMSCRIPTEN__
extern "C" const char* oneocr_wasm_numeric_profile();
#endif

using nlohmann::json;
using namespace oneocr_native;
namespace fs=std::filesystem;
namespace {
constexpr const char* capabilities=
#ifdef __EMSCRIPTEN__
R"({"backend":"wasm-onnx-cpu","numeric_profile":"oneocr-wasm-fma-nchwc16-v2",)"
#else
R"({"backend":"native-onnx-cpu",)"
#endif
R"("parity":"incomplete","word_confidence":"native_21_features_mlp","style_confidence":"native_classifier_calibrated","geometry":"native_curve_polygons"})";
class ImageSizeError : public std::runtime_error {public:using std::runtime_error::runtime_error;};
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
struct Region {Quad box;float score;bool vertical;Quad line_box;LineCurve curve;std::vector<Point> polygon;std::array<float,4> bounds;int level;};
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
    catch(const ImageSizeError& e) {error_string(error,e.what());return ONEOCR_IMAGE_SIZE_ERROR;}
    catch(const std::invalid_argument& e) {error_string(error,e.what());return ONEOCR_INVALID_ARGUMENT;}
    catch(const UnsupportedImageError& e) {error_string(error,e.what());return ONEOCR_UNSUPPORTED_IMAGE;}
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
    std::map<std::string,CharacterInfo> character_info;
    std::map<std::string,fs::path> auxiliary_files;
    std::map<std::string,std::unique_ptr<Ort::Session>> auxiliary_sessions;
    json character_settings,rejection_settings,layout_settings;
    ClassificationSettings classifier_settings;
    fs::path layout_file;
    std::unique_ptr<Ort::Session> layout_session;
    std::vector<std::array<float,2>> handwritten_calibration;
    explicit oneocr_context(const char* directory):root(fs::u8path(directory)) {
#ifdef __EMSCRIPTEN__
        if(std::string(oneocr_wasm_numeric_profile())!="oneocr-wasm-fma-nchwc16-v2")throw std::runtime_error("Wasm numerical runtime profile mismatch");
#endif
        if(std::string(OrtGetApiBase()->GetVersionString())!="1.23.2") throw std::runtime_error("ONNX Runtime v1.23.2 is required");
        auto manifest=read_json(root/"manifest.json");
        if(manifest.at("schema_version")!=1) throw std::runtime_error("Unsupported model manifest version");
        auto verify=[&](const json& record){auto file=asset(root,record.at("file").get<std::string>());if(hash_file(file)!=record.at("sha256").get<std::string>()) throw std::runtime_error("Model checksum mismatch: "+file.string());};
        for(const auto& record:manifest.at("models")) verify(record);
        verify(manifest.at("portable_detector"));
        for(const auto& record:manifest.at("vocabularies")) verify(record);
        for(const auto& record:manifest.at("character_metadata"))verify(record);
        character_settings=manifest.at("character_settings");rejection_settings=manifest.at("rejection_settings");
        layout_settings=manifest.at("layout_settings");
        const auto& cls_settings=manifest.at("classifier_settings");
        classifier_settings.script_margin=cls_settings.at("script_margin");classifier_settings.nontext_margin=cls_settings.at("nontext_margin");
        classifier_settings.handwriting_min=cls_settings.at("handwriting_min");classifier_settings.handwriting_max=cls_settings.at("handwriting_max");
        classifier_settings.flip_min=cls_settings.at("flip_min");classifier_settings.flip_max=cls_settings.at("flip_max");
        classifier_settings.handwriting_consecutive=cls_settings.at("handwriting_consecutive");
        const std::map<std::string,int> script_ids{{"CJK",2},{"Latin",3},{"Cyrillic",4},{"Arabic",5},{"Devanagari",6},{"Greek",7},{"Thai",8},{"Hebrew",9},{"Tamil",10}};
        classifier_settings.script_consecutive.fill(-1);
        for(auto [name,id]:script_ids)if(cls_settings.at("script_consecutive").contains(name))classifier_settings.script_consecutive[id]=cls_settings.at("script_consecutive").at(name);
        for(const auto& converted:manifest.at("portable_auxiliary"))if(converted.at("source")=="layout/line_layout.onnx")layout_file=asset(root,converted.at("file").get<std::string>());
        if(layout_file.empty())throw std::runtime_error("Missing original layout model conversion");
        verify(manifest.at("handwritten_calibration"));
        for(const auto& record:manifest.value("portable_auxiliary",json::array())) verify(record);
        for(const auto& original:manifest.at("models")) {
            std::string role=original.at("role");if(role!="confidence" && role!="rejection")continue;
            std::string resource=original.at("source_resource");auto first=resource.find('\\'),last=resource.find('\\',first+1);
            if(first==std::string::npos || last==std::string::npos)throw std::runtime_error("Invalid auxiliary model resource identity");
            std::string route=resource.substr(first+1,last-first-1);
            auto converted=std::find_if(manifest.at("portable_auxiliary").begin(),manifest.at("portable_auxiliary").end(),[&](const auto& item){return item.at("source")==original.at("file");});
            if(converted==manifest.at("portable_auxiliary").end())throw std::runtime_error("Missing converted auxiliary model: "+route);
            auxiliary_files.emplace(role+"/"+route,asset(root,converted->at("file").get<std::string>()));
        }
        handwritten_calibration=read_json(asset(root,manifest.at("handwritten_calibration").at("file").get<std::string>())).get<std::vector<std::array<float,2>>>();
        if(handwritten_calibration.empty()) throw std::runtime_error("Handwritten calibration table is empty");
        detector=session(asset(root,manifest.at("portable_detector").at("file").get<std::string>()));
        classifier=session(root/"classifier/script_classifier.onnx");
        const std::map<std::string,size_t> sizes{{"cjk",32632},{"cyrillic",415},{"latin",548},{"arabic",221},{"devanagari",237},{"greek",244},{"thai",199},{"hebrew",201},{"tamil",179}};
        for(const auto& [name,count]:sizes) {
            auto entries=read_json(root/"vocab"/("vocab_"+name+".json")).get<std::vector<std::string>>();
            if(entries.size()!=count || entries[0]!=" " || !entries.back().empty()) throw std::runtime_error("Invalid vocabulary contract: "+name);
            vocabularies.emplace(name,std::move(entries));
            auto metadata=read_json(root/"vocab"/("metadata_"+name+".json"));
            CharacterInfo info;
            info.tokens=metadata.at("tokens").get<std::vector<std::string>>();
            info.logical_indices=metadata.at("logical_indices").get<std::vector<int>>();
            info.types=metadata.at("character_types").get<std::vector<int>>();
            if(info.tokens.size()!=count || info.logical_indices.size()!=count || info.types.size()!=count)throw std::runtime_error("Incomplete character metadata: "+name);
            for(size_t i=0;i<count;i++) {
                if(info.logical_indices[i]<0 || size_t(info.logical_indices[i])>=count)throw std::runtime_error("Invalid logical character index");
                if(info.types[i]==4)info.blank=int(i);if(info.types[i]==5)info.space=int(i);if(info.types[i]==8)info.trash=int(i);
            }
            if(info.blank<0 || info.space<0)throw std::runtime_error("Missing CTC special character metadata");
            character_info.emplace(name,std::move(info));
        }
    }
    float auxiliary_probability(const std::string& key,const std::array<float,21>& features) {
        if(!auxiliary_sessions.count(key))auxiliary_sessions[key]=session(auxiliary_files.at(key));
        int64_t shape[]={1,21,1,1};auto memory=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
        auto input=Ort::Value::CreateTensor<float>(memory,const_cast<float*>(features.data()),features.size(),shape,4);
        const char* name="data";const char* output="softmax";
        auto result=auxiliary_sessions.at(key)->Run(Ort::RunOptions{nullptr},&name,&input,1,&output,1);
        if(result[0].GetTensorTypeAndShapeInfo().GetElementCount()!=2)throw std::runtime_error("Auxiliary model output contract mismatch");
        return result[0].GetTensorData<float>()[1];
    }
    std::vector<Region> detect(const Image& original,const oneocr_options& o) {
        TensorTrace trace("detector");
        auto size=detector_dimensions(original.width,original.height,o.resize_width,o.resize_height);
        int w=size[0],h=size[1];
        auto image=resize_detector_pixels(original,w,h);
        auto data=chw(image,false);
        float info[]={float(h),float(w),1};
        trace.tensor("data",data.data(),data.size(),{1,3,h,w});
        trace.tensor("im_info",info,3,{1,3});
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
        for(size_t i=0;i<outputs.size();i++) {
            auto type=outputs[i].GetTensorTypeAndShapeInfo();
            trace.tensor(output_storage[i],outputs[i].GetTensorData<float>(),type.GetElementCount(),type.GetShape());
        }
        std::vector<Region> regions;
        json fragment_trace=json::array();
        for(int level=0;level<3;level++) {
          size_t level_start=regions.size();int32_t horizontal_rows=0;std::vector<int32_t> group_keys;
          for(int direction=0;direction<2;direction++) {
            int base=level*6+direction*3;
            auto dims=outputs[base].GetTensorTypeAndShapeInfo().GetShape();
            int mh=int(dims[2]),mw=int(dims[3]);
            const float* scores=outputs[base].GetTensorData<float>();
            const float* deltas=outputs[base+1].GetTensorData<float>();
            const float* links=outputs[base+2].GetTensorData<float>();
            auto groups=detector_segments(scores,deltas,links,mw,mh,level+2,direction==0);
            int32_t group_key=direction?horizontal_rows:0;
            for(const auto& group:groups) {
                if(direction==0)horizontal_rows+=int32_t(group.segments.size());
                std::vector<Point> points;float score=0;
                json fragments=json::array();
                for(const auto& segment:group.segments) {
                    score+=segment.score;
                    if(trace.enabled())fragments.push_back({{"cell",segment.cell},{"score",segment.score},{"box",quad_json(segment.box)}});
                    for(auto point:segment.box)points.push_back({point.x*original.width/w,point.y*original.height/h});
                }
                if(trace.enabled())fragment_trace.push_back({{"level",level+2},{"vertical",direction==1},{"root",group.root},{"segments",fragments}});
                auto curve=fit_line_curve(group.segments,direction==0,float(w)/original.width,float(h)/original.height);
                if(trace.enabled()) {
                    fragment_trace.back()["curve"]={{"upper",curve.upper},{"lower",curve.lower},{"corners",quad_json(curve.corners)},{"score",curve.score},{"polygon_box",quad_json(minimum_rectangle(curve_polygon(curve)))}};
                }
                auto polygon=curve_polygon(curve);
                for(auto& point:polygon) {point.x=std::clamp(point.x,0.f,float(original.width-1));point.y=std::clamp(point.y,0.f,float(original.height-1));}
                auto box=line_polygon_box(polygon,polygon.size()>4);
                for(auto& p:box) {p.x=std::clamp(p.x,0.f,float(original.width-1));p.y=std::clamp(p.y,0.f,float(original.height-1));}
                auto line_box=box;
                for(auto& point:line_box){point.x=float(int(point.x));point.y=float(int(point.y));}
                std::array<float,4> bounds{polygon[0].x,polygon[0].y,polygon[0].x,polygon[0].y};
                for(auto p:polygon){bounds[0]=std::min(bounds[0],p.x);bounds[1]=std::min(bounds[1],p.y);bounds[2]=std::max(bounds[2],p.x);bounds[3]=std::max(bounds[3],p.y);}
                for(int k=0;k<4;k++)bounds[k]=k<2?std::floor(bounds[k]):std::ceil(bounds[k]);
                regions.push_back({curve.corners,curve.score,direction==1,line_box,curve,polygon,bounds,level+2});
                group_keys.push_back(group_key++);
            }
          }
          // FB1F0 -> 102A90 (group merging disabled): two MSVC hash-map
          // passes. Vertical IDs start at the horizontal proposal ROW count.
          auto groups_order=native_group_order(native_group_order(group_keys));std::map<int32_t,size_t> group_index;
          for(size_t i=0;i<group_keys.size();i++)group_index[group_keys[i]]=i;
          std::vector<Region> ordered_level;for(int32_t key:groups_order)ordered_level.push_back(std::move(regions[level_start+group_index.at(key)]));
          for(size_t i=0;i<ordered_level.size();i++)regions[level_start+i]=std::move(ordered_level[i]);
        }
        trace.document("segments",fragment_trace);
        // The two hash passes include every group. 1800e61f0 -> 1800fbd50
        // then rejects excessive curvature before the global score sort.
        regions.erase(std::remove_if(regions.begin(),regions.end(),[](const Region& region){return std::max(std::abs(region.curve.upper[2]),std::abs(region.curve.lower[2]))>.05f;}),regions.end());
        auto score_order=native_sort_order(regions.size(),[&](size_t a,size_t b){return regions[a].score>regions[b].score;});
        std::vector<Region> sorted_regions;for(size_t index:score_order)sorted_regions.push_back(std::move(regions[index]));regions=std::move(sorted_regions);
        // Native E00F0 removes low scores and invalid headings AFTER the
        // unstable score sort. Removing them earlier changes exact tie order.
        std::vector<Region> eligible;
        for(auto& region:regions) {
            Quad heading=region.curve.corners;if(region.vertical)heading={heading[1],heading[2],heading[3],heading[0]};
            float heading_angle=line_quad_angle(heading);while(heading_angle>90)heading_angle-=180;heading_angle=std::abs(heading_angle);
            if((!region.vertical && heading_angle>=50) || (region.vertical && heading_angle<=40))continue;
            if(quad_area(region.line_box)<16 || region.score<(region.level==2?.7f:.8f))continue;
            eligible.push_back(std::move(region));
        }
        regions=std::move(eligible);
        auto trace_regions=[&](const char* name,const std::vector<Region>& values) {
            if(!trace.enabled())return;
            json document=json::array();
            for(const auto& region:values){json points=json::array();for(auto p:region.polygon){points.push_back(p.x);points.push_back(p.y);}document.push_back({{"box",quad_json(region.box)},{"score",region.score},{"vertical",region.vertical},{"polygon",points},{"level",region.level}});}
            trace.document(name,document);
        };
        trace_regions("eligible",regions);
        std::vector<Region> kept;
        // 180103250 selects the active ambiguity configuration (.3, .4, .5,
        // .4), overriding the ordinary detector NMS threshold of .2.
        for(const auto& r:regions) if(std::none_of(kept.begin(),kept.end(),[&](const Region& k){
            if(r.vertical!=k.vertical || r.bounds[0]>k.bounds[2] || k.bounds[0]>r.bounds[2] || r.bounds[1]>k.bounds[3] || k.bounds[1]>r.bounds[3])return false;
            float intersection=line_polygon_intersection(r.polygon,k.polygon);
            return intersection/(line_polygon_area(r.polygon)+line_polygon_area(k.polygon)-intersection)>.3f;
        })) kept.push_back(r);
        trace_regions("primary",kept);
        std::vector<bool> suppressed(kept.size(),false);
        std::vector<std::vector<Point>> polygons;for(const auto& r:kept)polygons.push_back(r.polygon);
        for(size_t i=0;i<kept.size();i++)if(!suppressed[i])for(size_t j=0;j<kept.size();j++)if(i!=j && !suppressed[j]) {
            if(!line_polygon_covered(polygons[i],!kept[i].vertical,polygons[j],!kept[j].vertical))continue;
            if(kept[i].score<=kept[j].score || !line_polygon_covered(polygons[j],!kept[j].vertical,polygons[i],!kept[i].vertical)){suppressed[i]=true;break;}
            suppressed[j]=true;
        }
        std::vector<Region> secondary;for(size_t i=0;i<kept.size();i++)if(!suppressed[i])secondary.push_back(kept[i]);kept=std::move(secondary);
        trace_regions("secondary",kept);
        std::vector<Region> joined;std::vector<bool> merged(kept.size(),false);
        for(size_t i=0;i<kept.size();i++)if(!merged[i]) {
            auto current=kept[i];
            for(size_t j=i+1;j<kept.size();j++)if(!merged[j] && line_polygons_joinable(current.polygon,!current.vertical,kept[j].polygon,!kept[j].vertical)) {
                current.score=(current.score+kept[j].score)*.5f;
                current.curve=refit_line_polygons(current.polygon,kept[j].polygon,!current.vertical,current.score);
                for(int k=0;k<4;k++)current.bounds[k]=k<2?std::min(current.bounds[k],kept[j].bounds[k]):std::max(current.bounds[k],kept[j].bounds[k]);
                auto [minx,miny,maxx,maxy]=current.bounds;
                current.polygon=curve_polygon(current.curve);
                for(auto& point:current.polygon){point.x=std::clamp(point.x,minx,maxx);point.y=std::clamp(point.y,miny,maxy);}
                size_t half=current.polygon.size()/2;
                current.box={current.polygon[0],current.polygon[half-1],current.polygon[half],current.polygon.back()};
                if(current.vertical)current.box={current.box[3],current.box[0],current.box[1],current.box[2]};
                current.line_box=line_polygon_box(current.polygon,current.polygon.size()>4);
                merged[j]=true;j=i;
            }
            joined.push_back(std::move(current));
        }
        kept=std::move(joined);
        json proposals=json::array();
        if(trace.enabled())for(const auto& region:kept){json points=json::array();for(auto p:region.polygon){points.push_back(p.x);points.push_back(p.y);}proposals.push_back({{"box",quad_json(region.box)},{"score",region.score},{"vertical",region.vertical},{"polygon",points}});}
        trace.document("regions",proposals);
        return kept;
    }
    json recognize(const Image& image,const oneocr_options& options,const std::string& file,int64_t step) {
        // RunOcrPipeline 180029960 checks unsigned(dimension-50)<=9950.
        if(image.width<50 || image.height<50 || image.width>10000 || image.height>10000)throw ImageSizeError("Image dimensions must be 50..10000 pixels");
        auto regions=detect(image,options);
        json lines=json::array();std::vector<float> angles;std::vector<Quad> line_boxes;size_t rtl_lines=0;
        std::vector<RecognitionCandidate> recognition_candidates;
        constexpr const char* scripts[]={"latin","cjk","latin","cyrillic","arabic","devanagari","greek","thai","hebrew","tamil"};
        struct PreparedLine {
            Region region;Quad q,line_box;std::vector<Point> polygon;Image normalized;
            QuadNormalization quad_normalized;PolygonMapping clipped_mapping;TensorTrace trace;
        };
        std::vector<PreparedLine> prepared;
        std::vector<LineClassification> classifications;
        constexpr const char* cls_outputs[]={"script_id_score","hwpc_score","flip_score","script_id_perframe_predict","hwpc_perframe_predict"};
        for(const auto& region:regions) {
            TensorTrace line_trace("line");
            auto q=region.box;
            if(region.vertical)q={q[1],q[2],q[3],q[0]};
            auto line_box=orient_line_box(region.line_box,q);
            float width=(distance(q[0],q[1])+distance(q[3],q[2]))*.5f;
            float height=(distance(q[0],q[3])+distance(q[1],q[2]))*.5f;
            if(width<2 || height<2) continue;
            auto polygon=region.polygon;
            for(auto& p:polygon){p.x=std::clamp(p.x,0.f,float(image.width-1));p.y=std::clamp(p.y,0.f,float(image.height-1));}
            int maximum_width=int(std::min(1000000.,std::sqrt(double(options.resize_width)*options.resize_width+double(options.resize_height)*options.resize_height)*210+1));
            QuadNormalization quad_normalized;
            PolygonMapping clipped_mapping;
            auto normalized=polygon.size()==4?(quad_normalized=normalize_quad(image,polygon,maximum_width)).image:normalize_polygon(image,polygon,maximum_width,&clipped_mapping);
            auto cls=run_image(*classifier,normalized,false,1.f/255,{std::begin(cls_outputs),std::end(cls_outputs)});
            if(line_trace.enabled()) {
                auto pixels=chw(normalized,false,1.f/255);
                line_trace.tensor("classifier_input",pixels.data(),pixels.size(),{1,3,normalized.height,normalized.width});
                for(size_t i=0;i<cls.size();i++) {
                    auto type=cls[i].GetTensorTypeAndShapeInfo();
                    line_trace.tensor(cls_outputs[i],cls[i].GetTensorData<float>(),type.GetElementCount(),type.GetShape());
                }
                line_trace.document("region",{{"crop",quad_json(q)},{"line_box",quad_json(region.line_box)},{"vertical",region.vertical}});
            }
            auto frame_values=[&](size_t index){const auto* begin=cls[index].GetTensorData<float>();return std::vector<float>(begin,begin+cls[index].GetTensorTypeAndShapeInfo().GetElementCount());};
            classifications.push_back(classify_line(cls[0].GetTensorData<float>(),frame_values(3),cls[1].GetTensorData<float>()[0],frame_values(4),cls[2].GetTensorData<float>()[0],!region.vertical,classifier_settings));
            prepared.push_back({region,q,line_box,std::move(polygon),std::move(normalized),std::move(quad_normalized),std::move(clipped_mapping),std::move(line_trace)});
        }
        resolve_classifications(classifications);
        for(size_t region_index=0;region_index<prepared.size();region_index++) {
            if(lines.size()>=size_t(options.max_lines))break;
            auto& classification=classifications[region_index];
            if(classification.script==11)continue;
            auto& item=prepared[region_index];auto& line_trace=item.trace;
            auto q=item.q,line_box=item.line_box;auto& polygon=item.polygon;
            auto normalized=std::move(item.normalized);auto& quad_normalized=item.quad_normalized;
            int target_width=normalized.width;
            int script=classification.script-1;
            bool flipped=classification.flip==2;
            line_trace.document("classification",{{"script",classification.script},{"style",classification.style},{"flip",classification.flip},{"script_ambiguous",classification.script_ambiguous},{"flip_ambiguous",classification.flip_ambiguous},{"handwriting_probability",classification.handwriting_probability}});
            int layout=1;
            if(script==1) {
                if(!layout_session)layout_session=session(layout_file);
                auto output=run_image(*layout_session,normalized,false,1.f/255,{"line_layout_score"});
                float value=output[0].GetTensorData<float>()[0];
                layout=double(value)<=layout_settings.at("vertical_threshold").get<double>()?2:double(value)<=layout_settings.at("horizontal_threshold").get<double>()?0:1;
                line_trace.document("layout",{{"score",value},{"layout",layout}});
            }
            if(flipped) {normalized=rotate_quarter(normalized,2);q={q[2],q[3],q[0],q[1]};line_box={line_box[2],line_box[3],line_box[0],line_box[1]};}
            auto blank_intervals=line_blank_intervals(normalized);
            std::string language=scripts[script];
            int frame_stride=(language=="cjk" || language=="cyrillic")?8:4;
            int aligned_width=(target_width/frame_stride+1)*frame_stride;
            if(aligned_width!=target_width) {
                normalized=resize_line_pixels(normalized,aligned_width,60);
                target_width=aligned_width;
            }
            if(!recognizers.count(language)) recognizers[language]=session(root/"recognizers"/("recognizer_"+language+".onnx"));
            auto input=chw(normalized,false,1.f/255);
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
            line_trace.tensor("recognizer_input",input.data(),input.size(),{1,3,60,target_width});
            line_trace.tensor("logsoftmax",logits,result[0].GetTensorTypeAndShapeInfo().GetElementCount(),dims);
            struct Token {std::string text;int start,end;float confidence;int id;};
            std::vector<Token> tokens;int previous=-1;
            for(int t=0;t<steps;t++) {
                const float* row=logits+size_t(t)*v;
                int index=int(std::max_element(row,row+v)-row);
                if(index!=previous && index!=v-1 && !vocab[index].empty()) tokens.push_back({vocab[index],t,t+1,std::exp(row[index]),index});
                else if(index==previous && index!=v-1 && !tokens.empty()) tokens.back().end=t+1;
                previous=index;
            }
            json words=json::array();std::string text,current;float sum=0;int count=0,start=0,end=0;
            std::vector<WordFrames> word_frames;
            std::vector<std::vector<DecodedCharacter>> word_characters,coarse_words;
            std::vector<DecodedCharacter> current_characters,coarse;
            auto thai_boundaries=language=="thai"?thai_word_boundaries([&](){std::string value;for(const auto& token:tokens)value+=token.text;return value;}()):std::vector<int>{};
            int utf16_offset=0;
            auto emit=[&](){if(current.empty())return;word_frames.push_back({start,end-1});word_characters.push_back(std::move(current_characters));current_characters.clear();words.push_back({{"index",words.size()},{"text",current},{"confidence",sum/count}});current.clear();sum=0;count=0;};
            int punctuation_run=0;
            auto split_punctuation=[&](){emit();if(!coarse.empty()){coarse_words.push_back(std::move(coarse));coarse.clear();}if(!text.empty() && text.back()!=' ')text+=' ';};
            for(size_t token_index=0;token_index<tokens.size();token_index++) {
                const auto& token=tokens[token_index];
                if(std::binary_search(thai_boundaries.begin(),thai_boundaries.end(),utf16_offset))emit();
                utf16_offset+=utf16_length(token.text);
                if(token.text==" ") {emit();if(!coarse.empty()){coarse_words.push_back(std::move(coarse));coarse.clear();}if(!text.empty() && text.back()!=' ') text+=' ';punctuation_run=0;continue;}
                bool punctuation=character_info.at(language).types.at(token.id)==7;
                if(punctuation) {
                    if(!punctuation_run && !coarse.empty() && token_index+1<tokens.size() && character_info.at(language).types.at(tokens[token_index+1].id)==7)split_punctuation();
                    punctuation_run++;
                }else{if(punctuation_run>1)split_punctuation();punctuation_run=0;}
                bool individual=character_info.at(language).types.at(token.id)==2;
                if(individual) emit();
                if(current.empty()) start=token.start;
                current_characters.push_back({token.id,token.start,token.end-1});coarse.push_back({token.id,token.start,token.end-1});
                if(language=="cjk" && coarse.size()==30){coarse_words.push_back(std::move(coarse));coarse.clear();}
                current+=token.text;text+=token.text;end=token.end;sum+=token.confidence;count++;
                if(individual) emit();
            }
            emit();
            if(!coarse.empty())coarse_words.push_back(std::move(coarse));
            if(words.empty())continue;
            const auto& settings=character_settings.at(language);
            auto ctc=prepare_ctc_scores(logits,steps,v,character_info.at(language));
            std::string route=settings.at("name").get<std::string>()+"Dummy";
            // The fixed package maps LatinMixedV2/CJKMixed to the same
            // confidence/rejection routes as their printed modes.
            std::vector<float> confidence_features,rejection_features;
            float accepted_sum=0,accepted_weight=0;
            double byte_weighted_sum=0,byte_weight=0;
            for(size_t i=0;i<coarse_words.size();i++) {
                int first=i?(coarse_words[i-1].back().last+coarse_words[i].front().first)/2+1:0;
                int last=i+1==coarse_words.size()?steps-1:(coarse_words[i].back().last+coarse_words[i+1].front().first)/2;
                auto features=word_confidence_features(ctc,coarse_words[i],first,last,character_info.at(language));
                if(line_trace.enabled())rejection_features.insert(rejection_features.end(),features.begin(),features.end());
                size_t weight=coarse_words[i].size();
                if(weight>1 && character_info.at(language).types.at(coarse_words[i][0].id)==7 && character_info.at(language).types.at(coarse_words[i][1].id)==7)weight=1;
                float probability=auxiliary_probability("rejection/"+route,features);
                accepted_sum+=float(weight)*probability;accepted_weight+=float(weight);
                size_t bytes=0;for(const auto& character:coarse_words[i])bytes+=vocab[character.id].size();
                byte_weighted_sum+=double(float(bytes)*probability);byte_weight+=double(bytes);
            }
            const auto& rejection=rejection_settings.at(route);
            // Native default ProcessOption+0x6a disables the alternate
            // single-character threshold, even when this line is one glyph.
            double threshold=rejection.at("threshold");
            line_trace.tensor("rejection_features",rejection_features.data(),rejection_features.size(),{int64_t(coarse_words.size()),21});
            line_trace.document("rejection",{{"route",route},{"mean_acceptance",accepted_weight?accepted_sum/accepted_weight:0},{"threshold",threshold}});
            if(!accepted_weight || double(accepted_sum/accepted_weight)<threshold)continue;
            for(size_t i=0;i<words.size();i++) {
                int first=i?(word_frames[i-1].last+word_frames[i].first)/2+1:0;
                int last=i+1==words.size()?steps-1:(word_frames[i].last+word_frames[i+1].first)/2;
                auto features=word_confidence_features(ctc,word_characters[i],first,last,character_info.at(language));
                if(line_trace.enabled())confidence_features.insert(confidence_features.end(),features.begin(),features.end());
                words[i]["confidence"]=auxiliary_probability("confidence/"+route,features);
            }
            line_trace.tensor("confidence_features",confidence_features.data(),confidence_features.size(),{int64_t(words.size()),21});
            align_word_frames(word_frames,frame_stride,settings.at("pixels_per_space"),settings.at("start_shift"),settings.at("end_shift"),steps,blank_intervals);
            for(size_t i=0;i<words.size();i++)words[i]["bounding_box"]=quad_json(polygon.size()==4?map_quad_word(quad_normalized,word_frames[i].start,word_frames[i].end,flipped):item.clipped_mapping.triangles.empty()?map_line_word(polygon,word_frames[i].start,word_frames[i].end,flipped):map_clipped_polygon_word(item.clipped_mapping,polygon,word_frames[i].start,word_frames[i].end,flipped));
            while(!text.empty() && text.back()==' ') text.pop_back();
            if(text.empty()) continue;
            std::string joined_words;for(const auto& word:words)joined_words+=word.at("text").get<std::string>();
            RecognitionCandidate recognition{polygon,unicode_characters(joined_words),classification.horizontal,false,classification.script,classification.style,0,float(byte_weighted_sum/std::max(1.,byte_weight))};
            if(layout==2) {
                line_box={line_box[3],line_box[0],line_box[1],line_box[2]};
                for(auto& word:words){auto original=word["bounding_box"];json reordered=json::array();for(int index:{6,7,0,1,2,3,4,5})reordered.push_back(original[index]);word["bounding_box"]=std::move(reordered);}
            }
            if(language=="arabic" || language=="hebrew") {
                auto reordered=visual_to_logical(text);json mapped=json::array();
                for(const auto& word:reordered.words){auto item=words.at(word.source);item["text"]=word.text;item["index"]=mapped.size();mapped.push_back(std::move(item));}
                text=std::move(reordered.text);words=std::move(mapped);
            }
            float confidence=classification.handwriting_probability;
            auto calibration=std::lower_bound(handwritten_calibration.begin(),handwritten_calibration.end(),confidence,[](const auto& pair,float value){return pair[0]<value;});
            float calibrated=calibration==handwritten_calibration.end()?handwritten_calibration.back()[1]:(*calibration)[1];
            // Preserve the legacy wrapper's JSON mapping: only the native
            // handwritten category is emitted as handwritten; unknown maps to printed.
            lines.push_back({{"index",lines.size()},{"text",text},{"bounding_box",quad_json(line_box)},{"style",{{"type",classification.style==2?"handwritten":"printed"},{"confidence",calibrated}}},{"word_count",words.size()},{"words",words}});
            recognition_candidates.push_back(std::move(recognition));
            line_boxes.push_back(line_box);rtl_lines+=script==4 || script==8;
            angles.push_back(line_quad_angle(line_box));
        }
        auto suppressed=recognition_suppressed(recognition_candidates);
        json kept_lines=json::array();std::vector<Quad> kept_boxes;angles.clear();rtl_lines=0;
        for(size_t i=0;i<lines.size();i++)if(!suppressed[i]){kept_lines.push_back(std::move(lines[i]));kept_boxes.push_back(line_boxes[i]);angles.push_back(line_quad_angle(line_boxes[i]));rtl_lines+=recognition_candidates[i].script==5 || recognition_candidates[i].script==9;}
        lines=std::move(kept_lines);line_boxes=std::move(kept_boxes);
        float angle=dominant_angle(angles);
        auto order=reading_order(line_boxes,angle,double(rtl_lines)>double(lines.size())*.4);
        json ordered=json::array();
        for(size_t index:order){lines[index]["index"]=ordered.size();ordered.push_back(std::move(lines[index]));}
        return {{"file",file},{"image",{{"width",image.width},{"height",image.height},{"step",step}}},{"image_angle",angle},{"line_count",ordered.size()},{"lines",ordered},{"engine",json::parse(capabilities)}};
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
