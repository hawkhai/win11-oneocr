#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#include <Windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "json.hpp"

using json = nlohmann::json;

// ---------- error codes (keep in sync with oneocr.h) ----------

#define OCR_OK                  0
#define OCR_ERR_LOAD_DLL       -1
#define OCR_ERR_RESOLVE_FUNC   -2
#define OCR_ERR_INIT_OPTIONS   -3
#define OCR_ERR_DELAY_LOAD     -4
#define OCR_ERR_CREATE_PIPE    -5
#define OCR_ERR_NOT_INIT      -11
#define OCR_ERR_INVALID_PARAM -12
#define OCR_ERR_LOAD_IMAGE    -13
#define OCR_ERR_ALLOC_BGRA    -14
#define OCR_ERR_UNSUPPORTED   -15
#define OCR_ERR_PROC_OPTIONS  -16
#define OCR_ERR_MAX_LINES     -17
#define OCR_ERR_RUN_PIPELINE  -18
#define OCR_ERR_ALLOC_JSON    -19

// ---------- export macros ----------

typedef void* (*ALLOC_FUNC)(size_t size);

#define WRAPPER_EXPORT extern "C" __declspec(dllexport)

// ---------- oneocr.dll internal types ----------

typedef struct {
  __int32 t;
  __int32 col;
  __int32 row;
  __int32 _unk;
  __int64 step;
  __int64 data_ptr;
} OcrImg;

typedef __int64(__cdecl *CreateOcrInitOptions_t)(__int64 *);
typedef __int64(__cdecl *GetOcrLineCount_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrLine_t)(__int64, __int64, __int64 *);
typedef __int64(__cdecl *GetOcrLineContent_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrLineBoundingBox_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrLineWordCount_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrWord_t)(__int64, __int64, __int64 *);
typedef __int64(__cdecl *GetOcrWordContent_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrWordBoundingBox_t)(__int64, __int64 *);
typedef __int64(__cdecl *OcrProcessOptionsSetMaxRecognitionLineCount_t)(
    __int64, __int64);
typedef __int64(__cdecl *RunOcrPipeline_t)(__int64, OcrImg *, __int64,
                                           __int64 *);
typedef __int64(__cdecl *CreateOcrProcessOptions_t)(__int64 *);
typedef __int64(__cdecl *OcrInitOptionsSetUseModelDelayLoad_t)(__int64, char);
typedef __int64(__cdecl *CreateOcrPipeline_t)(__int64, __int64, __int64,
                                              __int64 *);

// ---------- global state ----------

static HINSTANCE g_hDLL = NULL;
static __int64 g_pipeline = 0;

static CreateOcrInitOptions_t pCreateOcrInitOptions = NULL;
static GetOcrLineCount_t pGetOcrLineCount = NULL;
static CreateOcrProcessOptions_t pCreateOcrProcessOptions = NULL;
static CreateOcrPipeline_t pCreateOcrPipeline = NULL;
static OcrInitOptionsSetUseModelDelayLoad_t pOcrInitOptionsSetUseModelDelayLoad = NULL;
static OcrProcessOptionsSetMaxRecognitionLineCount_t pOcrProcessOptionsSetMaxRecognitionLineCount = NULL;
static RunOcrPipeline_t pRunOcrPipeline = NULL;
static GetOcrLine_t pGetOcrLine = NULL;
static GetOcrLineContent_t pGetOcrLineContent = NULL;
static GetOcrLineBoundingBox_t pGetOcrLineBoundingBox = NULL;
static GetOcrLineWordCount_t pGetOcrLineWordCount = NULL;
static GetOcrWord_t pGetOcrWord = NULL;
static GetOcrWordContent_t pGetOcrWordContent = NULL;
static GetOcrWordBoundingBox_t pGetOcrWordBoundingBox = NULL;

// ---------- helpers ----------

static std::string wchar_to_utf8(const wchar_t *wstr) {
  if (!wstr) return "";
  int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  if (len <= 0) return "";
  std::string s(len - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &s[0], len, NULL, NULL);
  return s;
}

// ---------- exported functions ----------

WRAPPER_EXPORT int initModel(const wchar_t *model_dir) {
  if (g_hDLL) return 0; // already initialized

  // Build DLL path: model_dir\oneocr.dll
  std::wstring dll_path = std::wstring(model_dir) + L"\\oneocr.dll";
  g_hDLL = LoadLibraryW(dll_path.c_str());
  if (!g_hDLL) return OCR_ERR_LOAD_DLL;

  // Resolve all function pointers
  pCreateOcrInitOptions = (CreateOcrInitOptions_t)GetProcAddress(g_hDLL, "CreateOcrInitOptions");
  pGetOcrLineCount = (GetOcrLineCount_t)GetProcAddress(g_hDLL, "GetOcrLineCount");
  pCreateOcrProcessOptions = (CreateOcrProcessOptions_t)GetProcAddress(g_hDLL, "CreateOcrProcessOptions");
  pCreateOcrPipeline = (CreateOcrPipeline_t)GetProcAddress(g_hDLL, "CreateOcrPipeline");
  pOcrInitOptionsSetUseModelDelayLoad = (OcrInitOptionsSetUseModelDelayLoad_t)GetProcAddress(g_hDLL, "OcrInitOptionsSetUseModelDelayLoad");
  pOcrProcessOptionsSetMaxRecognitionLineCount = (OcrProcessOptionsSetMaxRecognitionLineCount_t)GetProcAddress(g_hDLL, "OcrProcessOptionsSetMaxRecognitionLineCount");
  pRunOcrPipeline = (RunOcrPipeline_t)GetProcAddress(g_hDLL, "RunOcrPipeline");
  pGetOcrLine = (GetOcrLine_t)GetProcAddress(g_hDLL, "GetOcrLine");
  pGetOcrLineContent = (GetOcrLineContent_t)GetProcAddress(g_hDLL, "GetOcrLineContent");
  pGetOcrLineBoundingBox = (GetOcrLineBoundingBox_t)GetProcAddress(g_hDLL, "GetOcrLineBoundingBox");
  pGetOcrLineWordCount = (GetOcrLineWordCount_t)GetProcAddress(g_hDLL, "GetOcrLineWordCount");
  pGetOcrWord = (GetOcrWord_t)GetProcAddress(g_hDLL, "GetOcrWord");
  pGetOcrWordContent = (GetOcrWordContent_t)GetProcAddress(g_hDLL, "GetOcrWordContent");
  pGetOcrWordBoundingBox = (GetOcrWordBoundingBox_t)GetProcAddress(g_hDLL, "GetOcrWordBoundingBox");

  if (!pCreateOcrInitOptions || !pCreateOcrProcessOptions || !pCreateOcrPipeline ||
      !pOcrInitOptionsSetUseModelDelayLoad ||
      !pOcrProcessOptionsSetMaxRecognitionLineCount || !pRunOcrPipeline ||
      !pGetOcrLineCount || !pGetOcrLine || !pGetOcrLineContent ||
      !pGetOcrLineBoundingBox || !pGetOcrLineWordCount || !pGetOcrWord ||
      !pGetOcrWordContent || !pGetOcrWordBoundingBox) {
    FreeLibrary(g_hDLL);
    g_hDLL = NULL;
    return OCR_ERR_RESOLVE_FUNC;
  }

  // Initialize OCR pipeline
  __int64 ctx = 0;
  __int64 res = pCreateOcrInitOptions(&ctx);
  if (res != 0) { FreeLibrary(g_hDLL); g_hDLL = NULL; return OCR_ERR_INIT_OPTIONS; }

  res = pOcrInitOptionsSetUseModelDelayLoad(ctx, 0);
  if (res != 0) { FreeLibrary(g_hDLL); g_hDLL = NULL; return OCR_ERR_DELAY_LOAD; }

  const char *key = {"kj)TGtrK>f]b[Piow.gU+nC@s\"\"\"\"\"\"4"};

  // Build model path: model_dir\oneocr.onemodel
  std::wstring model_wpath = std::wstring(model_dir) + L"\\oneocr.onemodel";
  // oneocr.dll expects char* path
  std::string model_path = wchar_to_utf8(model_wpath.c_str());

  res = pCreateOcrPipeline((__int64)model_path.c_str(), (__int64)key, ctx,
                           &g_pipeline);
  if (res != 0) {
    g_pipeline = 0;
    FreeLibrary(g_hDLL);
    g_hDLL = NULL;
    return OCR_ERR_CREATE_PIPE;
  }

  return OCR_OK;
}

WRAPPER_EXPORT int releaseModel() {
  if (g_hDLL) {
    FreeLibrary(g_hDLL);
    g_hDLL = NULL;
  }
  g_pipeline = 0;
  pCreateOcrInitOptions = NULL;
  pGetOcrLineCount = NULL;
  pCreateOcrProcessOptions = NULL;
  pCreateOcrPipeline = NULL;
  pOcrInitOptionsSetUseModelDelayLoad = NULL;
  pOcrProcessOptionsSetMaxRecognitionLineCount = NULL;
  pRunOcrPipeline = NULL;
  pGetOcrLine = NULL;
  pGetOcrLineContent = NULL;
  pGetOcrLineBoundingBox = NULL;
  pGetOcrLineWordCount = NULL;
  pGetOcrWord = NULL;
  pGetOcrWordContent = NULL;
  pGetOcrWordBoundingBox = NULL;
  return 0;
}

WRAPPER_EXPORT int ocrImage(const wchar_t *image_path, char *&utf8_json, ALLOC_FUNC func) {
  if (!g_hDLL || !g_pipeline) return OCR_ERR_NOT_INIT;
  if (!image_path || !func) return OCR_ERR_INVALID_PARAM;

  std::string path_u8 = wchar_to_utf8(image_path);

  // Load image with _wfopen + stbi_load_from_file to support Unicode paths
  int width = 0, height = 0, channels = 0;
  unsigned char *data = nullptr;
  FILE *f = _wfopen(image_path, L"rb");
  if (!f) return OCR_ERR_LOAD_IMAGE;
  data = stbi_load_from_file(f, &width, &height, &channels, 0);
  fclose(f);
  if (!data) return OCR_ERR_LOAD_IMAGE;

  // Convert to BGRA
  unsigned char *bgra = (unsigned char *)malloc(width * height * 4);
  if (!bgra) {
    stbi_image_free(data);
    return OCR_ERR_ALLOC_BGRA;
  }

  if (channels == 3) {
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i * 3 + 2]; // B
      bgra[i * 4 + 1] = data[i * 3 + 1]; // G
      bgra[i * 4 + 2] = data[i * 3 + 0]; // R
      bgra[i * 4 + 3] = 255;             // A
    }
  } else if (channels == 4) {
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i * 4 + 2]; // B
      bgra[i * 4 + 1] = data[i * 4 + 1]; // G
      bgra[i * 4 + 2] = data[i * 4 + 0]; // R
      bgra[i * 4 + 3] = data[i * 4 + 3]; // A
    }
  } else if (channels == 1) {
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i]; // B
      bgra[i * 4 + 1] = data[i]; // G
      bgra[i * 4 + 2] = data[i]; // R
      bgra[i * 4 + 3] = 255;     // A
    }
  } else {
    stbi_image_free(data);
    free(bgra);
    return OCR_ERR_UNSUPPORTED;
  }
  stbi_image_free(data);

  size_t step = (size_t)width * 4;
  OcrImg img = {.t = 3,
                .col = width,
                .row = height,
                ._unk = 0,
                .step = (__int64)step,
                .data_ptr = (__int64)reinterpret_cast<char *>(bgra)};

  // Create process options
  __int64 opt = 0;
  __int64 res = pCreateOcrProcessOptions(&opt);
  if (res != 0) { free(bgra); return OCR_ERR_PROC_OPTIONS; }

  res = pOcrProcessOptionsSetMaxRecognitionLineCount(opt, 1000);
  if (res != 0) { free(bgra); return OCR_ERR_MAX_LINES; }

  // Run OCR
  __int64 instance = 0;
  res = pRunOcrPipeline(g_pipeline, &img, opt, &instance);
  free(bgra);
  if (res != 0) return OCR_ERR_RUN_PIPELINE;

  // Collect results into JSON
  __int64 lc = 0;
  if (pGetOcrLineCount(instance, &lc) != 0) lc = 0;

  json result;
  result["file"] = path_u8;
  result["image"] = {{"width", width}, {"height", height}, {"step", (__int64)step}};
  result["line_count"] = lc;
  result["lines"] = json::array();

  for (__int64 lci = 0; lci < lc; lci++) {
    __int64 line = 0;
    pGetOcrLine(instance, lci, &line);
    if (!line) continue;

    __int64 line_content = 0;
    pGetOcrLineContent(line, &line_content);
    char *lcs = reinterpret_cast<char *>(line_content);

    __int64 line_bbox = 0;
    pGetOcrLineBoundingBox(line, &line_bbox);
    float *lb = reinterpret_cast<float *>(line_bbox);

    json line_obj;
    line_obj["index"] = lci;
    line_obj["text"] = std::string(lcs);
    if (lb) {
      line_obj["bbox"] = {lb[0], lb[1], lb[2], lb[3]};
    }

    __int64 lr = 0;
    pGetOcrLineWordCount(line, &lr);
    line_obj["word_count"] = lr;
    line_obj["words"] = json::array();

    for (__int64 j = 0; j < lr; j++) {
      __int64 word = 0;
      __int64 word_bbox_ptr = 0;
      __int64 word_content = 0;
      pGetOcrWord(line, j, &word);
      pGetOcrWordContent(word, &word_content);
      pGetOcrWordBoundingBox(word, &word_bbox_ptr);
      char *wcs = reinterpret_cast<char *>(word_content);
      float *wb = reinterpret_cast<float *>(word_bbox_ptr);

      json word_obj;
      word_obj["index"] = j;
      word_obj["text"] = std::string(wcs);
      if (wb) {
        word_obj["bbox"] = {wb[0], wb[1], wb[2], wb[3]};
      }
      line_obj["words"].push_back(word_obj);
    }
    result["lines"].push_back(line_obj);
  }

  // Serialize JSON to UTF-8 string
  std::string json_str = result.dump(2);
  size_t json_len = json_str.size() + 1; // +1 for '\0'

  // Allocate memory using caller's allocator
  utf8_json = (char *)func(json_len);
  if (!utf8_json) return OCR_ERR_ALLOC_JSON;

  memcpy(utf8_json, json_str.c_str(), json_len);
  return OCR_OK;
}
