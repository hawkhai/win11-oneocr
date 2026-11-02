#pragma once
#include "oneocr_portable.h"
#include <memory>
#include <stdexcept>
#include <string>

namespace oneocr {
class Engine {
    struct Destroy { void operator()(oneocr_context* p) const { oneocr_destroy(p); } };
    std::unique_ptr<oneocr_context,Destroy> context_;
    static std::string take(char* p) {
        std::unique_ptr<char,decltype(&oneocr_free)> guard(p,oneocr_free);
        return p ? std::string(p) : std::string();
    }
public:
    explicit Engine(const std::string& models) {
        oneocr_context* p=nullptr; char* error=nullptr;
        int code=oneocr_create(models.c_str(),&p,&error);
        std::string message=take(error);
        if(code) throw std::runtime_error(message);
        context_.reset(p);
    }
    std::string recognize(const std::string& path, const oneocr_options& options={}) {
        char *json=nullptr,*error=nullptr;
        int code=oneocr_recognize_file(context_.get(),path.c_str(),&options,&json,&error);
        std::string result=take(json),message=take(error);
        if(code) throw std::runtime_error(message);
        return result;
    }
    std::string recognize(const uint8_t* bgra,size_t bytes,int width,int height,int64_t stride,const oneocr_options& options={}) {
        char *json=nullptr,*error=nullptr;
        int code=oneocr_recognize_bgra(context_.get(),bgra,bytes,width,height,stride,&options,&json,&error);
        std::string result=take(json),message=take(error);
        if(code) throw std::runtime_error(message);
        return result;
    }
};
}
