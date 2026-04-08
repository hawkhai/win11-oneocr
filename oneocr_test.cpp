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

  // --- Test 1: basic ocrImage ---
  printf("\n=== Test 1: ocrImage (basic) ===\n");
  std::string json;
  ret = ocr.ocrImage(image_path, json);
  if (ret != OCR_OK) {
    printf("ocrImage failed: %d\n", ret);
    return 1;
  }

  const char *out_file = "oneocr_test_result.json";
  std::ofstream ofs(out_file, std::ios::binary);
  ofs << json;
  ofs.close();
  printf("OCR result saved to %s\n", out_file);

  // Print preview
  printf("--- JSON preview (first 800 chars) ---\n");
  printf("%.800s\n", json.c_str());
  if (json.size() > 800) {
    printf("... (%zu bytes total)\n", json.size());
  }

  // --- Test 2: ocrImageEx with custom options ---
  printf("\n=== Test 2: ocrImageEx (max_lines=50, resize=800x600) ===\n");
  std::string json_ex;
  ret = ocr.ocrImageEx(image_path, json_ex, 50, 800, 600);
  if (ret != OCR_OK) {
    printf("ocrImageEx failed: %d (may not be supported in this DLL version)\n", ret);
    // Not fatal: the DLL version may not support SetResizeResolution
  } else {
    const char *out_file_ex = "oneocr_test_result_ex.json";
    std::ofstream ofs_ex(out_file_ex, std::ios::binary);
    ofs_ex << json_ex;
    ofs_ex.close();
    printf("Extended result saved to %s\n", out_file_ex);
  }

  return 0;
}
