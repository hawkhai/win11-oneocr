# Windows 11 Snipping Tool OCR (OneOCR)

新接手先看 [一页入口](docs/start-here.md)：当前结论、运行、编译、复核和当前离线包选择。第二轮交付工具完整测试 70/70、已有全量结果复核 538/538，见 [补强报告](reports/cross-delivery-v2-2026-11-04/README.md)。下文旧测试计数与历史证据仍保留。

2026-11-04 用户后续要求的 [Wasm 网页能力对齐](reports/wasm-web-alignment-2026-11-04/README.md)已完成：网页支持行预算及缩放参数，Chrome 当前 **269/269** 新推理及独立复核通过（266 识别、3 预期拒绝），网页/Worker/Node API 功能检查通过，Python 当前完整套件 **71/71**。未修改模型、原生实现或阈值，原有运行资产不变，Wasm 仍不作为 C++ 收手门槛。

**验收铁律（2026-11-03 用户确认）：C++ 原生版本必须按原 DLL 严格对齐；沿用坐标 1px、角度 0.1°、置信度 1e-3 的阈值，其余识别字段按既有契约一致。JS/WebAssembly 不要求绝对一致，其差异不阻塞 C++ 验收。** 详见 [AGENTS.md](AGENTS.md) 和[当前验收标准](docs/acceptance-standard.md)。下述五环境结果为历史实测记录。

**收手范围（2026-11-04 用户确认）：Windows x64 / Linux x64 C++ 当前全部登记项严格通过后，本地提交并刷新离线包即结束本轮，不继续自动扩大输入、参数或平台。** 当前两端全量新推理与独立复核均为 538/538，收尾依据见 [Windows/Linux 验收报告](reports/windows-linux-closeout-2026-11-04/README.md)。其他平台的未验证状态继续保留，不阻塞本轮完成。

交付和换机接手先读[最终交接文档（2026-11-04 更新）](docs/handoff-2026-11-03.md)：当前收手状态、最新证据、资产/包身份、同事自备依赖的编译入口、历史快照与后续边界均已整理。本轮已结束，未经用户另行要求不自动恢复开发。

最新[同事自备依赖的干净源码构建](reports/colleague-build-2026-11-04/README.md)已验证显式 SDK/源码/数据路径；[此前联合参数扩展](reports/parameter-expansion-2026-11-04/README.md)保留旧范围并增加 48 组旋转手写、多语种及版式参数。Windows/Linux 原生全新严格回归各 **269/269**，合计 **538/538**（532 项识别、6 项预期拒绝），保存结果独立复核亦为 538/538；CTest 各 **26/26**，该次 Python 完整套件 **67/67**。生产算法、模型和阈值未改，该次原生收尾未执行新的 Wasm 推理。此前 [Photos 交集测试 P2 修复](reports/photos-intersection-p2-2026-11-03/README.md)的 13 个负例和 3 个边界正例继续保留。

严格比较器的换机字节身份已通过 `.gitattributes` 固定，避免 Git CRLF 转换导致摘要漂移，见[补充校验](reports/photos-intersection-checkout-2026-11-03/README.md)。该修补不改比较器或阈值，历史封存报告保持原样。

## Clone and run / 拉取即用

同事重新编译 C++ 的依赖准备与命令见[编译说明](docs/colleague-build.md)。依赖由同事自行编译；本轮提交源码、脚本、版本锁定与说明，不新增大体积依赖包或二进制。Windows 脚本可显式接收 ORT SDK、OpenCV/ICU 源码和 ICU 数据路径，依赖就绪后不要求研究 Python 或 WSL。

完整 Git 检出已包含独立模型与验证过的 Windows x64、Linux x64 原生运行文件，无需 Python、编译器、账号或激活服务即可识别。

当前 `models/` 使用 Photos 2026.11090.22001.0 的模型，以配套 OneOCR 11.0.15 为基准。样本已扩展为 126 张，并注册 115 个参数组合（114 个非默认组合和一个默认对照）；新增两张真实纸面中文手写照片及八张旋转派生图，见 [参数与真实中文手写回归](reports/options-handwriting-2026-11-03/README.md)。曲线备用求解、极长行裁剪等既有能力继续纳入全量。原 9.0.103 DLL/模型仍保存在 `bin/` 作旧版对照；新版对照文件单独位于 `bin/photos-2026.11090.22001.0/`。

此前参数与真实手写一轮的 Windows／Linux 原生、Windows Node／Chrome／Linux Node 新推理均为 **221/221**，共 **1105/1105**（1090 项识别输出、15 项预期拒绝）。该历史记录不改变当前以 C++ 为门槛的铁律。模型与比较阈值保持不变；更多参数、书写者、文字类别和未实测平台仍未完成，整体状态继续为 incomplete。

```powershell
.\ocr.cmd .\ocr-book.jpg --text
```

```sh
./ocr.sh ./ocr-book.jpg --text
```

Linux 现有预编译包要求 **glibc 2.43+**（在 Ubuntu 26.04 x64 验证），不满足时需在目标系统从源码构建。macOS、Android、ARM64 仍需目标平台构建验证。模型与运行库均使用普通 Git 文件，未使用 Git LFS；首次克隆会下载较多二进制资源。完整资产范围、来源声明和标准库校验命令见 [交付说明](docs/checkout-ready.md)。本次仅本地 commit，未 push 或发布模型。

上一轮[曲线与极长行补齐回归](reports/curve-boundaries-2026-11-03/README.md)曾用全新推理完成五端各 **142/142**，共 **710/710**（695 项识别、15 项预期拒绝）。模型、原始图片和严格比较阈值均未改变；源码、四张新增样本、内部预言机、运行文件与报告均已提交。

## Native source runtime — experimental

[词框断言与源码包完整性修复](reports/word-box-zip-p2-2026-11-03/README.md)补齐最终词框 NaN/Infinity 拒绝负例，并按报告索引收齐源码 ZIP 的压缩日志。原五端 710 项结果按原严格比较器独立复核；执行记录和新复核分别保存，模型与推理运行文件保持原样。

The repository now contains a C++20/CMake backend that runs independently of
the original OneOCR DLL. Windows x64 and Linux x64 builds have been tested;
Linux inference also passed in a network namespace with no outbound route.
Model preparation can unpack the existing local `.onemodel` without loading
Microsoft DLLs. Android/macOS configurations and an Android JNI example are
included but have not been verified on those targets.

The recovered detector, line/word alignment, reading order, confidence,
handwriting and orientation logic is checked against the fixed original DLL.
See the [current model regression report](reports/curve-boundaries-2026-11-03/README.md)
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

JavaScript/浏览器接入使用同一引擎的 [WebAssembly 版本](docs/wasm.md)，或运行 `node web/cli.mjs ocr-book.jpg --text`。Chrome 156 已通过当前全部 **269 项**原阈值诊断；Windows/Linux Node 24.14.0 的完整集仍为历史 **221 项**，见[历史实测报告](reports/options-handwriting-2026-11-03/README.md)。插件配置 `backend: wasm` 可选择该后端；模型仍使用根目录 `models/`。

[Review 修复后的增强回归](reports/review-fixes-2026-11-03/README.md)已用全新推理验证 Windows/Linux 原生、Windows/Linux Node 24 和 Chrome，五端各 **138/138**。新比较器额外检查计数、索引及字段类型；模型准备不会默认覆盖回旧版，自定义运行模块/Worker URL 的接入错误已修复。模型与数值阈值保持不变，未验证平台继续标注。

[后续边界修复](reports/review-boundaries-2026-11-03/README.md)还补齐了数值合法范围、无清单非空目录保护和只读复核记录。原五端 690 项记录已按新规则全量复核通过；本轮未重新执行 690 次推理，也未修改原始执行记录、模型或数值引擎。

[执行证据绑定修复](reports/evidence-binding-2026-11-03/README.md)将重复元数据与封存原件绑定，并统一拒绝冲突和未登记的结果。五端 690 项保存记录按修复后的规则复核通过；复核与新推理分别记录。

[输入范围与参考缓存修复](reports/input-cache-binding-2026-11-03/README.md)补齐五环境入口对当前图片和样本范围的核验，并拒绝参考实现复用异版本或损坏缓存。模型、推理运行文件和严格阈值保持原样，历史复核与新运行分别记录。

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
