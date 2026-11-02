#include "oneocr_portable.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <exception>
#include <thread>
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
    try {
        oneocr_context* context=reinterpret_cast<oneocr_context*>(1);char* error=nullptr;
        require(oneocr_create(nullptr,&context,&error)==ONEOCR_INVALID_ARGUMENT,"null model argument");
        require(!context && error,"creation must clear context and report error");oneocr_free(error);error=nullptr;
        require(oneocr_create("__oneocr_missing_model_directory__",&context,&error)!=0,"missing model must fail");
        require(!context && error,"missing model failure contract");oneocr_free(error);error=nullptr;
        char* result=reinterpret_cast<char*>(1);
        require(oneocr_recognize_file(nullptr,"x.png",nullptr,&result,&error)==ONEOCR_INVALID_ARGUMENT,"null context must fail");
        require(!result && error,"recognition must clear output");oneocr_free(error);
        oneocr_destroy(nullptr);oneocr_free(nullptr);
        if(argc>1) {
            for(int i=0;i<3;i++) {
                error=nullptr;
                require(oneocr_create(argv[1],&context,&error)==0,error?error:"model initialization");
                uint8_t pixel[4]={255,255,255,255};result=nullptr;error=nullptr;
                require(oneocr_recognize_bgra(context,pixel,4,2,2,8,nullptr,&result,&error)==ONEOCR_INVALID_ARGUMENT,"undersized buffer must fail");
                oneocr_free(error);error=nullptr;
                uint8_t small[50*49*4]{};
                require(oneocr_recognize_bgra(context,small,sizeof(small),50,49,200,nullptr,&result,&error)==ONEOCR_IMAGE_SIZE_ERROR,"original DLL minimum image size must be enforced");
                require(!result && error,"image size failure must leave JSON null");oneocr_free(error);error=nullptr;
                require(oneocr_recognize_bgra(context,pixel,4,1,1,-4,nullptr,&result,&error)==ONEOCR_INVALID_ARGUMENT,"negative stride must fail");
                oneocr_free(error);oneocr_destroy(context);
            }
        }
        if(argc>2) {
            std::array<std::string,2> results;std::array<std::exception_ptr,2> errors;
            auto recognize=[&](size_t index) {
                oneocr_context* instance=nullptr;char *message=nullptr,*json=nullptr;
                try {
                    int code=oneocr_create(argv[1],&instance,&message);
                    if(code)throw std::runtime_error(message?message:"concurrent initialization failed");
                    oneocr_free(message);message=nullptr;
                    code=oneocr_recognize_file(instance,argv[2],nullptr,&json,&message);
                    if(code)throw std::runtime_error(message?message:"concurrent recognition failed");
                    results[index]=json;
                }catch(...){errors[index]=std::current_exception();}
                oneocr_free(json);oneocr_free(message);oneocr_destroy(instance);
            };
            std::thread first(recognize,0),second(recognize,1);first.join();second.join();
            for(auto error:errors)if(error)std::rethrow_exception(error);
            require(!results[0].empty() && results[0]==results[1],"concurrent independent instances must be deterministic");
        }
        std::cout<<"API ownership and failure contracts: PASS\n";return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
