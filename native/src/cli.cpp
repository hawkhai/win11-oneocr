#include "oneocr_portable.hpp"
#include "json.hpp"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
namespace fs=std::filesystem;
static fs::path executable_path(const std::string& fallback) {
#ifdef _WIN32
    std::wstring buffer(32768,L'\0');
    DWORD length=GetModuleFileNameW(nullptr,buffer.data(),DWORD(buffer.size()));
    if(length && length<buffer.size()) {buffer.resize(length);return fs::path(buffer);}
#elif defined(__linux__)
    std::error_code error;auto path=fs::read_symlink("/proc/self/exe",error);
    if(!error) return path;
#elif defined(__APPLE__)
    uint32_t length=0;_NSGetExecutablePath(nullptr,&length);
    std::string buffer(length,'\0');
    if(_NSGetExecutablePath(buffer.data(),&length)==0) return fs::path(buffer.c_str());
#endif
    return fs::absolute(fs::u8path(fallback));
}
static int number(const std::string& s) {
    int n=0;auto result=std::from_chars(s.data(),s.data()+s.size(),n);
    if(result.ec!=std::errc{} || result.ptr!=s.data()+s.size() || n<=0) throw std::invalid_argument("Expected a positive integer: "+s);
    return n;
}
static int command(const std::vector<std::string>& args) {
    try {
        oneocr_options options{};std::vector<std::string> images;
        std::string models,output;bool text=false,quiet=false;
        for(size_t i=1;i<args.size();i++) {
            const auto& arg=args[i];
            auto value=[&]()->const std::string& {if(++i>=args.size()) throw std::invalid_argument("Missing value for "+arg);return args[i];};
            if(arg=="--help" || arg=="-h") {
                std::cout<<"OneOCR native source runtime (experimental; full DLL parity not yet achieved)\n"
                    "Usage: ocr <image> [images ...] [options]\n"
                    "  --model-dir DIR    Independent ONNX models and manifest\n"
                    "  --text, -t         Print recognized text\n"
                    "  --output, -o FILE  Write JSON (one image only)\n"
                    "  --max-lines N      Maximum lines, 1..1000 (default 1000)\n"
                    "  --resize WxH       Internal maximum dimensions (default 1152x768)\n"
                    "  --quiet, -q        Suppress progress\n"
                    "  --capabilities     Show verified capability status\n";
                return 0;
            }
            if(arg=="--capabilities") {std::cout<<oneocr_capabilities()<<'\n';return 0;}
            if(arg=="--text" || arg=="-t") text=true;
            else if(arg=="--quiet" || arg=="-q") quiet=true;
            else if(arg=="--model-dir") models=value();
            else if(arg=="--output" || arg=="-o") output=value();
            else if(arg=="--max-lines") options.max_lines=number(value());
            else if(arg=="--resize") {const auto& v=value();auto x=v.find('x');if(x==std::string::npos) throw std::invalid_argument("Resize must be WxH");options.resize_width=number(v.substr(0,x));options.resize_height=number(v.substr(x+1));}
            else if(arg.rfind('-',0)==0) throw std::invalid_argument("Unknown option: "+arg);
            else images.push_back(arg);
        }
        if(images.empty()) throw std::invalid_argument("No input images; use --help");
        if(!output.empty() && images.size()!=1) throw std::invalid_argument("--output requires exactly one input image");
        if(models.empty()) {
            fs::path exe=executable_path(args[0]).parent_path();
            for(const auto& p:{fs::path("models"),exe/"models",exe/"../models",exe/"../share/oneocr/models"}) if(fs::exists(p/"manifest.json")) {auto u=p.u8string();models.assign(u.begin(),u.end());break;}
            if(models.empty()) throw std::invalid_argument("Model directory not found; use --model-dir");
        }
        oneocr::Engine engine(models);
        for(const auto& path:images) {
            if(!quiet) std::cerr<<"OCR: "<<path<<'\n';
            std::string result=engine.recognize(path,options);
            if(text) {
                auto json=nlohmann::json::parse(result);
                for(const auto& line:json.at("lines")) std::cout<<line.at("text").get<std::string>()<<'\n';
            } else {
                std::string destination=output.empty()?path+".json":output;
                std::ofstream f(fs::u8path(destination),std::ios::binary);
                if(!f || !(f<<result<<'\n')) throw std::runtime_error("Cannot write output: "+destination);
            }
        }
        return 0;
    } catch(const std::exception& e) {std::cerr<<"ERROR: "<<e.what()<<'\n';return 1;}
}
#ifdef _WIN32
int wmain(int argc,wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    std::vector<std::string> args;
    for(int i=0;i<argc;i++) {
        int bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[i],-1,nullptr,0,nullptr,nullptr);
        if(!bytes) return 1;
        std::string s(size_t(bytes),'\0');
        WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[i],-1,s.data(),bytes,nullptr,nullptr);
        s.pop_back();args.push_back(std::move(s));
    }
    return command(args);
}
#else
int main(int argc,char** argv) {return command(std::vector<std::string>(argv,argv+argc));}
#endif
