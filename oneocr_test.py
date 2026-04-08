"""
oneocr_test.py - Test oneocr_wrapper.dll directly via ctypes.

Logic mirrors oneocr_test.cpp:
  1. Load oneocr_wrapper.dll
  2. initModel(".")
  3. Test 1: ocrImage(test.png) -> oneocr_test_result.json
  4. Test 2: ocrImageEx(test.png, max_lines=50, resize=800x600) -> oneocr_test_result_ex.json
  5. releaseModel()
"""

import ctypes
import json
import os
import sys

# ---------- error codes (keep in sync with oneocr.h) ----------
OCR_OK = 0

# ---------- types ----------

# ALLOC_FUNC: void* (*)(size_t)
ALLOC_FUNC = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_size_t)

# ctypes malloc wrapper — allocate via libc malloc, return as c_void_p
_crt = ctypes.cdll.msvcrt
_crt.malloc.argtypes = [ctypes.c_size_t]
_crt.malloc.restype = ctypes.c_void_p
_crt.free.argtypes = [ctypes.c_void_p]
_crt.free.restype = None


def _malloc_cb(size):
    return _crt.malloc(size)


alloc_func = ALLOC_FUNC(_malloc_cb)


def load_wrapper(dll_dir=None):
    """Load oneocr_wrapper.dll from dll_dir (default: script directory)."""
    if dll_dir is None:
        dll_dir = os.path.dirname(os.path.abspath(__file__))

    dll_path = os.path.join(dll_dir, "oneocr_wrapper.dll")

    # Set DLL search directory so wrapper can find oneocr.dll / onnxruntime.dll
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel32.SetDllDirectoryW(dll_dir)

    dll = ctypes.CDLL(dll_path)

    # int initModel(const wchar_t* model_dir)
    dll.initModel.argtypes = [ctypes.c_wchar_p]
    dll.initModel.restype = ctypes.c_int

    # int releaseModel()
    dll.releaseModel.argtypes = []
    dll.releaseModel.restype = ctypes.c_int

    # int ocrImage(const wchar_t* image_path, char*& utf8_json, ALLOC_FUNC func)
    # char*& in C ABI = char** (pointer to pointer)
    dll.ocrImage.argtypes = [ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_char_p), ALLOC_FUNC]
    dll.ocrImage.restype = ctypes.c_int

    # int ocrImageEx(const wchar_t* image_path, char*& utf8_json, ALLOC_FUNC func,
    #                int max_lines, int resize_w, int resize_h)
    dll.ocrImageEx.argtypes = [
        ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_char_p), ALLOC_FUNC,
        ctypes.c_int, ctypes.c_int, ctypes.c_int,
    ]
    dll.ocrImageEx.restype = ctypes.c_int

    return dll


def call_ocr_image(dll, image_path):
    """Call ocrImage, return (ret_code, json_string)."""
    buf = ctypes.c_char_p(None)
    ret = dll.ocrImage(image_path, ctypes.byref(buf), alloc_func)
    result = None
    if buf.value is not None:
        if ret == OCR_OK:
            result = buf.value.decode("utf-8")
        _crt.free(ctypes.cast(buf, ctypes.c_void_p))
    return ret, result


def call_ocr_image_ex(dll, image_path, max_lines, resize_w, resize_h):
    """Call ocrImageEx, return (ret_code, json_string)."""
    buf = ctypes.c_char_p(None)
    ret = dll.ocrImageEx(image_path, ctypes.byref(buf), alloc_func,
                         max_lines, resize_w, resize_h)
    result = None
    if buf.value is not None:
        if ret == OCR_OK:
            result = buf.value.decode("utf-8")
        _crt.free(ctypes.cast(buf, ctypes.c_void_p))
    return ret, result


def main():
    image_path = "test.png"
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Load DLL
    print("Loading oneocr_wrapper.dll ...")
    dll = load_wrapper(script_dir)

    # initModel: "." = current directory containing oneocr.dll + oneocr.onemodel
    model_dir = script_dir
    ret = dll.initModel(model_dir)
    if ret != OCR_OK:
        print(f"initModel failed: {ret}")
        return 1
    print("Model initialized.")

    # --- Test 1: basic ocrImage ---
    print("\n=== Test 1: ocrImage (basic) ===")
    abs_image = os.path.join(script_dir, image_path)
    ret, json_str = call_ocr_image(dll, abs_image)
    if ret != OCR_OK:
        print(f"ocrImage failed: {ret}")
        dll.releaseModel()
        return 1

    out_file = os.path.join(script_dir, "oneocr_test_result.json")
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(json_str)
    print(f"OCR result saved to {out_file}")

    print("--- JSON preview (first 800 chars) ---")
    print(json_str[:800])
    if len(json_str) > 800:
        print(f"... ({len(json_str)} bytes total)")

    # --- Test 2: ocrImageEx with custom options ---
    print("\n=== Test 2: ocrImageEx (max_lines=50, resize=800x600) ===")
    ret_ex, json_str_ex = call_ocr_image_ex(dll, abs_image, 50, 800, 600)
    if ret_ex != OCR_OK:
        print(f"ocrImageEx failed: {ret_ex} (may not be supported in this DLL version)")
    else:
        out_file_ex = os.path.join(script_dir, "oneocr_test_result_ex.json")
        with open(out_file_ex, "w", encoding="utf-8") as f:
            f.write(json_str_ex)
        print(f"Extended result saved to {out_file_ex}")

    # Cleanup
    dll.releaseModel()
    return 0


if __name__ == "__main__":
    sys.exit(main())
