#pragma once

#include <Windows.h>

#ifdef ONEOCR_WRAPPER_EXPORTS
#define ONEOCR_API __declspec(dllexport)
#else
#define ONEOCR_API __declspec(dllimport)
#endif

// ALLOC_FUNC: caller-provided memory allocator for utf8_json buffer
// size: bytes to allocate (including '\0')
// return: pointer to allocated memory
typedef void* (*ALLOC_FUNC)(size_t size);

// ---------- error codes ----------

#define OCR_OK                  0   // success

// initModel errors
#define OCR_ERR_LOAD_DLL       -1   // LoadLibraryW("oneocr.dll") failed
#define OCR_ERR_RESOLVE_FUNC   -2   // GetProcAddress failed for critical functions
#define OCR_ERR_INIT_OPTIONS   -3   // CreateOcrInitOptions failed
#define OCR_ERR_DELAY_LOAD     -4   // OcrInitOptionsSetUseModelDelayLoad failed
#define OCR_ERR_CREATE_PIPE    -5   // CreateOcrPipeline failed (bad key or model)

// ocrImage errors
#define OCR_ERR_NOT_INIT      -11   // model not initialized (call initModel first)
#define OCR_ERR_INVALID_PARAM -12   // image_path or func is NULL
#define OCR_ERR_LOAD_IMAGE    -13   // stb_image failed to load the image file
#define OCR_ERR_ALLOC_BGRA    -14   // malloc failed for BGRA buffer
#define OCR_ERR_UNSUPPORTED   -15   // unsupported image channel count
#define OCR_ERR_PROC_OPTIONS  -16   // CreateOcrProcessOptions failed
#define OCR_ERR_MAX_LINES     -17   // OcrProcessOptionsSetMaxRecognitionLineCount failed
#define OCR_ERR_RUN_PIPELINE  -18   // RunOcrPipeline failed
#define OCR_ERR_ALLOC_JSON    -19   // ALLOC_FUNC returned NULL

#ifdef __cplusplus
extern "C" {
#endif

// return OCR_OK(0) on success, negative error code on failure.

// init OCR model, model_dir is the directory containing oneocr.dll and oneocr.onemodel
ONEOCR_API int initModel(const wchar_t* model_dir);

// release OCR model and free resources
ONEOCR_API int releaseModel();

// run OCR on an image file
// image_path: image file path (wide char)
// utf8_json:  [out] UTF-8 JSON result string, allocated by func
// func:       caller-provided allocator for utf8_json
ONEOCR_API int ocrImage(const wchar_t* image_path, char*& utf8_json, ALLOC_FUNC func);

#ifdef __cplusplus
}
#endif
