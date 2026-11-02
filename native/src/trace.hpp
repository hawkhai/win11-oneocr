#pragma once
// Opt-in research tensors only. Normal inference does not write diagnostics.
#include "json.hpp"
#include <atomic>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace oneocr_native {
class TensorTrace {
    std::filesystem::path directory;
public:
    bool enabled()const {return !directory.empty();}
    explicit TensorTrace(const char* role) {
        const char* path=std::getenv("ONEOCR_TRACE_DIR");
        if(!path || !*path)return;
        static std::atomic<unsigned> sequence{0};
        directory=std::filesystem::u8path(path)/(std::string(role)+"-"+std::to_string(sequence.fetch_add(1)));
        std::filesystem::create_directories(directory);
    }
    void tensor(const std::string& name,const float* data,size_t count,const std::vector<int64_t>& shape)const {
        if(directory.empty())return;
        std::ofstream binary(directory/(name+".f32"),std::ios::binary);
        if(!binary.write(reinterpret_cast<const char*>(data),std::streamsize(count*sizeof(float))))
            throw std::runtime_error("Cannot write research tensor trace");
        std::ofstream meta(directory/(name+".json"));
        if(!(meta<<nlohmann::json{{"dtype","float32"},{"shape",shape},{"elements",count}}.dump(2)))
            throw std::runtime_error("Cannot write research tensor metadata");
    }
    void document(const std::string& name,const nlohmann::json& value)const {
        if(directory.empty())return;
        std::ofstream output(directory/(name+".json"));
        if(!(output<<value.dump(2)))throw std::runtime_error("Cannot write research pipeline trace");
    }
};
}
