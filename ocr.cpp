#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

#include <Windows.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
  __int32 t;
  __int32 col;
  __int32 row;
  __int32 _unk;
  __int64 step;
  __int64 data_ptr;
} Img;

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
typedef __int64(__cdecl *RunOcrPipeline_t)(__int64, Img *, __int64, __int64 *);
typedef __int64(__cdecl *CreateOcrProcessOptions_t)(__int64 *);
typedef __int64(__cdecl *OcrInitOptionsSetUseModelDelayLoad_t)(__int64, char);
typedef __int64(__cdecl *CreateOcrPipeline_t)(__int64, __int64, __int64,
                                              __int64 *);

void ocr(Img img) {
  HINSTANCE hDLL = LoadLibraryA("oneocr.dll");
  if (hDLL == NULL) {
    std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
    return;
  }
  // Get function pointers
  CreateOcrInitOptions_t CreateOcrInitOptions =
      (CreateOcrInitOptions_t)GetProcAddress(hDLL, "CreateOcrInitOptions");
  GetOcrLineCount_t GetOcrLineCount =
      (GetOcrLineCount_t)GetProcAddress(hDLL, "GetOcrLineCount");
  CreateOcrProcessOptions_t CreateOcrProcessOptions =
      (CreateOcrProcessOptions_t)GetProcAddress(hDLL,
                                                "CreateOcrProcessOptions");
  CreateOcrPipeline_t CreateOcrPipeline =
      (CreateOcrPipeline_t)GetProcAddress(hDLL, "CreateOcrPipeline");
  OcrInitOptionsSetUseModelDelayLoad_t OcrInitOptionsSetUseModelDelayLoad =
      (OcrInitOptionsSetUseModelDelayLoad_t)GetProcAddress(
          hDLL, "OcrInitOptionsSetUseModelDelayLoad");
  OcrProcessOptionsSetMaxRecognitionLineCount_t
      OcrProcessOptionsSetMaxRecognitionLineCount =
          (OcrProcessOptionsSetMaxRecognitionLineCount_t)GetProcAddress(
              hDLL, "OcrProcessOptionsSetMaxRecognitionLineCount");
  RunOcrPipeline_t RunOcrPipeline =
      (RunOcrPipeline_t)GetProcAddress(hDLL, "RunOcrPipeline");
  GetOcrLine_t GetOcrLine = (GetOcrLine_t)GetProcAddress(hDLL, "GetOcrLine");
  GetOcrLineContent_t GetOcrLineContent =
      (GetOcrLineContent_t)GetProcAddress(hDLL, "GetOcrLineContent");
  GetOcrLineBoundingBox_t GetOcrLineBoundingBox =
      (GetOcrLineBoundingBox_t)GetProcAddress(hDLL, "GetOcrLineBoundingBox");
  GetOcrLineWordCount_t GetOcrLineWordCount =
      (GetOcrLineWordCount_t)GetProcAddress(hDLL, "GetOcrLineWordCount");
  GetOcrWord_t GetOcrWord = (GetOcrWord_t)GetProcAddress(hDLL, "GetOcrWord");
  GetOcrWordContent_t GetOcrWordContent =
      (GetOcrWordContent_t)GetProcAddress(hDLL, "GetOcrWordContent");
  GetOcrWordBoundingBox_t GetOcrWordBoundingBox =
      (GetOcrWordBoundingBox_t)GetProcAddress(hDLL, "GetOcrWordBoundingBox");
  __int64 ctx = 0;
  __int64 pipeline = 0;
  __int64 opt = 0;
  __int64 instance = 0;
  __int64 res = CreateOcrInitOptions(&ctx);
  assert(res == 0);
  res = OcrInitOptionsSetUseModelDelayLoad(ctx, 0);
  assert(res == 0);
  // key: kj)TGtrK>f]b[Piow.gU+nC@s""""""4
  const char *key = {"kj)TGtrK>f]b[Piow.gU+nC@s\"\"\"\"\"\"4"};
  res = CreateOcrPipeline((__int64)"oneocr.onemodel", (__int64)key, ctx,
                          &pipeline);
  printf("OCR model loaded...\n");
  // printf("res: %lld, ctx: 0x%llx, pipeline: 0x%llx\n", res, ctx, pipeline);
  // The key is for the AI model, if key is not right, CreateOcrPipeline will
  // return 6 with error message: Crypto.cpp:78 Check failed: meta->magic_number
  // == MAGIC_NUMBER (0 vs. 1) Unable to uncompress. Source data mismatch.
  res = CreateOcrProcessOptions(&opt);
  assert(res == 0);
  res = OcrProcessOptionsSetMaxRecognitionLineCount(opt, 1000);
  assert(res == 0);
#ifdef DEBUG
  __int16 *ibs = reinterpret_cast<__int16 *>(&img);
  for (int i = 0; i < 8; i++) {
    printf("%02x ", ibs[i]);
  }
  printf("\n");
#endif
  assert(sizeof(img) == 0x20);
  res = RunOcrPipeline(pipeline, &img, opt, &instance);
  assert(res == 0);
  printf("Running ocr pipeline...\n");
#ifdef DEBUG  
  printf("\t ctx: 0x%llx, pipeline: 0x%llx, opt: 0x%llx, instance: "
         "0x%llx\n",
         ctx, pipeline, opt, instance);
#endif
  __int64 lc;
  res = GetOcrLineCount(instance, &lc);
  assert(res == 0);
  printf("Recognize %lld lines\n", lc);
  for (__int64 lci = 0; lci < lc; lci++) {
    __int64 line = 0;
    __int64 v106 = 0;
    GetOcrLine(instance, lci, &line);
    if (!line) {
      continue;
    }
    __int64 line_content = 0;
    GetOcrLineContent(line, &line_content);
    char *lcs = reinterpret_cast<char *>(line_content);
    printf("%02lld: %s\n", lci, lcs);
    // GetOcrLineBoundingBox(line, &v106);
    __int64 lr = 0;
    GetOcrLineWordCount(line, &lr);
    for (__int64 j = 0; j < lr; j++) {
      __int64 v105 = 0;
      __int64 v107 = 0;
      __int64 lpMultiByteStr = 0;
      GetOcrWord(line, j, &v105);
      GetOcrWordContent(v105, &lpMultiByteStr);
      GetOcrWordBoundingBox(v105, &v107);
    }
  }
}

int main(int argc, char *argv[]) {
  //  char* file_name = "a1.png";
  char *file_name = NULL;
  if (argc > 1) {
    file_name = argv[1];
  } else {
    printf("Usage: ocr.exe <abc.png>\n");
    return 0;
  }

  int width = 0, height = 0, channels = 0;
  unsigned char *data = stbi_load(file_name, &width, &height, &channels, 0);
  if (!data) {
    printf("can't read image: %s\n", file_name);
    return -1;
  }
  // Convert to BGRA (4 channels)
  unsigned char *bgra = NULL;
  if (channels == 3) {
    // RGB -> BGRA
    bgra = (unsigned char *)malloc(width * height * 4);
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i * 3 + 2]; // B
      bgra[i * 4 + 1] = data[i * 3 + 1]; // G
      bgra[i * 4 + 2] = data[i * 3 + 0]; // R
      bgra[i * 4 + 3] = 255;             // A
    }
  } else if (channels == 4) {
    // RGBA -> BGRA
    bgra = (unsigned char *)malloc(width * height * 4);
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i * 4 + 2]; // B
      bgra[i * 4 + 1] = data[i * 4 + 1]; // G
      bgra[i * 4 + 2] = data[i * 4 + 0]; // R
      bgra[i * 4 + 3] = data[i * 4 + 3]; // A
    }
  } else if (channels == 1) {
    // Gray -> BGRA
    bgra = (unsigned char *)malloc(width * height * 4);
    for (int i = 0; i < width * height; i++) {
      bgra[i * 4 + 0] = data[i]; // B
      bgra[i * 4 + 1] = data[i]; // G
      bgra[i * 4 + 2] = data[i]; // R
      bgra[i * 4 + 3] = 255;     // A
    }
  } else {
    printf("image type not support (channels=%d)\n", channels);
    stbi_image_free(data);
    return -1;
  }
  stbi_image_free(data);

  int rows = height;
  int cols = width;
  size_t step = (size_t)cols * 4;

  Img ig = {.t = 3,
            .col = cols,
            .row = rows,
            ._unk = 0,
            .step = (__int64)step,
            .data_ptr = (__int64)reinterpret_cast<char *>(bgra)};

  ocr(ig);
  free(bgra);
  return 0;
}
