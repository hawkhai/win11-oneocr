# Windows 11 Snipping Tool OCR (OneOCR)

## Clone and run / 拉取即用

完整 Git 检出已包含独立模型与验证过的 Windows x64、Linux x64 原生运行文件，无需 Python、编译器、账号或激活服务即可识别。

当前 `models/` 使用 Photos 2026.11090.22001.0 的模型，以配套 OneOCR 11.0.15 为基准，两端全部 110 张图片及原固定回归已严格通过，见 [新版模型回归](reports/photos-model-update-2026-11-02/README.md)。原 9.0.103 DLL/模型仍保存在 `bin/` 作旧版对照；新版对照文件单独位于 `bin/photos-2026.11090.22001.0/`。

```powershell
.\ocr.cmd .\ocr-book.jpg --text
```

```sh
./ocr.sh ./ocr-book.jpg --text
```

Linux 现有预编译包要求 **glibc 2.43+**（在 Ubuntu 26.04 x64 验证），不满足时需在目标系统从源码构建。macOS、Android、ARM64 仍需目标平台构建验证。模型与运行库均使用普通 Git 文件，未使用 Git LFS；首次克隆会下载较多二进制资源。完整资产范围、来源声明和标准库校验命令见 [交付说明](docs/checkout-ready.md)。本次仅本地 commit，未 push 或发布模型。

## Native source runtime — experimental

The repository now contains a C++20/CMake backend that runs independently of
the original OneOCR DLL. Windows x64 and Linux x64 builds have been tested;
Linux inference also passed in a network namespace with no outbound route.
Model preparation can unpack the existing local `.onemodel` without loading
Microsoft DLLs. Android/macOS configurations and an Android JNI example are
included but have not been verified on those targets.

The recovered detector, line/word alignment, reading order, confidence,
handwriting and orientation logic is checked against the fixed original DLL.
See the [current model regression report](reports/photos-model-update-2026-11-02/README.md)
for current results and exact tolerances. Full cross-platform validation is
still incomplete; native results expose `engine.parity = "incomplete"`.

See [native build and API instructions](docs/portable.md),
[all error codes and integration handling](docs/error-codes.md),
[JavaScript/WebAssembly build and integration](docs/wasm.md),
[research report](docs/2026-11-02_oneocr-source-report.md) and
[component/model notices](THIRD_PARTY_NOTICES.md). The Windows DLL-dependent
implementation documented below remains available as the optional legacy
backend. The plugin now defaults to the portable backend; use `backend: legacy`
explicitly for the old Windows executable.

JavaScript/浏览器接入使用同一引擎的 [WebAssembly 版本](docs/wasm.md)，或运行 `node web/cli.mjs ocr-book.jpg --text`。Node 24.14.0 和 Chrome 156 各通过全部 138 项原阈值回归，两端 JSON 逐字节相同，见[实测报告](reports/wasm-2026-11-03/README.md)。插件配置 `backend: wasm` 可选择该后端；模型仍使用根目录 `models/`。

[Review 修复后的增强回归](reports/review-fixes-2026-11-03/README.md)已用全新推理验证 Windows/Linux 原生、Windows/Linux Node 24 和 Chrome，五端各 **138/138**。新比较器额外检查计数、索引及字段类型；模型准备不会默认覆盖回旧版，自定义运行模块/Worker URL 的接入错误已修复。模型与数值阈值保持不变，未验证平台继续标注。

[后续边界修复](reports/review-boundaries-2026-11-03/README.md)还补齐了数值合法范围、无清单非空目录保护和只读复核记录。原五端 690 项记录已按新规则全量复核通过；本轮未重新执行 690 次推理，也未修改原始执行记录、模型或数值引擎。

Offline OCR engine extracted from the Windows 11 Snipping Tool, with full-featured C++ CLI, reusable DLL wrapper, and Python visualization.

Based on: https://b1tg.github.io/post/win11-oneocr/

## DeepSeek Harness plugin

This repository can be installed as a [DeepSeek Harness](https://github.com/deepseek-ai/DeepSeek-Harness) plugin:

```powershell
dsh plugin add github:hawkhai/win11-oneocr
```

It registers `oneocr_recognize`, a model-facing tool that accepts a local image path and returns recognized text together with OneOCR's structured line/word polygons, confidence values, image angle, and handwriting style. The tool runs locally using the configured native, Wasm or Windows legacy backend; image bytes are not sent to an external OCR service.

The plugin defaults to the native CLI under `artifacts/<platform>-<arch>/bin/` and the shared `models/` directory. Use `backend: wasm` for the Node/Wasm CLI or `backend: legacy` for `bin/ocr.exe`. Override `ocrBin`, `modelDir`, `timeoutMs`, or `maxOutputBytes` if needed. Installing as an npm package does not include the Microsoft models; the complete Git checkout does.

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

## Classic OCR Sample Library

The [Chinese case index](samples/ocr-classics/README.md) contains 30 independently sourced OCR inputs from 9 articles or official documents, plus 16 reference images. It covers OpenCV, Tesseract, PaddleOCR, EasyOCR, and CnOCR, with 12 Chinese or mixed Chinese/English inputs. Source attribution and image permissions are recorded separately in the corpus manifest; these materials do not inherit this repository's code license.

Run `python scripts/ocr-classics.py verify` for offline validation, or `python scripts/ocr-classics.py fetch` to restore the frozen collection. The samples are unannotated research materials, not recognition ground truth.

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
      "style": { "type": "printed", "confidence": 0.035 },
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

Native runtime and plugin code retain this project's MIT designation.
The standalone `scripts/unpack-onemodel.py` preparation utility is separately
AGPL-3.0-only. Microsoft model assets are not covered by either code license.
See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
