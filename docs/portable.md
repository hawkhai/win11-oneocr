# OneOCR 原生源码运行时

已有可编译、可运行的 C++20 实现，Windows x64 和 Linux x64 已验证脱离原始 OneOCR DLL 运行。**完整能力对齐尚未完成，当前是实验版本。** 27 项固定用例均能执行，但按全部字段、框坐标、角度和置信度的严格标准仅 1 项通过，不能替换要求原 DLL 等价输出的业务。

## 已交付内容

- C++ 检测、文字分类、识别、CTC 解码、四边形处理和 JSON 输出；正式运行不使用 Python、Wine 或 `oneocr.dll`。
- 实例式 C ABI、C++ RAII 接口、Windows 旧接口适配 DLL、Android JNI/Java 示例。
- 从本地模型容器静态导出 67 项资源，包括 34 个 ONNX 图、九组完整字表、组合字符映射和校准表；模型准备也不加载微软 DLL。
- 23 个私有特征层转换为标准 ONNX 运算。22 个小型网络各使用 64 组原生探针；最大输出差为 `1.7673e-5`。布局网络在 12 次原生调用的输出上误差为 0。
- Windows 的 ONNX Runtime 已从固定提交源码构建。Linux 本轮使用校验过的官方 CPU SDK 编译、运行本项目 C++ 源码；Linux ORT 自身的源码构建尚未执行。

模型数据保存在独立的 `models/`，不作为 C++ 数组嵌入程序。容器准备工具单独采用 AGPL-3.0-only；C++ 运行时维持项目 MIT 标识，模型所有权保持原样，详见 [第三方声明](../THIRD_PARTY_NOTICES.md)。

## 本机重新构建

在项目根目录运行。研究环境仅用于模型准备、构建和验证。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/bootstrap-research.ps1
.\.research-python\python.exe scripts/prepare-models.py
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-native.ps1 -SourceRuntime
.\build\source-runtime\native\Release\ocr.exe test.png --model-dir models --text
```

`prepare-models.py` 只读取现有 `bin/oneocr.onemodel`，不访问 Store，不加载 `oneocr.dll`，不下载模型。输出包含原资源名、SHA-256、图输入输出、算子、字表特殊 token、校准表和转换合约。未知私有特征层不会被猜测为已兼容。

`-SourceRuntime` 下载固定的 ORT 源码、构建 CPU 动态库并安装到 `work/ort-install`，再编译本项目。依赖源码下载需要网络；准备完成后的识别不需要网络。固定版本见 [dependencies.json](../research/dependencies.json)，研究 Python 的完整版本锁定在 `scripts/requirements-lock.txt`。

已有 SDK 时可显式使用 `-SdkRoot`，这条路径不声称 SDK 是本机从源码构建的：

```powershell
.\.research-python\python.exe scripts/fetch-runtime.py --sdk windows-x64
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-native.ps1 -SdkRoot work/vendor/sdk-windows-x64/onnxruntime-win-x64-1.23.2
```

## Linux、macOS 与 Android

使用 Python 3.12+ 准备数据；将 `models/` 复制到目标平台即可，推理不需要 Python。

Linux x64 使用官方 SDK 的已验证路径：

```bash
python3 scripts/fetch-runtime.py --sdk linux-x64
export ONNXRUNTIME_ROOT="$PWD/work/vendor/sdk-linux-x64/onnxruntime-linux-x64-1.23.2"
cmake --preset linux-x64
cmake --build --preset linux-x64
ctest --test-dir build/linux-x64 --output-on-failure
build/linux-x64/native/ocr test.png --model-dir models --text
```

ORT 自身从源码构建的通用路径，在对应架构主机上执行：

```bash
python3 scripts/fetch-runtime.py --source
python3 scripts/build-ort.py --build_dir "$PWD/work/ort-source" --config Release --update --build --target onnxruntime --parallel 4 --build_shared_lib --skip_tests --compile_no_warning_as_error --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF
cmake --install work/ort-source/Release --config Release --prefix "$PWD/work/ort-install"
export ONNXRUNTIME_ROOT="$PWD/work/ort-install"
```

随后选择 `linux-x64`、`macos-x64` 或 `macos-arm64` 预设。Linux ARM64 可在 ARM64 主机按上述通用流程原生构建；从 x64 交叉编译时，`linux-arm64` 预设要求 `aarch64-linux-gnu-g++` 和对应 ARM64 ORT SDK。

Android 使用同一份 C++ 核心和 NDK：

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/27.2.12479018"
python3 scripts/fetch-runtime.py --source
python3 scripts/build-ort.py --build_dir "$PWD/work/ort-android" --config Release --update --build --target onnxruntime --parallel 4 --android --android_abi arm64-v8a --android_api 27 --android_sdk_path "$ANDROID_SDK_ROOT" --android_ndk_path "$ANDROID_NDK_HOME" --android_cpp_shared --build_shared_lib --skip_tests --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF
cmake --install work/ort-android/Release --config Release --prefix "$PWD/work/ort-android-install"
export ONNXRUNTIME_ROOT="$PWD/work/ort-android-install"
cmake --preset android-arm64
cmake --build --preset android-arm64
```

把 `liboneocr_jni.so`、`liboneocr_portable.so`、目标架构的 `libonnxruntime.so` 以及 NDK 的 `libc++_shared.so` 放入宿主应用的 `jniLibs/arm64-v8a/`，将模型复制到应用私有目录。Java 示例是 `native/android/OneOcr.java`，用 `try-with-resources` 管理生命周期。原始缓冲区接口接受 **BGRA**，Android Bitmap 常用的 RGBA 需要调用方转换；缓冲区从 direct ByteBuffer 基址读取。

Android 与 macOS 命令是构建配置，尚未进行目标平台编译或真机运行验证。Java 示例已用 JDK 21 编译通过，这不等同于 Android 验证。

| 平台 | 本项目编译 | 本项目运行 | ORT 本机源码构建 |
| --- | --- | --- | --- |
| Windows x64 | 通过 | 通过 | 通过 |
| Linux x64 / WSL | 通过 | 无网络命名空间内通过 | 未执行；本轮使用官方 SDK |
| Linux arm64 | 未验证 | 未验证 | 未验证 |
| macOS x64 / arm64 | 未验证 | 未验证 | 未验证 |
| Android arm64-v8a | 未验证 | 未验证 | 未验证 |

## 接口与使用

CLI 保留 `--text`、`--output`、`--max-lines`、`--resize`、`--quiet`；新增 `--model-dir` 和 `--capabilities`。默认最大行数 1000，内部缩放上限 `1152×768`。批处理不能把多张图片写入同一个 `--output` 文件。

```cpp
#include "oneocr_portable.hpp"
oneocr::Engine engine("models");
std::string result = engine.recognize("test.png");
```

C 接口见 `native/include/oneocr_portable.h`。路径与 JSON 使用 UTF-8，所有返回字符串由 `oneocr_free` 释放；失败时 JSON 输出置空。BGRA 接口额外接收缓冲区字节数，可验证步长和越界；alpha 与旧接口一致，不参与识别。一个实例内识别调用串行化，独立实例可并发；销毁实例前调用方必须结束使用它的线程。

Windows 构建另生成 `oneocr_wrapper.dll`，保留 `initModel`、`releaseModel`、`ocrImage`、`ocrImageEx`、`ocrImageRaw` 五个旧入口；初始化目录改为准备后的 `models/`。新适配库调用本项目 C++ 引擎，不加载原 `oneocr.dll`。

JSON 继续使用 `file`、`image`、`image_angle`、`lines`、`words`、`bounding_box`、`style`、`confidence`。`image_angle` 使用 `[0,360)` 度。增加的 `engine` 对象明确记录 `parity: incomplete`：当前词置信度为未经原始校准器处理的 CTC 分数，框、阅读顺序与手写处理也未完全对齐。

插件默认使用 `artifacts/<process.platform>-<process.arch>/bin/ocr[.exe]`，可配置 `ocrBin`、`modelDir`、`timeoutMs`、`maxOutputBytes`。`backend: legacy` 显式启用旧 Windows 路径；发布包不附带原始 OCR DLL 或模型。原 DLL 构建可使用 `-DONEOCR_BUILD_PORTABLE=OFF -DONEOCR_BUILD_LEGACY=ON`，CLI 目标为 `ocr_legacy`。

## 验证与本地运行包

```powershell
.\.research-python\python.exe scripts/test-native-api.py --library build/source-runtime/native/Release/oneocr_portable.dll --image work/oneocr-portable/corpus/latin-36.png
.\.research-python\python.exe scripts/make-corpus.py
.\.research-python\python.exe scripts/run-corpus.py --backend legacy
.\.research-python\python.exe scripts/run-corpus.py --backend native --binary build/source-runtime/native/Release/ocr.exe
.\.research-python\python.exe scripts/summarize-parity.py
```

最后一条当前应返回非零，这是原 DLL 对齐失败的真实结果。原始 CLI 不支持本次 Unicode 路径用例，基准工具改用原始 wrapper 的宽字符入口，并在独立子进程运行。

Linux 的断网测试使用独立网络命名空间，不修改主机联网状态：

```bash
sudo unshare -n python3 scripts/verify-offline.py --binary build/linux-x64/native/ocr --models models --image ocr-book.jpg --output work/oneocr-portable/linux-offline-ocr-book.json
```

本地打包，不上传、不发布：

```powershell
.\.research-python\python.exe scripts/stage-runtime.py --build build/source-runtime --prefix artifacts/win32-x64
```

这条命令要求 CMake 在当前进程 PATH 内；也可通过 `--cmake` 指定 `cmake.exe` 的绝对路径。模型与二进制分开存放，`BUILD.json` 记录产物及源码指纹。本轮不向 Git 新增模型；原仓库已跟踪的 `bin/` 样本保持原样，原生 npm 包不附带模型。

独立源码包可用 `.research-python/python.exe scripts/package-source.py` 生成，输出 `artifacts/oneocr-source.zip`。它包含源码、构建脚本、依赖锁定、说明和示例图片，排除所有模型、DLL 及编译产物；`SOURCE_BUILD.json` 记录源码摘要与未完成状态。准备模型时需要另行提供原始本地 `bin/oneocr.onemodel`，或直接使用已准备的独立 `models/`。

详细研究结论、失败指标和后续缺口见 [2026-11-02 研究报告](2026-11-02_oneocr-source-report.md)。
