#pragma once

#include <Windows.h>
#include <string>

// ---------- ALLOC_FUNC ----------
// Caller-provided memory allocator for utf8_json buffer.
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

// OneOcr wrapper load errors
#define OCR_ERR_LOAD_WRAPPER  -90   // LoadLibraryW("oneocr_wrapper.dll") failed
#define OCR_ERR_RESOLVE_API   -91   // GetProcAddress failed for wrapper API

// ---------- DLL function signatures ----------

typedef int (*initModel_t)(const wchar_t* model_dir);
typedef int (*releaseModel_t)();
typedef int (*ocrImage_t)(const wchar_t* image_path, char*& utf8_json, ALLOC_FUNC func);

// ---------- Header-only wrapper class ----------
// Usage:
//   OneOcr ocr(L"path_to_dir_containing_oneocr_wrapper_dll");
//   if (ocr.isOk()) {
//       int ret = ocr.initModel(L"path_to_model_dir");
//       char* json = nullptr;
//       ret = ocr.ocrImage(L"test.jpg", json, malloc);
//       // use json ...
//       free(json);
//       ocr.releaseModel();
//   }

class OneOcr {
public:
    // dll_dir: directory containing oneocr_wrapper.dll
    //          if nullptr, searches in the exe's current directory
    OneOcr(const wchar_t* dll_dir = nullptr) {
        std::wstring dll_path;
        if (dll_dir && dll_dir[0]) {
            dll_path = dll_dir;
            if (dll_path.back() != L'\\' && dll_path.back() != L'/') {
                dll_path += L'\\';
            }
        }
        dll_path += L"oneocr_wrapper.dll";

        // Save and restore CWD so the DLL can find its dependencies
        wchar_t saved_cwd[MAX_PATH] = {0};
        GetCurrentDirectoryW(MAX_PATH, saved_cwd);
        if (dll_dir && dll_dir[0]) {
            SetCurrentDirectoryW(dll_dir);
        }

        m_hDLL = LoadLibraryW(dll_path.c_str());

        SetCurrentDirectoryW(saved_cwd);

        if (!m_hDLL) return;

        m_initModel    = (initModel_t)GetProcAddress(m_hDLL, "initModel");
        m_releaseModel = (releaseModel_t)GetProcAddress(m_hDLL, "releaseModel");
        m_ocrImage     = (ocrImage_t)GetProcAddress(m_hDLL, "ocrImage");

        if (!m_initModel || !m_releaseModel || !m_ocrImage) {
            FreeLibrary(m_hDLL);
            m_hDLL = nullptr;
        }
    }

    ~OneOcr() {
        if (m_hDLL) {
            if (m_releaseModel) m_releaseModel();
            // DLL is not freed to avoid crash if OCR engine holds resources.
        }
    }

    // Returns true if oneocr_wrapper.dll was loaded and all APIs resolved.
    bool isOk() const { return m_hDLL != nullptr; }

    int initModel(const wchar_t* model_dir) {
        if (!m_initModel) return OCR_ERR_LOAD_WRAPPER;
        return m_initModel(model_dir);
    }

    int releaseModel() {
        if (!m_releaseModel) return OCR_ERR_LOAD_WRAPPER;
        return m_releaseModel();
    }

    int ocrImage(const wchar_t* image_path, char*& utf8_json, ALLOC_FUNC func) {
        if (!m_ocrImage) return OCR_ERR_LOAD_WRAPPER;
        return m_ocrImage(image_path, utf8_json, func);
    }

private:
    OneOcr(const OneOcr&) = delete;
    OneOcr& operator=(const OneOcr&) = delete;

    HINSTANCE     m_hDLL         = nullptr;
    initModel_t    m_initModel    = nullptr;
    releaseModel_t m_releaseModel = nullptr;
    ocrImage_t     m_ocrImage     = nullptr;
};
