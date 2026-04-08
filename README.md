# Windows 11 Snipping Tool OCR (OneOCR)

Offline OCR engine extracted from the Windows 11 Snipping Tool, with full-featured C++ CLI, reusable DLL wrapper, and Python visualization.

Based on: https://b1tg.github.io/post/win11-oneocr/

## Features

| Feature | Description |
|---|---|
| **8-point Bounding Box** | 4-corner polygon bbox for lines and words (not just axis-aligned rect) |
| **Word Confidence** | Per-word recognition confidence score (0.0–1.0) |
| **Image Angle** | Detected rotation angle of the text in the image |
| **Line Style** | Handwritten vs. printed text classification with confidence |
| **Resize Resolution** | Configurable max internal resize before OCR (performance/accuracy trade-off) |
| **Resource Release** | Proper cleanup via `ReleaseOcrResult`, `ReleaseOcrPipeline`, etc. |
| **Unicode Path** | Full Unicode file path support via `_wfopen` in the DLL wrapper |
| **Multi-image Batch** | Process multiple images in one invocation |
| **Plain Text Output** | `--text` mode for pipe-friendly output (no JSON) |
| **Raw Buffer OCR** | `ocrImageRaw()` for in-memory BGRA pixel buffers (no file I/O) |
| **Visualization** | Python script with confidence-colored word boxes and style labels |

## Prerequisites

- Windows 11 (tested on 23H2+)
- Snipping Tool 11.2409.25.0+

Copy these 3 files from the Snipping Tool installation folder into the same directory as `ocr.exe`:

- `oneocr.dll`
- `oneocr.onemodel`
- `onnxruntime.dll`

Find the Snipping Tool folder:
```powershell
Get-AppxPackage Microsoft.ScreenSketch | Select-Object -ExpandProperty InstallLocation
```

Example: `C:\Program Files\WindowsApps\Microsoft.ScreenSketch_11.2409.25.0_x64__8wekyb3d8bbwe\SnippingTool`

## CLI Usage (`ocr.exe`)

```cmd
ocr.exe <image1.png> [image2.jpg ...] [options]
```

### Options

| Option | Description |
|---|---|
| `--text`, `-t` | Output plain text only (no JSON) |
| `--output`, `-o <file>` | Write JSON to specified file (default: `<image>.json`) |
| `--max-lines <n>` | Max recognition lines, 1–1000 (default 1000) |
| `--resize <WxH>` | Max internal resize resolution (e.g. `1152x768`) |
| `--quiet`, `-q` | Suppress progress messages |
| `--help`, `-h` | Show help |

### Examples

```cmd
# Single image → JSON
ocr.exe screenshot.png

# Plain text output (pipe to file)
ocr.exe screenshot.png --text > result.txt

# Batch process
ocr.exe img1.png img2.jpg img3.bmp

# Custom options
ocr.exe photo.jpg --max-lines 50 --resize 800x600 -o result.json
```

### JSON Output Format

```json
{
  "file": "test.png",
  "image": { "width": 771, "height": 479, "step": 3084 },
  "image_angle": 0.0643,
  "line_count": 2,
  "lines": [
    {
      "index": 0,
      "text": "Hello World",
      "bounding_box": [
        13.0, 38.0, 458.0, 38.0,
        458.0, 77.0, 13.0, 76.0
      ],
      "style": { "type": "printed", "confidence": 0.98 },
      "word_count": 2,
      "words": [
        {
          "index": 0,
          "text": "Hello",
          "bounding_box": [
            14.35, 39.70, 140.35, 41.31,
            139.93, 73.42, 13.78, 74.09
          ],
          "confidence": 0.987
        }
      ]
    }
  ]
}
```

## DLL Wrapper (`oneocr_wrapper.dll`)

A reusable C DLL wrapper with 3 main APIs:

| Function | Description |
|---|---|
| `initModel(model_dir)` | Load DLL + model, initialize pipeline |
| `ocrImage(image_path, json, alloc)` | OCR an image file → JSON string |
| `ocrImageEx(image_path, json, alloc, max_lines, resize_w, resize_h)` | OCR with configurable options |
| `ocrImageRaw(pixel_data, w, h, step, json, alloc)` | OCR on raw BGRA pixel buffer |
| `releaseModel()` | Clean up all resources |

### C++ Header-Only Usage (`oneocr.h`)

```cpp
#include "oneocr.h"

OneOcr ocr;                               // loads oneocr_wrapper.dll
ocr.initModel(L".");                      // directory with oneocr.dll + .onemodel

std::string json;
ocr.ocrImage(L"test.png", json);          // basic OCR
ocr.ocrImageEx(L"test.png", json, 50);    // max 50 lines
ocr.ocrImageRaw(bgra_ptr, w, h, json);    // raw buffer OCR
```

## Visualization (`visualize.py`)

```cmd
python visualize.py <image_path> <json_path> [output_path]
```

Features:
- 8-point polygon bounding boxes (lines in red, words colored by confidence)
- Confidence score labels below each word
- Handwritten lines highlighted in orange, printed in red
- Image angle and line count overlay

## Build

Requires: MSVC (Visual Studio), `json.hpp` (nlohmann/json), `stb_image.h`.

```cmd
# Build CLI
cl /EHsc /O2 ocr.cpp /Fe:ocr.exe

# Build wrapper DLL
cl /EHsc /O2 /LD oneocr_wrapper.cpp /Fe:oneocr_wrapper.dll

# Build test
cl /EHsc /O2 oneocr_test.cpp /Fe:oneocr_test.exe
```

## Other Implementations

| Directory | Language | Description |
|---|---|---|
| `oneocr/` | Python | PyPI package with PIL/cv2 input, FastAPI web server |
| `oneocr-rs/` | Rust | crates.io library with `image` crate, serde JSON |
| `oneocr-cli/` | Rust | Minimal CLI, plain text output |
| `win11_oneocr_py/` | Python | Basic ctypes script (original Python port) |

## Credits

- [b1tg](https://github.com/b1tg/win11-oneocr) - Original reverse engineering and C++ implementation
- [AuroraWright/oneocr](https://github.com/AuroraWright/oneocr) - Python package with web server
- [wangfu91/oneocr-rs](https://github.com/wangfu91/oneocr-rs) - Rust binding with full feature coverage
- [Cecilia-pj/win11_oneocr_py](https://github.com/Cecilia-pj/win11_oneocr_py) - Original Python port

## License

MIT
