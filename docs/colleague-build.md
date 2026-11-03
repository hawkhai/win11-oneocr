# 同事拉取后的 C++ 编译

引擎源码、CMake/脚本、依赖版本锁定、说明和验收记录随 Git 提交。**第三方依赖由同事自行取得和编译，不把大体积依赖归档、预编译 SDK 或新构建的二进制加入本轮 Git 提交。** 原有已封存模型、运行资产及其历史保持原样；新增编译输出留在忽略的 build/work 目录。

## 先自行准备固定依赖

版本与来源以 [research/dependencies.json](../research/dependencies.json) 为准：

| 输入 | 固定身份 | 提供给项目的路径 |
|---|---|---|
| ONNX Runtime | 1.23.2 / a83fc4d58cb48eb68890dd689f94f28288cf2278 | 已安装的目标平台 SDK 前缀，含 include 和 lib；Windows 还需 bin 或 lib 下的 onnxruntime.dll |
| OpenCV | 4.10.0 / 71d3237a093b60a27601c20e9ee6c3e52154e8b1 | 同版本 Git 源码检出；项目编译 core/imgproc 并静态链接 |
| ICU | 74.2 / 2d029329c82c7792b985024b2bdab5fc7278fbc8 | 同版本 Git 源码检出；项目编译 common |
| ICU Unicode/字典数据 | data SHA-256 dc778b9ffe18ed319ad3fb70754f80e51cf7b6dbfff38fc0c0a5f27bb5463dad | 同版源数据生成的 icudt74l_dat.c 文件 |

ORT 从源码构建应使用仓库已有的 scripts/build-ort.py，保留固定浮点规则；完整命令见 [portable.md](portable.md)。不要把能加载的任意官方 SDK 自动当作严格等价 SDK，既有 Linux 官方 SDK 差异记录仍保留。

ICU 数据可以由同事在 Linux/macOS/WSL 上执行 `python3 scripts/prepare-icu.py --source /path/to/icu` 生成，再将 C 文件交给 Windows 构建。已自行得到同版 icudt74l.dat 时，也可在 Windows 执行 `python scripts/prepare-icu.py --source C:/deps/icu --data C:/deps/icu-build/icudt74l.dat`，工具校验原数据摘要后输出 work/icu-generated/icudt74l_dat.c。生成依赖时可使用 Python；**依赖就绪后的引擎编译和正式识别不要求研究 Python 或 WSL**。

这里要求 Git 源码检出是为了核对固定提交，不能将无版本来源的头文件或数据静默替换进去。依赖准备输出不提交到本仓库。

## Windows x64

安装 Visual Studio 2022 C++ 桌面开发工具、Windows SDK 和 CMake。脚本能定位 VS 自带 CMake。依赖准备好后，在项目根执行，路径替换为同事自己的实际目录：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build-native.ps1 -SdkRoot C:/deps/ort-install -OpenCvSource C:/deps/opencv -IcuSource C:/deps/icu -IcuData C:/deps/icu-generated/icudt74l_dat.c -BuildDirectory build/colleague
.\build\colleague\native\Release\ocr.exe .\test.png --model-dir .\models --text
```

该模式只配置、编译并运行 CTest，不下载依赖，也不要求 .research-python 或 WSL。还可通过 ONNXRUNTIME_ROOT、ONEOCR_OPENCV_SOURCE、ONEOCR_ICU_SOURCE、ONEOCR_ICU_DATA_C 环境变量提供路径。

显式 `-SourceRuntime` 才进入原有的研究/完整 ORT 重建流程，可下载固定源代码并按需生成 ICU 数据。它和已经准备好依赖的 `-SdkRoot` 路径分别说明，不混称同事必须使用原开发机缓存。

## Linux x64

安装 CMake 3.21+、Ninja、支持 C++20 的编译器和常规系统开发头文件。以同事自己源码构建的 ORT SDK 为例：

```sh
export ONNXRUNTIME_ROOT=/path/to/ort-install
cmake --preset linux-x64 -DONEOCR_OPENCV_SOURCE=/path/to/opencv -DONEOCR_ICU_SOURCE=/path/to/icu -DONEOCR_ICU_DATA_C=/path/to/icu-generated/icudt74l_dat.c
cmake --build --preset linux-x64 --parallel 3
ctest --test-dir build/linux-x64 --output-on-failure
./build/linux-x64/native/ocr ./test.png --model-dir ./models --text
```

自编译版的系统要求由目标系统和所用依赖决定。仓库原有 Linux 预编译运行包仍要求 glibc ≥2.43，这不冒称源码只能在该版本编译；其他系统和编译器仍须自行严格验证，不能用已有 Windows/Ubuntu 测试替代。

## Git 与离线包

本任务只做本地 commit，不自动 push。团队远端需要先取得本次提交，同事的普通 git pull 才能拿到；也可以直接交付提交后生成的完整离线 bundle：

```sh
git clone -b research win11-oneocr-offline.bundle oneocr
```

完整 Git/bundle 含既有独立 models/；源码 ZIP 按原约定排除模型和编译产物，第三方依赖也由同事自行准备。脚本、锁定文件、说明、样本与报告应全部提交，不能遗漏源码或以本机构建缓存代替交付。

当前 C++ 验收范围仍为 Windows/Linux 各 269 项。完成构建交付和严格核验后按 AGENTS.md 收手；不继续扩展样本、参数或平台。
