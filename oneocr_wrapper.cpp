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
#define OCR_ERR_SET_RESIZE    -20

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

// 8-point bounding box (4 corners x 2 coords)
typedef struct {
  float x1, y1;  // top-left
  float x2, y2;  // top-right
  float x3, y3;  // bottom-right
  float x4, y4;  // bottom-left
} OcrBBox;

typedef __int64(__cdecl *CreateOcrInitOptions_t)(__int64 *);
typedef __int64(__cdecl *OcrInitOptionsSetUseModelDelayLoad_t)(__int64, char);
typedef __int64(__cdecl *CreateOcrPipeline_t)(__int64, __int64, __int64, __int64 *);
typedef __int64(__cdecl *CreateOcrProcessOptions_t)(__int64 *);
typedef __int64(__cdecl *OcrProcessOptionsSetMaxRecognitionLineCount_t)(__int64, __int64);
typedef __int64(__cdecl *OcrProcessOptionsGetMaxRecognitionLineCount_t)(__int64, __int64 *);
typedef __int64(__cdecl *OcrProcessOptionsSetResizeResolution_t)(__int64, __int32, __int32);
typedef __int64(__cdecl *OcrProcessOptionsGetResizeResolution_t)(__int64, __int64 *, __int64 *);
typedef __int64(__cdecl *RunOcrPipeline_t)(__int64, OcrImg *, __int64, __int64 *);
typedef __int64(__cdecl *GetImageAngle_t)(__int64, float *);
typedef __int64(__cdecl *GetOcrLineCount_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrLine_t)(__int64, __int64, __int64 *);
typedef __int64(__cdecl *GetOcrLineContent_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrLineBoundingBox_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrLineStyle_t)(__int64, __int32 *, float *);
typedef __int64(__cdecl *GetOcrLineWordCount_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrWord_t)(__int64, __int64, __int64 *);
typedef __int64(__cdecl *GetOcrWordContent_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrWordBoundingBox_t)(__int64, __int64 *);
typedef __int64(__cdecl *GetOcrWordConfidence_t)(__int64, float *);
typedef void(__cdecl *ReleaseOcrResult_t)(__int64);
typedef void(__cdecl *ReleaseOcrInitOptions_t)(__int64);
typedef void(__cdecl *ReleaseOcrPipeline_t)(__int64);
typedef void(__cdecl *ReleaseOcrProcessOptions_t)(__int64);

// ---------- global state ----------

static HINSTANCE g_hDLL = NULL;
static __int64 g_pipeline = 0;
static __int64 g_initOptions = 0;

static CreateOcrInitOptions_t                    pCreateOcrInitOptions = NULL;
static OcrInitOptionsSetUseModelDelayLoad_t      pOcrInitOptionsSetUseModelDelayLoad = NULL;
static CreateOcrPipeline_t                       pCreateOcrPipeline = NULL;
static CreateOcrProcessOptions_t                 pCreateOcrProcessOptions = NULL;
static OcrProcessOptionsSetMaxRecognitionLineCount_t pOcrProcessOptionsSetMaxRecognitionLineCount = NULL;
static OcrProcessOptionsGetMaxRecognitionLineCount_t pOcrProcessOptionsGetMaxRecognitionLineCount = NULL;
static OcrProcessOptionsSetResizeResolution_t    pOcrProcessOptionsSetResizeResolution = NULL;
static OcrProcessOptionsGetResizeResolution_t    pOcrProcessOptionsGetResizeResolution = NULL;
static RunOcrPipeline_t                          pRunOcrPipeline = NULL;
static GetImageAngle_t                           pGetImageAngle = NULL;
static GetOcrLineCount_t                         pGetOcrLineCount = NULL;
static GetOcrLine_t                              pGetOcrLine = NULL;
static GetOcrLineContent_t                       pGetOcrLineContent = NULL;
static GetOcrLineBoundingBox_t                   pGetOcrLineBoundingBox = NULL;
static GetOcrLineStyle_t                         pGetOcrLineStyle = NULL;
static GetOcrLineWordCount_t                     pGetOcrLineWordCount = NULL;
static GetOcrWord_t                              pGetOcrWord = NULL;
static GetOcrWordContent_t                       pGetOcrWordContent = NULL;
static GetOcrWordBoundingBox_t                   pGetOcrWordBoundingBox = NULL;
static GetOcrWordConfidence_t                    pGetOcrWordConfidence = NULL;
static ReleaseOcrResult_t                        pReleaseOcrResult = NULL;
static ReleaseOcrInitOptions_t                   pReleaseOcrInitOptions = NULL;
static ReleaseOcrPipeline_t                      pReleaseOcrPipeline = NULL;
static ReleaseOcrProcessOptions_t                pReleaseOcrProcessOptions = NULL;

// ---------- helpers ----------

static std::string wchar_to_utf8(const wchar_t *wstr) {
  if (!wstr) return "";
  int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  if (len <= 0) return "";
  std::string s(len - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &s[0], len, NULL, NULL);
  return s;
}

static void clear_function_pointers() {
  pCreateOcrInitOptions = NULL;
  pOcrInitOptionsSetUseModelDelayLoad = NULL;
  pCreateOcrPipeline = NULL;
  pCreateOcrProcessOptions = NULL;
  pOcrProcessOptionsSetMaxRecognitionLineCount = NULL;
  pOcrProcessOptionsGetMaxRecognitionLineCount = NULL;
  pOcrProcessOptionsSetResizeResolution = NULL;
  pOcrProcessOptionsGetResizeResolution = NULL;
  pRunOcrPipeline = NULL;
  pGetImageAngle = NULL;
  pGetOcrLineCount = NULL;
  pGetOcrLine = NULL;
  pGetOcrLineContent = NULL;
  pGetOcrLineBoundingBox = NULL;
  pGetOcrLineStyle = NULL;
  pGetOcrLineWordCount = NULL;
  pGetOcrWord = NULL;
  pGetOcrWordContent = NULL;
  pGetOcrWordBoundingBox = NULL;
  pGetOcrWordConfidence = NULL;
  pReleaseOcrResult = NULL;
  pReleaseOcrInitOptions = NULL;
  pReleaseOcrPipeline = NULL;
  pReleaseOcrProcessOptions = NULL;
}

// Convert image bytes to BGRA; returns nullptr on unsupported channel count
static unsigned char *to_bgra(const unsigned char *data, int width, int height, int channels) {
  unsigned char *bgra = (unsigned char *)malloc((size_t)width * height * 4);
  if (!bgra) return nullptr;

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
    free(bgra);
    return nullptr;
  }
  return bgra;
}

// Build JSON from OCR result handle (full 8-point bbox, confidence, angle, style)
static json build_result_json(const char *file_path_utf8, int width, int height,
                              __int64 step, __int64 instance) {
  json result;
  result["file"] = std::string(file_path_utf8);
  result["image"] = {{"width", width}, {"height", height}, {"step", step}};

  // Image angle
  float angle = 0.0f;
  if (pGetImageAngle && pGetImageAngle(instance, &angle) == 0) {
    result["image_angle"] = angle;
  } else {
    result["image_angle"] = nullptr;
  }

  __int64 lc = 0;
  if (pGetOcrLineCount(instance, &lc) != 0) lc = 0;
  result["line_count"] = lc;
  result["lines"] = json::array();

  for (__int64 lci = 0; lci < lc; lci++) {
    __int64 line = 0;
    pGetOcrLine(instance, lci, &line);
    if (!line) continue;

    // Line text
    __int64 line_content = 0;
    pGetOcrLineContent(line, &line_content);
    char *lcs = reinterpret_cast<char *>(line_content);

    // Line bounding box (8-point)
    __int64 line_bbox = 0;
    pGetOcrLineBoundingBox(line, &line_bbox);
    OcrBBox *lb = reinterpret_cast<OcrBBox *>(line_bbox);

    json line_obj;
    line_obj["index"] = lci;
    line_obj["text"] = lcs ? std::string(lcs) : "";
    if (lb) {
      line_obj["bounding_box"] = {
        lb->x1, lb->y1, lb->x2, lb->y2,
        lb->x3, lb->y3, lb->x4, lb->y4
      };
    }

    // Line style: handwritten vs printed
    if (pGetOcrLineStyle) {
      __int32 style = -1;
      float style_confidence = 0.0f;
      if (pGetOcrLineStyle(line, &style, &style_confidence) == 0) {
        line_obj["style"] = {
          {"type", style == 0 ? "handwritten" : "printed"},
          {"confidence", style_confidence}
        };
      }
    }

    // Words
    __int64 wc = 0;
    pGetOcrLineWordCount(line, &wc);
    line_obj["word_count"] = wc;
    line_obj["words"] = json::array();

    for (__int64 j = 0; j < wc; j++) {
      __int64 word = 0;
      pGetOcrWord(line, j, &word);
      if (!word) continue;

      __int64 word_content = 0;
      pGetOcrWordContent(word, &word_content);
      char *wcs = reinterpret_cast<char *>(word_content);

      __int64 word_bbox_ptr = 0;
      pGetOcrWordBoundingBox(word, &word_bbox_ptr);
      OcrBBox *wb = reinterpret_cast<OcrBBox *>(word_bbox_ptr);

      json word_obj;
      word_obj["index"] = j;
      word_obj["text"] = wcs ? std::string(wcs) : "";
      if (wb) {
        word_obj["bounding_box"] = {
          wb->x1, wb->y1, wb->x2, wb->y2,
          wb->x3, wb->y3, wb->x4, wb->y4
        };
      }

      // Word confidence
      if (pGetOcrWordConfidence) {
        float conf = 0.0f;
        if (pGetOcrWordConfidence(word, &conf) == 0) {
          word_obj["confidence"] = conf;
        }
      }

      line_obj["words"].push_back(word_obj);
    }
    result["lines"].push_back(line_obj);
  }

  return result;
}

// ---------- exported functions ----------

WRAPPER_EXPORT int initModel(const wchar_t *model_dir) {
  if (g_hDLL) return 0; // already initialized

  // Build DLL path: model_dir\oneocr.dll
  std::wstring dll_path = std::wstring(model_dir) + L"\\oneocr.dll";
  g_hDLL = LoadLibraryW(dll_path.c_str());
  if (!g_hDLL) return OCR_ERR_LOAD_DLL;

  // Resolve all function pointers (required)
  pCreateOcrInitOptions = (CreateOcrInitOptions_t)GetProcAddress(g_hDLL, "CreateOcrInitOptions");
  pOcrInitOptionsSetUseModelDelayLoad = (OcrInitOptionsSetUseModelDelayLoad_t)GetProcAddress(g_hDLL, "OcrInitOptionsSetUseModelDelayLoad");
  pCreateOcrPipeline = (CreateOcrPipeline_t)GetProcAddress(g_hDLL, "CreateOcrPipeline");
  pCreateOcrProcessOptions = (CreateOcrProcessOptions_t)GetProcAddress(g_hDLL, "CreateOcrProcessOptions");
  pOcrProcessOptionsSetMaxRecognitionLineCount = (OcrProcessOptionsSetMaxRecognitionLineCount_t)GetProcAddress(g_hDLL, "OcrProcessOptionsSetMaxRecognitionLineCount");
  pRunOcrPipeline = (RunOcrPipeline_t)GetProcAddress(g_hDLL, "RunOcrPipeline");
  pGetOcrLineCount = (GetOcrLineCount_t)GetProcAddress(g_hDLL, "GetOcrLineCount");
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
    clear_function_pointers();
    return OCR_ERR_RESOLVE_FUNC;
  }

  // Resolve optional function pointers (may not exist in all DLL versions)
  pOcrProcessOptionsGetMaxRecognitionLineCount = (OcrProcessOptionsGetMaxRecognitionLineCount_t)GetProcAddress(g_hDLL, "OcrProcessOptionsGetMaxRecognitionLineCount");
  pOcrProcessOptionsSetResizeResolution = (OcrProcessOptionsSetResizeResolution_t)GetProcAddress(g_hDLL, "OcrProcessOptionsSetResizeResolution");
  pOcrProcessOptionsGetResizeResolution = (OcrProcessOptionsGetResizeResolution_t)GetProcAddress(g_hDLL, "OcrProcessOptionsGetResizeResolution");
  pGetImageAngle = (GetImageAngle_t)GetProcAddress(g_hDLL, "GetImageAngle");
  pGetOcrLineStyle = (GetOcrLineStyle_t)GetProcAddress(g_hDLL, "GetOcrLineStyle");
  pGetOcrWordConfidence = (GetOcrWordConfidence_t)GetProcAddress(g_hDLL, "GetOcrWordConfidence");
  pReleaseOcrResult = (ReleaseOcrResult_t)GetProcAddress(g_hDLL, "ReleaseOcrResult");
  pReleaseOcrInitOptions = (ReleaseOcrInitOptions_t)GetProcAddress(g_hDLL, "ReleaseOcrInitOptions");
  pReleaseOcrPipeline = (ReleaseOcrPipeline_t)GetProcAddress(g_hDLL, "ReleaseOcrPipeline");
  pReleaseOcrProcessOptions = (ReleaseOcrProcessOptions_t)GetProcAddress(g_hDLL, "ReleaseOcrProcessOptions");

  // Initialize OCR pipeline
  __int64 ctx = 0;
  __int64 res = pCreateOcrInitOptions(&ctx);
  if (res != 0) { FreeLibrary(g_hDLL); g_hDLL = NULL; clear_function_pointers(); return OCR_ERR_INIT_OPTIONS; }
  g_initOptions = ctx;

  res = pOcrInitOptionsSetUseModelDelayLoad(ctx, 0);
  if (res != 0) { FreeLibrary(g_hDLL); g_hDLL = NULL; clear_function_pointers(); return OCR_ERR_DELAY_LOAD; }

  const char *key = {"kj)TGtrK>f]b[Piow.gU+nC@s\"\"\"\"\"\"4"};

  // Build model path: model_dir\oneocr.onemodel
  std::wstring model_wpath = std::wstring(model_dir) + L"\\oneocr.onemodel";
  std::string model_path = wchar_to_utf8(model_wpath.c_str());

  res = pCreateOcrPipeline((__int64)model_path.c_str(), (__int64)key, ctx, &g_pipeline);
  if (res != 0) {
    g_pipeline = 0;
    FreeLibrary(g_hDLL);
    g_hDLL = NULL;
    clear_function_pointers();
    return OCR_ERR_CREATE_PIPE;
  }

  return OCR_OK;
}

WRAPPER_EXPORT int releaseModel() {
  if (g_hDLL) {
    if (pReleaseOcrPipeline && g_pipeline) pReleaseOcrPipeline(g_pipeline);
    if (pReleaseOcrInitOptions && g_initOptions) pReleaseOcrInitOptions(g_initOptions);
    FreeLibrary(g_hDLL);
    g_hDLL = NULL;
  }
  g_pipeline = 0;
  g_initOptions = 0;
  clear_function_pointers();
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
  unsigned char *bgra = to_bgra(data, width, height, channels);
  stbi_image_free(data);
  if (!bgra) return (channels != 1 && channels != 3 && channels != 4)
                        ? OCR_ERR_UNSUPPORTED : OCR_ERR_ALLOC_BGRA;

  __int64 step = (__int64)width * 4;
  OcrImg img = {.t = 3, .col = width, .row = height, ._unk = 0,
                .step = step, .data_ptr = (__int64)bgra};

  // Create process options
  __int64 opt = 0;
  __int64 res = pCreateOcrProcessOptions(&opt);
  if (res != 0) { free(bgra); return OCR_ERR_PROC_OPTIONS; }

  res = pOcrProcessOptionsSetMaxRecognitionLineCount(opt, 1000);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    free(bgra);
    return OCR_ERR_MAX_LINES;
  }

  // Run OCR
  __int64 instance = 0;
  res = pRunOcrPipeline(g_pipeline, &img, opt, &instance);
  free(bgra);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    return OCR_ERR_RUN_PIPELINE;
  }

  // Build result JSON with all features
  json result = build_result_json(path_u8.c_str(), width, height, step, instance);

  // Release OCR result and process options
  if (pReleaseOcrResult) pReleaseOcrResult(instance);
  if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);

  // Serialize JSON to UTF-8 string
  std::string json_str = result.dump(2);
  size_t json_len = json_str.size() + 1;

  utf8_json = (char *)func(json_len);
  if (!utf8_json) return OCR_ERR_ALLOC_JSON;

  memcpy(utf8_json, json_str.c_str(), json_len);
  return OCR_OK;
}

// ocrImageEx: OCR with configurable max_lines and resize_resolution
WRAPPER_EXPORT int ocrImageEx(const wchar_t *image_path, char *&utf8_json, ALLOC_FUNC func,
                              int max_lines, int resize_w, int resize_h) {
  if (!g_hDLL || !g_pipeline) return OCR_ERR_NOT_INIT;
  if (!image_path || !func) return OCR_ERR_INVALID_PARAM;

  std::string path_u8 = wchar_to_utf8(image_path);

  int width = 0, height = 0, channels = 0;
  unsigned char *data = nullptr;
  FILE *f = _wfopen(image_path, L"rb");
  if (!f) return OCR_ERR_LOAD_IMAGE;
  data = stbi_load_from_file(f, &width, &height, &channels, 0);
  fclose(f);
  if (!data) return OCR_ERR_LOAD_IMAGE;

  unsigned char *bgra = to_bgra(data, width, height, channels);
  stbi_image_free(data);
  if (!bgra) return (channels != 1 && channels != 3 && channels != 4)
                        ? OCR_ERR_UNSUPPORTED : OCR_ERR_ALLOC_BGRA;

  __int64 step = (__int64)width * 4;
  OcrImg img = {.t = 3, .col = width, .row = height, ._unk = 0,
                .step = step, .data_ptr = (__int64)bgra};

  __int64 opt = 0;
  __int64 res = pCreateOcrProcessOptions(&opt);
  if (res != 0) { free(bgra); return OCR_ERR_PROC_OPTIONS; }

  int ml = (max_lines > 0 && max_lines <= 1000) ? max_lines : 1000;
  res = pOcrProcessOptionsSetMaxRecognitionLineCount(opt, ml);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    free(bgra);
    return OCR_ERR_MAX_LINES;
  }

  if (resize_w > 0 && resize_h > 0 && pOcrProcessOptionsSetResizeResolution) {
    res = pOcrProcessOptionsSetResizeResolution(opt, resize_w, resize_h);
    if (res != 0) {
      if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
      free(bgra);
      return OCR_ERR_SET_RESIZE;
    }
  }

  __int64 instance = 0;
  res = pRunOcrPipeline(g_pipeline, &img, opt, &instance);
  free(bgra);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    return OCR_ERR_RUN_PIPELINE;
  }

  json result = build_result_json(path_u8.c_str(), width, height, step, instance);

  if (pReleaseOcrResult) pReleaseOcrResult(instance);
  if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);

  std::string json_str = result.dump(2);
  size_t json_len = json_str.size() + 1;

  utf8_json = (char *)func(json_len);
  if (!utf8_json) return OCR_ERR_ALLOC_JSON;

  memcpy(utf8_json, json_str.c_str(), json_len);
  return OCR_OK;
}

// ocrImageRaw: OCR on raw pixel buffer (no file I/O)
// pixel_data must be BGRA format, step = width * 4
WRAPPER_EXPORT int ocrImageRaw(const unsigned char *pixel_data, int width, int height,
                               __int64 step, char *&utf8_json, ALLOC_FUNC func) {
  if (!g_hDLL || !g_pipeline) return OCR_ERR_NOT_INIT;
  if (!pixel_data || !func || width <= 0 || height <= 0) return OCR_ERR_INVALID_PARAM;

  OcrImg img = {.t = 3, .col = width, .row = height, ._unk = 0,
                .step = step, .data_ptr = (__int64)pixel_data};

  __int64 opt = 0;
  __int64 res = pCreateOcrProcessOptions(&opt);
  if (res != 0) return OCR_ERR_PROC_OPTIONS;

  res = pOcrProcessOptionsSetMaxRecognitionLineCount(opt, 1000);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    return OCR_ERR_MAX_LINES;
  }

  __int64 instance = 0;
  res = pRunOcrPipeline(g_pipeline, &img, opt, &instance);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    return OCR_ERR_RUN_PIPELINE;
  }

  json result = build_result_json("<raw_buffer>", width, height, step, instance);

  if (pReleaseOcrResult) pReleaseOcrResult(instance);
  if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);

  std::string json_str = result.dump(2);
  size_t json_len = json_str.size() + 1;

  utf8_json = (char *)func(json_len);
  if (!utf8_json) return OCR_ERR_ALLOC_JSON;

  memcpy(utf8_json, json_str.c_str(), json_len);
  return OCR_OK;
}
