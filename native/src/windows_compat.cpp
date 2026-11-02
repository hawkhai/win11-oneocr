// Compatibility for the existing oneocr.h/ctypes API, backed by native source.
// New integrations should use instance-based oneocr_portable.h instead.
#define NOMINMAX
#include <Windows.h>
#include "oneocr_portable.h"
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

namespace {
using Allocator=void*(*)(size_t);
struct Destroy {void operator()(oneocr_context* p) const {oneocr_destroy(p);}};
std::unique_ptr<oneocr_context,Destroy> model;
std::mutex mutex;
std::string utf8(const wchar_t* text) {
    if(!text) throw std::invalid_argument("Null path");
    int size=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,nullptr,0,nullptr,nullptr);
    if(!size) throw std::invalid_argument("Invalid Unicode path");
    std::string result(size_t(size),'\0');
    if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,result.data(),size,nullptr,nullptr)) throw std::invalid_argument("Invalid Unicode path");
    result.pop_back();return result;
}
int transfer(int status,char* result,char* error,char*& output,Allocator allocator) {
    std::unique_ptr<char,decltype(&oneocr_free)> json(result,oneocr_free),message(error,oneocr_free);
    if(status) return status==ONEOCR_MODEL_ERROR || status==ONEOCR_RUNTIME_ERROR ? -18 : status;
    if(!result) return -19;
    size_t length=std::strlen(result)+1;
    output=static_cast<char*>(allocator(length));
    if(!output) return -19;
    std::memcpy(output,result,length);return 0;
}
}
#define EXPORT extern "C" __declspec(dllexport)
EXPORT int initModel(const wchar_t* directory) {
    try {
        auto path=utf8(directory);std::lock_guard lock(mutex);
        oneocr_context* context=nullptr;char* error=nullptr;
        int code=oneocr_create(path.c_str(),&context,&error);oneocr_free(error);
        if(code) return -5;
        model.reset(context);return 0;
    } catch(...) {return -12;}
}
EXPORT int releaseModel() {try {std::lock_guard lock(mutex);model.reset();return 0;} catch(...) {return -12;}}
EXPORT int ocrImageEx(const wchar_t* path,char*& output,Allocator allocator,int maxLines,int width,int height) {
    output=nullptr;
    try {
        if(!path || !allocator) return -12;
        auto name=utf8(path);std::lock_guard lock(mutex);
        if(!model) return -11;
        oneocr_options options{maxLines>0 && maxLines<=1000?maxLines:1000,width,height};
        char *json=nullptr,*error=nullptr;
        int code=oneocr_recognize_file(model.get(),name.c_str(),&options,&json,&error);
        return transfer(code,json,error,output,allocator);
    } catch(...) {return -12;}
}
EXPORT int ocrImage(const wchar_t* path,char*& output,Allocator allocator) {return ocrImageEx(path,output,allocator,0,0,0);}
EXPORT int ocrImageRaw(const unsigned char* pixels,int width,int height,int64_t stride,char*& output,Allocator allocator) {
    output=nullptr;
    try {
        if(!pixels || !allocator || width<=0 || height<=0 || stride<int64_t(width)*4) return -12;
        uint64_t tail=uint64_t(width)*4;
        if(uint64_t(height-1) && uint64_t(stride)>(std::numeric_limits<size_t>::max()-tail)/uint64_t(height-1)) return -12;
        size_t size=size_t(height-1)*size_t(stride)+size_t(tail);
        std::lock_guard lock(mutex);if(!model) return -11;
        char *json=nullptr,*error=nullptr;
        int code=oneocr_recognize_bgra(model.get(),pixels,size,width,height,stride,nullptr,&json,&error);
        return transfer(code,json,error,output,allocator);
    } catch(...) {return -12;}
}
