# OneOCR 原生源码运行时

当前曲线与极长行版本已重新推理五个环境，各 **142/142**，共 **710/710**；其中每端 139 项识别输出、3 项预期拒绝。备用 SVD、长行截断及检测 AREA 分支已补齐，见[最新报告](../reports/curve-boundaries-2026-11-03/README.md)。下列早期 110/138 项记录保留其历史范围，未替代本轮新执行。模型与严格阈值不变，未验证平台和剩余覆盖仍明确标注。

当前独立模型已升级为 Photos 2026.11090.22001.0（配套 DLL 11.0.15）。Windows/Linux 的 110 张图片、原 27 项与新版 106 行书页均按原阈值通过，详见 [新版模型报告](../reports/photos-model-update-2026-11-02/README.md)。原生 CPU 运行库仍固定为源码构建的 ORT 1.23.2，已实测兼容新版图；微软配套运行库为 1.24.20260504.1.7444c45，两者没有混为同一个构建。

后续已按补齐计数、索引和类型检查的比较器重新执行 Windows/Linux 原生完整回归，各 **138/138**，见 [Review 修复报告](../reports/review-fixes-2026-11-03/README.md)。模型准备工具默认来源也已绑定当前 Photos 版本，并拒绝异版本覆盖。

已有可编译、可运行的 C++20 实现，Windows x64 和 Linux x64 已验证脱离原始 OneOCR DLL 运行。第二阶段已恢复检测片段连接、曲线校正、行词框、阅读顺序、21 项置信度特征、手写和方向决策，并按原有严格阈值逐项回归。当前证据与剩余边界见 [第二阶段报告](2026-11-02_oneocr-parity-phase2.md)。Android、macOS、ARM64 尚未完成目标环境验证，完整跨平台目标仍为未完成。

## 已交付内容

- C++ 检测、文字分类、识别、CTC 解码、四边形处理和 JSON 输出；正式运行不使用 Python、Wine 或 `oneocr.dll`。
- 实例式 C ABI、C++ RAII 接口、Windows 旧接口适配 DLL、Android JNI/Java 示例。
- 从本地模型容器静态导出 67 项资源，包括 34 个 ONNX 图、九组完整字表、组合字符映射和校准表；模型准备也不加载微软 DLL。
- 旧版 9.0.103 包中的 23 个私有特征层可按已有合约转换。当前 Photos 包的 23 个辅助网络已是标准 ONNX，直接保留原始字节，无需该转换；两种准备路径均已验证。
- ONNX Runtime 固定为 1.23.2。严格对齐构建需使用本文的源码构建入口；它会固定浮点融合规则，并补齐 GCC 15 所需的标准头文件。各平台实测状态见第二阶段报告。

模型数据保存在独立的 `models/`，不作为 C++ 数组嵌入程序。容器准备工具单独采用 AGPL-3.0-only；C++ 运行时维持项目 MIT 标识，模型所有权保持原样，详见 [第三方声明](../THIRD_PARTY_NOTICES.md)。

## 本机重新构建

在项目根目录运行。研究环境仅用于模型准备、构建和验证。

图像校正使用与固定 DLL 内一致的 OpenCV 4.10.0 源码版本，只构建 `core` 和 `imgproc` 并静态链接。构建脚本自动调用 `scripts/fetch-opencv.py`，核对固定提交；直接调用 CMake 或在其他平台构建时，先运行 `python3 scripts/fetch-opencv.py`，也可通过 `ONEOCR_OPENCV_SOURCE` 指定已有的同版本源码。关闭 IPP、视频、图像编解码器和其他模块的下载。

Unicode 双向文本和泰语分词使用固定 ICU 74.2 的 common 源码与字典数据，全部静态链接。先用 `python3 scripts/prepare-icu.py` 在 Linux/macOS 构建主机生成数据 C 文件；Windows 构建脚本在数据缺失时调用 WSL，需要其中安装 C/C++ 编译器和 make。已有同版源码可用 `-IcuSource` 或 CMake 的 `ONEOCR_ICU_SOURCE` 指定，数据 C 文件可用 `ONEOCR_ICU_DATA_C` 指定。宿主生成工具不参与正式识别。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/bootstrap-research.ps1
.\.research-python\python.exe scripts/prepare-models.py
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-native.ps1 -SourceRuntime
.\build\source-runtime\native\Release\ocr.exe test.png --model-dir models --text
```

完整 Git 检出已有准备好的 `models/`，仅重新构建引擎时可跳过模型准备。`prepare-models.py` 默认根据 `research/delivery-manifest.json` 的活动基准选择并核对本地来源，当前为 `bin/photos-2026.11090.22001.0/oneocr.onemodel`；不会回退到旧的 `bin/oneocr.onemodel`。它不访问 Store、不加载微软 DLL、不下载模型。

在任何写入前，工具会拒绝来源与活动根目录模型绑定不符，或覆盖由另一来源准备的模型目录。独立研究其他版本必须显式指定 `--source` 和全新的 `--models` 目录。需要保留现有文件时，可用 `--models work/prepared-photos --unpacked work/prepared-photos-unpacked` 重新提取到独立目录，验证后再决定如何使用；不会自动替换正式模型。

非空输出目录若缺失有效的 `manifest.json`，也会在写入前被拒绝，避免将中断留下的其他版本文件当作新目录继续覆盖。此时应选择新的空目录，不会自动删除或修复未知内容；已存在的空目录仍可使用。

输出包含原资源名、SHA-256、图输入输出、算子、字表特殊 token、校准表和转换合约。未知私有特征层不会被猜测为已兼容。

研究对照工具 `scripts/run-reference.py` 在启动参考实现前，按当前模型清单校验源图、字表及已有参考缓存；不一致时拒绝运行并保留缓存，可用 `--cache-dir work/reference-current` 选择全新目录。新建缓存使用独立副本，不与正式模型共享可写硬链接。输出旁的 `.inputs.json` 记录来源模型、清单及全部 20 个实际输入文件摘要。该工具只用于对照研究，参考实现缺失的手写等能力仍不标记为已对齐。

`-SourceRuntime` 下载固定的 ORT 源码、构建 CPU 动态库并安装到 `work/ort-install`，再编译本项目。依赖源码下载需要网络；准备完成后的识别不需要网络。固定版本见 [dependencies.json](../research/dependencies.json)，研究 Python 的完整版本锁定在 `scripts/requirements-lock.txt`。

已有 SDK 时可显式使用 `-SdkRoot`，这条路径不声称 SDK 是本机从源码构建的：

```powershell
.\.research-python\python.exe scripts/fetch-runtime.py --sdk windows-x64
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-native.ps1 -SdkRoot work/vendor/sdk-windows-x64/onnxruntime-win-x64-1.23.2
```

## Linux、macOS 与 Android

使用 Python 3.12+ 准备数据；将 `models/` 复制到目标平台即可，推理不需要 Python。

Linux/macOS 构建主机先准备 OpenCV 与 ICU 数据：

```bash
python3 scripts/fetch-opencv.py
python3 scripts/prepare-icu.py
```

ORT 自身从源码构建的通用路径，在对应架构主机上执行。Linux 建议在本地 Linux 文件系统中选择构建目录，以减少跨 WSL 文件系统的编译开销：

```bash
python3 scripts/fetch-runtime.py --source
python3 scripts/build-ort.py --build_dir "$PWD/work/ort-source" --config Release --update --build --parallel 4 --build_shared_lib --skip_tests --compile_no_warning_as_error --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF
cmake --install work/ort-source/Release --config Release --prefix "$PWD/work/ort-install"
export ONNXRUNTIME_ROOT="$PWD/work/ort-install"
cmake --preset linux-x64 -U ONNXRUNTIME_INCLUDE_DIR -U ONNXRUNTIME_LIBRARY
cmake --build --preset linux-x64
ctest --test-dir build/linux-x64 --output-on-failure
build/linux-x64/native/ocr test.png --model-dir models --text
```

随后选择 `linux-x64`、`macos-x64` 或 `macos-arm64` 预设。Linux ARM64 可在 ARM64 主机按上述通用流程原生构建；从 x64 交叉编译时，`linux-arm64` 预设要求 `aarch64-linux-gnu-g++` 和对应 ARM64 ORT SDK。

`build-ort.py` 在非 Windows 平台强制 `-ffp-contract=off`，保留源码中显式的 FMA 运算。官方 Linux 预编译 SDK 虽能运行，本轮有 2/27 固定用例因量化加法的半整数舍入差异而超出严格阈值，不能作为等价验收的默认运行库。失败记录保留在案件 `evidence/hash-linux/`；没有改模型或放宽比较标准。

Android 使用同一份 C++ 核心和 NDK：

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/27.2.12479018"
python3 scripts/fetch-runtime.py --source
python3 scripts/build-ort.py --build_dir "$PWD/work/ort-android" --config Release --update --build --parallel 4 --android --android_abi arm64-v8a --android_api 27 --android_sdk_path "$ANDROID_SDK_ROOT" --android_ndk_path "$ANDROID_NDK_HOME" --android_cpp_shared --build_shared_lib --skip_tests --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF
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
| Linux x64 / WSL | 通过 | 无网络命名空间内通过 | 通过；固定隐式浮点融合规则 |
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

C 接口见 `native/include/oneocr_portable.h`。路径与 JSON 使用 UTF-8，所有返回字符串由 `oneocr_free` 释放；失败时 JSON 输出置空。图像宽、高各须为 50～10000 像素，与固定 DLL 的 `RunOcrPipeline` 边界一致；不满足时返回 `ONEOCR_IMAGE_SIZE_ERROR (-18)`。BGRA 接口额外接收缓冲区字节数，可验证步长和越界；alpha 与旧接口一致，不参与识别。一个实例内识别调用串行化，独立实例可并发；销毁实例前调用方必须结束使用它的线程。

完整错误码、原生/旧接口的映射差异、错误字符串与内存释放、CLI/插件/JNI 的错误处理及接入示例，见 [错误码与接入处理](error-codes.md)。原生 C ABI 的 `-18` 表示尺寸问题，旧接口的 `-18` 是汇总管线错误；CLI 失败统一为进程退出码 `1`。

该文档另附微软公开 OCR API 的就绪状态、HRESULT 和固定 ORT 版本的官方源码链接；本项目的 `ONEOCR_*`、`OCR_ERR_*` 负数是项目定义，不能作为微软内部 OneOCR 返回码表使用。

Windows 构建另生成 `oneocr_wrapper.dll`，保留 `initModel`、`releaseModel`、`ocrImage`、`ocrImageEx`、`ocrImageRaw` 五个旧入口；初始化目录改为准备后的 `models/`。新适配库调用本项目 C++ 引擎，不加载原 `oneocr.dll`。

JSON 继续使用 `file`、`image`、`image_angle`、`lines`、`words`、`bounding_box`、`style`、`confidence`。`image_angle` 使用 `[0,360)` 度。词置信度使用恢复的原始 21 项特征和同源网络；手写置信度使用原始分类器与校准表。增加的 `engine` 对象保留 `parity: incomplete`，表示有限回归通过不等于所有输入、参数与目标平台都已验证。

插件默认使用 `artifacts/<process.platform>-<process.arch>/bin/ocr[.exe]`，可配置 `ocrBin`、`modelDir`、`timeoutMs`、`maxOutputBytes`。`backend: legacy` 显式启用旧 Windows 路径；发布包不附带原始 OCR DLL 或模型。原 DLL 构建可使用 `-DONEOCR_BUILD_PORTABLE=OFF -DONEOCR_BUILD_LEGACY=ON`，CLI 目标为 `ocr_legacy`。

## 验证与本地运行包

```powershell
.\.research-python\python.exe scripts/test-native-api.py --library build/source-runtime/native/Release/oneocr_portable.dll --image work/oneocr-portable/corpus/latin-36.png
.\.research-python\python.exe scripts/make-corpus.py
.\.research-python\python.exe scripts/run-corpus.py --backend legacy
.\.research-python\python.exe scripts/run-corpus.py --backend native --binary build/source-runtime/native/Release/ocr.exe
.\.research-python\python.exe scripts/summarize-parity.py
```

固定语料 27 项与全目录 114 张图片（含缩略图、拼图、参考图和新增曲线/长行）均纳入当前回归，另含书页，共 142 项。最新结果与复跑命令见 [曲线与极长行报告](../reports/curve-boundaries-2026-11-03/README.md)；使用 `scripts/run-all-samples.py` 检查，不允许遗漏图片。原始 CLI 不支持 Unicode 路径用例，基准工具使用原始 wrapper 的宽字符入口，并在独立子进程运行。`scripts/run-parity-suite.py` 可复放其他固定清单；所有失败项保留，不自动放宽阈值。`--error-baseline` 只用于已记录的原 DLL 错误契约，不能将识别差异当成预期错误。

退化曲线保留原 LU 路径，仅在零主元时使用原 DLL 对应的 SVD 正规方程备用解。超长多边形保留源顶点、零目标尾部及三角形覆盖顺序，并恢复裁剪后的词框逆映射。极长图缩小后的面积比例不超过 0.04 时使用 AREA 插值；这是还原的算法常量，不是验收阈值。内部几何、像素及张量预言机在 Windows、Linux、Wasm 共用，见 [分支证据](../research/curve-boundaries/README.md)。

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
