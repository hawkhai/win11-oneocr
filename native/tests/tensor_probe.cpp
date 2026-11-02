// Research-only tensor replay: no OCR postprocessing, downloads or model edits.
#include <onnxruntime_cxx_api.h>
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
namespace fs=std::filesystem;
int main(int argc,char** argv) {
    try {
        if(argc<4)throw std::runtime_error("Usage: oneocr_tensor_probe model input_directory output_directory [disabled]");
        fs::path root=fs::u8path(argv[2]),output=fs::u8path(argv[3]);fs::create_directories(output);
        Ort::Env environment(ORT_LOGGING_LEVEL_ERROR,"tensor-probe");Ort::SessionOptions options;options.SetIntraOpNumThreads(1);options.SetGraphOptimizationLevel(argc>4?ORT_DISABLE_ALL:ORT_ENABLE_ALL);
        Ort::Session session(environment,fs::u8path(argv[1]).c_str(),options);Ort::AllocatorWithDefaultOptions allocator;auto memory=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
        std::vector<std::string> input_names,output_names;std::vector<std::vector<float>> buffers;std::vector<Ort::Value> inputs;
        for(size_t i=0;i<session.GetInputCount();i++) {
            std::string name=session.GetInputNameAllocated(i,allocator).get();input_names.push_back(name);nlohmann::json metadata;std::ifstream(root/(name+".json"))>>metadata;auto shape=metadata.at("shape").get<std::vector<int64_t>>();
            std::ifstream stream(root/(name+".f32"),std::ios::binary|std::ios::ate);if(!stream)throw std::runtime_error("Missing input tensor");size_t bytes=size_t(stream.tellg());buffers.emplace_back(bytes/4);stream.seekg(0);stream.read(reinterpret_cast<char*>(buffers.back().data()),bytes);
            inputs.push_back(Ort::Value::CreateTensor<float>(memory,buffers.back().data(),buffers.back().size(),shape.data(),shape.size()));
        }
        for(size_t i=0;i<session.GetOutputCount();i++)output_names.push_back(session.GetOutputNameAllocated(i,allocator).get());
        std::vector<const char*> in,out;for(auto& name:input_names)in.push_back(name.c_str());for(auto& name:output_names)out.push_back(name.c_str());
        auto values=session.Run(Ort::RunOptions{nullptr},in.data(),inputs.data(),inputs.size(),out.data(),out.size());
        for(size_t i=0;i<values.size();i++) {
            auto type=values[i].GetTensorTypeAndShapeInfo();auto element=type.GetElementType();size_t bytes=type.GetElementCount()*(element==ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT?4:1);if(element!=ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT && element!=ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8 && element!=ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8)throw std::runtime_error("Unsupported output type");
            std::string name=output_names[i];for(auto& c:name)if(c=='/' || c=='\\')c='_';std::ofstream stream(output/(name+".bin"),std::ios::binary);stream.write(static_cast<const char*>(values[i].GetTensorRawData()),bytes);std::ofstream(output/(name+".json"))<<nlohmann::json{{"name",output_names[i]},{"shape",type.GetShape()},{"element_type",int(element)}}.dump(2);
        }
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
