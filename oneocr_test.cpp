#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "oneocr.h"

int main(int argc, char *argv[]) {
  const wchar_t *image_path = L"test.png";

  // OneOcr: header-only, no .lib needed.
  // Pass nullptr to load oneocr_wrapper.dll from current directory.
  OneOcr ocr;
  if (!ocr.isOk()) {
    printf("Failed to load oneocr_wrapper.dll\n");
    return 1;
  }

  // initModel: pass the directory containing oneocr.dll + oneocr.onemodel
  // Here we use "." since bin files are copied to the exe directory.
  int ret = ocr.initModel(L".");
  if (ret != OCR_OK) {
    printf("initModel failed: %d\n", ret);
    return 1;
  }
  printf("Model initialized.\n");

  // ocrImage: result returned as std::string, no manual free needed
  std::string json;
  ret = ocr.ocrImage(image_path, json);
  if (ret != OCR_OK) {
    printf("ocrImage failed: %d\n", ret);
    return 1;
  }

  // Save JSON to file (UTF-8)
  const char *out_file = "oneocr_test_result.json";
  std::ofstream ofs(out_file, std::ios::binary);
  ofs << json;
  ofs.close();
  printf("OCR result saved to %s\n", out_file);

  // Print first 500 chars as preview
  printf("--- JSON preview (first 500 chars) ---\n");
  printf("%.500s\n", json.c_str());
  if (json.size() > 500) {
    printf("... (%zu bytes total)\n", json.size());
  }

  return 0;
}
