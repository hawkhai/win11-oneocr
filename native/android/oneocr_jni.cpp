#include <jni.h>
#include "oneocr_portable.h"
#include <cstdint>
#include <string>
#include <cstring>
#include <limits>
#include <memory>

static std::string bytes(JNIEnv* env,jbyteArray value) {
    if(!value) return {};
    jsize length=env->GetArrayLength(value);
    std::string result(size_t(length),'\0');
    env->GetByteArrayRegion(value,0,length,reinterpret_cast<jbyte*>(result.data()));
    return result;
}
static void failure(JNIEnv* env,char* error) {
    jclass type=env->FindClass("java/lang/IllegalStateException");
    if(type) env->ThrowNew(type,error?error:"Native OCR failed");
    oneocr_free(error);
}
static jbyteArray result_bytes(JNIEnv* env,int code,char* result,char* error) {
    if(code) {oneocr_free(result);failure(env,error);return nullptr;}
    oneocr_free(error);
    std::unique_ptr<char,decltype(&oneocr_free)> guard(result,oneocr_free);
    size_t length=result?std::strlen(result):0;
    if(length>size_t(std::numeric_limits<jsize>::max())) {failure(env,nullptr);return nullptr;}
    jbyteArray output=env->NewByteArray(jsize(length));
    if(output && length) env->SetByteArrayRegion(output,0,jsize(length),reinterpret_cast<const jbyte*>(result));
    return output;
}
extern "C" JNIEXPORT jlong JNICALL Java_com_example_oneocr_OneOcr_create(JNIEnv* env,jclass,jbyteArray path) {
    try {
        auto model=bytes(env,path);oneocr_context* context=nullptr;char* error=nullptr;
        if(env->ExceptionCheck()) return 0;
        int code=oneocr_create(model.c_str(),&context,&error);
        if(code) {failure(env,error);return 0;}
        return jlong(reinterpret_cast<intptr_t>(context));
    } catch(...) {failure(env,nullptr);return 0;}
}
extern "C" JNIEXPORT jbyteArray JNICALL Java_com_example_oneocr_OneOcr_file(JNIEnv* env,jclass,jlong handle,jbyteArray path,jint maxLines,jint width,jint height) {
    try {
        auto image=bytes(env,path);char *result=nullptr,*error=nullptr;
        if(env->ExceptionCheck()) return nullptr;
        oneocr_options options{maxLines,width,height};
        int code=oneocr_recognize_file(reinterpret_cast<oneocr_context*>(intptr_t(handle)),image.c_str(),&options,&result,&error);
        return result_bytes(env,code,result,error);
    } catch(...) {failure(env,nullptr);return nullptr;}
}
extern "C" JNIEXPORT jbyteArray JNICALL Java_com_example_oneocr_OneOcr_bgra(JNIEnv* env,jclass,jlong handle,jobject buffer,jint width,jint height,jlong stride,jint maxLines) {
    try {
        auto* data=static_cast<uint8_t*>(env->GetDirectBufferAddress(buffer));
        jlong size=env->GetDirectBufferCapacity(buffer);char *result=nullptr,*error=nullptr;
        if(size<0 || !data) {failure(env,nullptr);return nullptr;}
        oneocr_options options{maxLines,0,0};
        int code=oneocr_recognize_bgra(reinterpret_cast<oneocr_context*>(intptr_t(handle)),data,size_t(size),width,height,stride,&options,&result,&error);
        return result_bytes(env,code,result,error);
    } catch(...) {failure(env,nullptr);return nullptr;}
}
extern "C" JNIEXPORT void JNICALL Java_com_example_oneocr_OneOcr_destroy(JNIEnv*,jclass,jlong handle) {oneocr_destroy(reinterpret_cast<oneocr_context*>(intptr_t(handle)));}
