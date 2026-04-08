// ocr.cpp - Full-featured OneOCR CLI
//
// Usage:
//   ocr.exe <image1.png> [image2.jpg ...] [options]
//
// Options:
//   --text, -t           Output plain text only (no JSON)
//   --output, -o <file>  Write JSON to specified file (default: <image>.json)
//   --max-lines <n>      Max recognition lines (1-1000, default 1000)
//   --resize <WxH>       Max internal resize resolution (e.g. 1152x768)
//   --quiet, -q          Suppress progress messages
//   --help, -h           Show this help

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <Windows.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "json.hpp"
using json = nlohmann::json;

// ---------- oneocr.dll types ----------

typedef struct {
  __int32 t;
  __int32 col;
  __int32 row;
  __int32 _unk;
  __int64 step;
  __int64 data_ptr;
} Img;

typedef struct {
  float x1, y1;
  float x2, y2;
  float x3, y3;
  float x4, y4;
} BBox8;

typedef __int64(__cdecl *CreateOcrInitOptions_t)(__int64 *);
typedef __int64(__cdecl *OcrInitOptionsSetUseModelDelayLoad_t)(__int64, char);
typedef __int64(__cdecl *CreateOcrPipeline_t)(__int64, __int64, __int64, __int64 *);
typedef __int64(__cdecl *CreateOcrProcessOptions_t)(__int64 *);
typedef __int64(__cdecl *OcrProcessOptionsSetMaxRecognitionLineCount_t)(__int64, __int64);
typedef __int64(__cdecl *OcrProcessOptionsSetResizeResolution_t)(__int64, __int32, __int32);
typedef __int64(__cdecl *RunOcrPipeline_t)(__int64, Img *, __int64, __int64 *);
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

// ---------- global function pointers ----------

static CreateOcrInitOptions_t                    pCreateOcrInitOptions;
static OcrInitOptionsSetUseModelDelayLoad_t      pOcrInitOptionsSetUseModelDelayLoad;
static CreateOcrPipeline_t                       pCreateOcrPipeline;
static CreateOcrProcessOptions_t                 pCreateOcrProcessOptions;
static OcrProcessOptionsSetMaxRecognitionLineCount_t pOcrProcessOptionsSetMaxRecognitionLineCount;
static OcrProcessOptionsSetResizeResolution_t    pOcrProcessOptionsSetResizeResolution;
static RunOcrPipeline_t                          pRunOcrPipeline;
static GetImageAngle_t                           pGetImageAngle;
static GetOcrLineCount_t                         pGetOcrLineCount;
static GetOcrLine_t                              pGetOcrLine;
static GetOcrLineContent_t                       pGetOcrLineContent;
static GetOcrLineBoundingBox_t                   pGetOcrLineBoundingBox;
static GetOcrLineStyle_t                         pGetOcrLineStyle;
static GetOcrLineWordCount_t                     pGetOcrLineWordCount;
static GetOcrWord_t                              pGetOcrWord;
static GetOcrWordContent_t                       pGetOcrWordContent;
static GetOcrWordBoundingBox_t                   pGetOcrWordBoundingBox;
static GetOcrWordConfidence_t                    pGetOcrWordConfidence;
static ReleaseOcrResult_t                        pReleaseOcrResult;
static ReleaseOcrInitOptions_t                   pReleaseOcrInitOptions;
static ReleaseOcrPipeline_t                      pReleaseOcrPipeline;
static ReleaseOcrProcessOptions_t                pReleaseOcrProcessOptions;

// ---------- helpers ----------

// Convert image to BGRA; returns nullptr on unsupported channels
static unsigned char *to_bgra(const unsigned char *data, int width, int height, int channels) {
  unsigned char *bgra = (unsigned char *)malloc((size_t)width * height * 4);
  if (!bgra) return nullptr;

  if (channels == 3) {
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i * 3 + 2];
      bgra[i * 4 + 1] = data[i * 3 + 1];
      bgra[i * 4 + 2] = data[i * 3 + 0];
      bgra[i * 4 + 3] = 255;
    }
  } else if (channels == 4) {
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i * 4 + 2];
      bgra[i * 4 + 1] = data[i * 4 + 1];
      bgra[i * 4 + 2] = data[i * 4 + 0];
      bgra[i * 4 + 3] = data[i * 4 + 3];
    }
  } else if (channels == 1) {
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i];
      bgra[i * 4 + 1] = data[i];
      bgra[i * 4 + 2] = data[i];
      bgra[i * 4 + 3] = 255;
    }
  } else {
    free(bgra);
    return nullptr;
  }
  return bgra;
}

static void show_help() {
  printf(
    "OneOCR - Windows 11 Snipping Tool OCR Engine CLI\n"
    "\n"
    "Usage: ocr.exe <image1.png> [image2.jpg ...] [options]\n"
    "\n"
    "Options:\n"
    "  --text, -t           Output plain text only (no JSON)\n"
    "  --output, -o <file>  Write JSON to specified file (default: <image>.json)\n"
    "  --max-lines <n>      Max recognition lines (1-1000, default 1000)\n"
    "  --resize <WxH>       Max internal resize resolution (e.g. 1152x768)\n"
    "  --quiet, -q          Suppress progress messages\n"
    "  --help, -h           Show this help\n"
    "\n"
    "Features:\n"
    "  - 8-point bounding boxes (4 corners)\n"
    "  - Word-level confidence scores\n"
    "  - Image angle detection\n"
    "  - Handwritten vs. printed text classification\n"
    "  - Multi-image batch processing\n"
    "  - Configurable resize resolution\n"
    "\n"
    "Requires: oneocr.dll, oneocr.onemodel, onnxruntime.dll in same directory.\n"
  );
}

// ---------- DLL loading ----------

static HINSTANCE g_hDLL = NULL;
static __int64 g_pipeline = 0;
static __int64 g_initOptions = 0;

static bool load_dll() {
  g_hDLL = LoadLibraryW(L"oneocr.dll");
  if (!g_hDLL) {
    fprintf(stderr, "ERROR: Failed to load oneocr.dll (code %lu)\n", GetLastError());
    return false;
  }

  // Required
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
      !pOcrInitOptionsSetUseModelDelayLoad || !pOcrProcessOptionsSetMaxRecognitionLineCount ||
      !pRunOcrPipeline || !pGetOcrLineCount || !pGetOcrLine || !pGetOcrLineContent ||
      !pGetOcrLineBoundingBox || !pGetOcrLineWordCount || !pGetOcrWord ||
      !pGetOcrWordContent || !pGetOcrWordBoundingBox) {
    fprintf(stderr, "ERROR: Failed to resolve required DLL functions\n");
    FreeLibrary(g_hDLL);
    g_hDLL = NULL;
    return false;
  }

  // Optional (may not exist in all DLL versions)
  pOcrProcessOptionsSetResizeResolution = (OcrProcessOptionsSetResizeResolution_t)GetProcAddress(g_hDLL, "OcrProcessOptionsSetResizeResolution");
  pGetImageAngle = (GetImageAngle_t)GetProcAddress(g_hDLL, "GetImageAngle");
  pGetOcrLineStyle = (GetOcrLineStyle_t)GetProcAddress(g_hDLL, "GetOcrLineStyle");
  pGetOcrWordConfidence = (GetOcrWordConfidence_t)GetProcAddress(g_hDLL, "GetOcrWordConfidence");
  pReleaseOcrResult = (ReleaseOcrResult_t)GetProcAddress(g_hDLL, "ReleaseOcrResult");
  pReleaseOcrInitOptions = (ReleaseOcrInitOptions_t)GetProcAddress(g_hDLL, "ReleaseOcrInitOptions");
  pReleaseOcrPipeline = (ReleaseOcrPipeline_t)GetProcAddress(g_hDLL, "ReleaseOcrPipeline");
  pReleaseOcrProcessOptions = (ReleaseOcrProcessOptions_t)GetProcAddress(g_hDLL, "ReleaseOcrProcessOptions");

  return true;
}

static bool init_pipeline() {
  __int64 ctx = 0;
  if (pCreateOcrInitOptions(&ctx) != 0) {
    fprintf(stderr, "ERROR: CreateOcrInitOptions failed\n");
    return false;
  }
  g_initOptions = ctx;

  if (pOcrInitOptionsSetUseModelDelayLoad(ctx, 0) != 0) {
    fprintf(stderr, "ERROR: OcrInitOptionsSetUseModelDelayLoad failed\n");
    return false;
  }

  const char *key = {"kj)TGtrK>f]b[Piow.gU+nC@s\"\"\"\"\"\"4"};
  if (pCreateOcrPipeline((__int64)"oneocr.onemodel", (__int64)key, ctx, &g_pipeline) != 0) {
    fprintf(stderr, "ERROR: CreateOcrPipeline failed (check model file and key)\n");
    return false;
  }

  return true;
}

static void cleanup() {
  if (g_hDLL) {
    if (pReleaseOcrPipeline && g_pipeline) pReleaseOcrPipeline(g_pipeline);
    if (pReleaseOcrInitOptions && g_initOptions) pReleaseOcrInitOptions(g_initOptions);
    FreeLibrary(g_hDLL);
  }
}

// ---------- OCR processing ----------

struct OcrConfig {
  int max_lines = 1000;
  int resize_w = 0;
  int resize_h = 0;
  bool text_only = false;
  bool quiet = false;
  std::string output_file; // empty = auto (<image>.json)
};

// Run OCR on one image, return JSON result
static json ocr_one(const char *input_file, const OcrConfig &cfg) {
  int width = 0, height = 0, channels = 0;
  unsigned char *data = stbi_load(input_file, &width, &height, &channels, 0);
  if (!data) {
    fprintf(stderr, "ERROR: can't read image: %s\n", input_file);
    return json();
  }

  unsigned char *bgra = to_bgra(data, width, height, channels);
  stbi_image_free(data);
  if (!bgra) {
    fprintf(stderr, "ERROR: unsupported image format (channels=%d): %s\n", channels, input_file);
    return json();
  }

  __int64 step = (__int64)width * 4;
  Img ig = {.t = 3, .col = width, .row = height, ._unk = 0,
            .step = step, .data_ptr = (__int64)bgra};

  // Process options
  __int64 opt = 0;
  if (pCreateOcrProcessOptions(&opt) != 0) {
    free(bgra);
    fprintf(stderr, "ERROR: CreateOcrProcessOptions failed\n");
    return json();
  }

  int ml = (cfg.max_lines > 0 && cfg.max_lines <= 1000) ? cfg.max_lines : 1000;
  pOcrProcessOptionsSetMaxRecognitionLineCount(opt, ml);

  if (cfg.resize_w > 0 && cfg.resize_h > 0 && pOcrProcessOptionsSetResizeResolution) {
    pOcrProcessOptionsSetResizeResolution(opt, cfg.resize_w, cfg.resize_h);
  }

  // Run pipeline
  __int64 instance = 0;
  __int64 res = pRunOcrPipeline(g_pipeline, &ig, opt, &instance);
  free(bgra);
  if (res != 0) {
    if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);
    fprintf(stderr, "ERROR: RunOcrPipeline failed (code %lld): %s\n", res, input_file);
    return json();
  }

  // Build JSON result
  json result;
  result["file"] = std::string(input_file);
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
    BBox8 *lb = reinterpret_cast<BBox8 *>(line_bbox);

    json line_obj;
    line_obj["index"] = lci;
    line_obj["text"] = lcs ? std::string(lcs) : "";
    if (lb) {
      line_obj["bounding_box"] = {
        lb->x1, lb->y1, lb->x2, lb->y2,
        lb->x3, lb->y3, lb->x4, lb->y4
      };
    }

    // Line style: GetOcrLineStyle returns (style, handwritten_confidence)
    //   style: 0 = handwritten, 1 = printed
    //   handwritten_confidence: 0.0 = definitely printed, 1.0 = definitely handwritten
    if (pGetOcrLineStyle) {
      __int32 style = -1;
      float hw_conf = 0.0f;
      if (pGetOcrLineStyle(line, &style, &hw_conf) == 0) {
        line_obj["style"] = {
          {"type", style == 0 ? "handwritten" : "printed"},
          {"confidence", hw_conf}
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
      BBox8 *wb = reinterpret_cast<BBox8 *>(word_bbox_ptr);

      json word_obj;
      word_obj["index"] = j;
      word_obj["text"] = wcs ? std::string(wcs) : "";
      if (wb) {
        word_obj["bounding_box"] = {
          wb->x1, wb->y1, wb->x2, wb->y2,
          wb->x3, wb->y3, wb->x4, wb->y4
        };
      }
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

  // Release resources
  if (pReleaseOcrResult) pReleaseOcrResult(instance);
  if (pReleaseOcrProcessOptions) pReleaseOcrProcessOptions(opt);

  return result;
}

// ---------- main ----------

int main(int argc, char *argv[]) {
  if (argc < 2) {
    show_help();
    return 0;
  }

  // Parse arguments
  OcrConfig cfg;
  std::vector<std::string> image_files;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      show_help();
      return 0;
    } else if (arg == "--text" || arg == "-t") {
      cfg.text_only = true;
    } else if (arg == "--quiet" || arg == "-q") {
      cfg.quiet = true;
    } else if ((arg == "--output" || arg == "-o") && i + 1 < argc) {
      cfg.output_file = argv[++i];
    } else if (arg == "--max-lines" && i + 1 < argc) {
      cfg.max_lines = atoi(argv[++i]);
    } else if (arg == "--resize" && i + 1 < argc) {
      // Parse WxH format
      std::string val = argv[++i];
      size_t xpos = val.find('x');
      if (xpos == std::string::npos) xpos = val.find('X');
      if (xpos != std::string::npos) {
        cfg.resize_w = atoi(val.substr(0, xpos).c_str());
        cfg.resize_h = atoi(val.substr(xpos + 1).c_str());
      }
    } else if (arg[0] != '-') {
      image_files.push_back(arg);
    } else {
      fprintf(stderr, "WARNING: unknown option: %s\n", arg.c_str());
    }
  }

  if (image_files.empty()) {
    fprintf(stderr, "ERROR: no image files specified\n");
    show_help();
    return 1;
  }

  // Load DLL and initialize pipeline
  if (!load_dll()) return 1;
  if (!init_pipeline()) { cleanup(); return 1; }
  if (!cfg.quiet) printf("OCR model loaded.\n");

  // Process each image
  for (size_t fi = 0; fi < image_files.size(); fi++) {
    const std::string &input = image_files[fi];
    if (!cfg.quiet) printf("Processing: %s\n", input.c_str());

    json result = ocr_one(input.c_str(), cfg);
    if (result.is_null()) continue;

    if (cfg.text_only) {
      // Plain text output (like oneocr-cli)
      for (auto &line : result["lines"]) {
        printf("%s\n", line["text"].get<std::string>().c_str());
      }
    } else {
      // JSON output
      std::string json_out;
      if (!cfg.output_file.empty() && image_files.size() == 1) {
        json_out = cfg.output_file;
      } else {
        json_out = input + ".json";
      }
      std::ofstream ofs(json_out, std::ios::binary);
      ofs << result.dump(2);
      ofs.close();
      if (!cfg.quiet) {
        __int64 lc = result["line_count"].get<__int64>();
        printf("  -> %lld lines, saved to %s\n", lc, json_out.c_str());
      }
    }
  }

  cleanup();
  return 0;
}
