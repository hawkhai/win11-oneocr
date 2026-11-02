# 错误码与接入处理

本文覆盖仓库公开定义的全部状态码，依据 `native/include/oneocr_portable.h`、`native/src/engine.cpp`、`native/src/windows_compat.cpp`、`oneocr.h` 和 `oneocr_wrapper.cpp` 的当前实现。返回码要与**接口、后端和操作阶段**一起记录：同一个数字在不同接口中可能有不同含义。

**本文的 `ONEOCR_*` 和 `OCR_ERR_*` 负数均由本项目定义，不是微软官方 OneOCR 错误码。** 下节单独列出已核实的官方资料；不能将不同接口的数值直接互换。

## 微软官方资料、状态与适用范围

核查日期：**2026-11-02**。在 Microsoft Learn / microsoft.com 检索 `oneocr.dll`、`RunOcrPipeline`、`OneOCR error code`，并检查下列公开 API 文档，**未找到 Photos 内部 `oneocr.dll` 导出函数的官方完整返回码表**。这里记录的是本次检索结果，不把社区绑定、逆向观测或项目包装层当成微软接口规范。

微软有公开的 [Windows App SDK OCR 接入文档](https://learn.microsoft.com/en-us/windows/ai/apis/text-recognition) 和 [TextRecognizer API](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.windows.ai.imaging.textrecognizer?view=windows-app-sdk-1.8)。其公开入口是 WinRT `TextRecognizer`；[RecognizeTextFromImage](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.windows.ai.imaging.textrecognizer.recognizetextfromimage?view=windows-app-sdk-1.8) 返回识别对象，该参考页没有列出 Photos 内部 `RunOcrPipeline` 的数字错误枚举。不能据此推定两者使用同一模型、版本、限制或内部状态值。

| 来源层级 | 本文内容 | 可用于什么 |
| --- | --- | --- |
| 微软公开 API / 上游源码 | 下列就绪状态、HRESULT、固定版本 ORT 枚举 | 解释相应官方接口或底层运行库的实际返回值 |
| 本项目源码定义 | `ONEOCR_*`、`OCR_ERR_*`、兼容层映射 | 对接本仓库提供的 C ABI / 包装 DLL |
| 指定 DLL 的研究观测 | 如历史版本 `RunOcrPipeline=3` | 仅解释记录中指定版本、函数和输入；不推广为官方枚举 |

### 官方 OCR 模型就绪状态

`TextRecognizer.GetReadyState()` 使用 [`AIFeatureReadyState`](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.windows.ai.aifeaturereadystate?view=windows-app-sdk-2.0)。以下是核查时官方页面列出的全部成员；其中 4..6 明确标注从 Windows App SDK 2.0 起提供。

| 数值 | 官方成员 | 含义与版本范围 |
| --- | --- | --- |
| `0` | `Ready` | 模型已就绪；后续仍可能退回非就绪状态 |
| `1` | `NotReady` | 尚未就绪；按官方流程检查并准备模型 |
| `2` | `NotSupportedOnCurrentSystem` | 当前系统或硬件不支持 |
| `3` | `DisabledByUser` | 模型被移除或禁用 |
| `4` | `CapabilityMissing` | 2.0 起：应用缺少 capability 声明；官方说明关联 `systemAIModels` 和访问拒绝异常 |
| `5` | `NotCompatibleWithSystemHardware` | 2.0 起：硬件与模型不兼容 |
| `6` | `OSUpdateNeeded` | 2.0 起：操作系统版本不兼容，需要升级 |

`EnsureReadyAsync()` 的结果另用 [`AIFeatureReadyResultState`](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.windows.ai.aifeaturereadyresultstate?view=windows-app-sdk-1.8)：

| 数值 | 官方成员 | 含义 |
| --- | --- | --- |
| `0` | `InProgress` | 准备中 |
| `1` | `Success` | 准备成功 |
| `2` | `Failure` | 准备失败 |

这两套枚举连成功值都不同：`Ready=0`，`Success=1`。失败详情来自 [`AIFeatureReadyResult.ExtendedError`](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.windows.ai.aifeaturereadyresult.extendederror?view=windows-app-sdk-1.8)，其 C++/WinRT 类型为 `HResult`，C# 投影为 `System.Exception`。官方 OCR 示例在准备失败时读取该属性。

上述就绪检查、组件安装及 capability 属于官方 Windows API 的接入流程；本项目离线原生引擎不调用它们，也不返回这些状态。

### 官方通用 HRESULT 参考

微软的 [Common HRESULT values](https://learn.microsoft.com/en-us/windows/win32/seccrypto/common-hresult-values) 列出以下常见值，完整 Windows 定义另见 SDK 的 `Winerror.h`。**这是通用 Windows 错误表，不是 OneOCR 专属错误表，也不表示 OCR 一定会返回其中每项。**

| 十六进制值 | 官方名称 | 含义 |
| --- | --- | --- |
| `0x00000000` | `S_OK` | 成功 |
| `0x80004001` | `E_NOTIMPL` | 未实现 |
| `0x80004002` | `E_NOINTERFACE` | 不支持所请求接口 |
| `0x80004003` | `E_POINTER` | 指针无效 |
| `0x80004004` | `E_ABORT` | 操作中止 |
| `0x80004005` | `E_FAIL` | 未细分的失败 |
| `0x8000FFFF` | `E_UNEXPECTED` | 非预期失败 |
| `0x80070005` | `E_ACCESSDENIED` | 访问被拒绝 |
| `0x80070006` | `E_HANDLE` | 句柄无效 |
| `0x8007000E` | `E_OUTOFMEMORY` | 内存分配失败 |
| `0x80070057` | `E_INVALIDARG` | 参数无效 |

对官方 WinRT API，记录异常类型、HRESULT 的 **32 位十六进制表示**及消息；不要把 HRESULT 截断、取负或映射为本项目 `-12/-18` 后声称来自官方。特别是本项目 `OCR_ERR_LOAD_DLL=-1` 只是包装层失败标记，并非 Windows 加载器的具体原因。

### 固定推理依赖：ORT 1.23.2 的官方错误枚举

本项目固定版本的微软上游源码 [`onnxruntime_c_api.h`：OrtErrorCode](https://github.com/microsoft/onnxruntime/blob/a83fc4d58cb48eb68890dd689f94f28288cf2278/include/onnxruntime/core/session/onnxruntime_c_api.h#L232-L248) 定义以下全部成员。数值按 C 枚举声明顺序从 0 递增；中文说明用于接入解读，不是 Photos 内部状态的推断。

| 数值 | 上游常量 | 含义 |
| --- | --- | --- |
| `0` | `ORT_OK` | 成功 |
| `1` | `ORT_FAIL` | 一般失败 |
| `2` | `ORT_INVALID_ARGUMENT` | 参数非法 |
| `3` | `ORT_NO_SUCHFILE` | 文件不存在 |
| `4` | `ORT_NO_MODEL` | 无模型 |
| `5` | `ORT_ENGINE_ERROR` | 引擎错误 |
| `6` | `ORT_RUNTIME_EXCEPTION` | 运行时异常 |
| `7` | `ORT_INVALID_PROTOBUF` | Protobuf 无效 |
| `8` | `ORT_MODEL_LOADED` | 模型已加载相关状态 |
| `9` | `ORT_NOT_IMPLEMENTED` | 未实现 |
| `10` | `ORT_INVALID_GRAPH` | 计算图非法 |
| `11` | `ORT_EP_FAIL` | 执行提供程序失败 |
| `12` | `ORT_MODEL_LOAD_CANCELED` | 模型加载已取消 |
| `13` | `ORT_MODEL_REQUIRES_COMPILATION` | 模型需要编译 |
| `14` | `ORT_NOT_FOUND` | 请求对象未找到 |

当前本项目边界把捕获到的 `Ort::Exception` 统一转为 `ONEOCR_RUNTIME_ERROR (-31)`，只传出异常文本，**不单独透传 `OrtErrorCode` 整数**；不能从 `-31` 反推上表某一项，也不能把 `ORT_NO_SUCHFILE=3` 当成 `RunOcrPipeline=3` 的解释。其余本项目状态仍按下一节源码定义判断。

## 本项目各接入入口

| 接入入口 | 错误形式 | 适用位置 |
| --- | --- | --- |
| 原生 C ABI：`oneocr_create`、`oneocr_recognize_file`、`oneocr_recognize_bgra` | 整数状态码＋可选 UTF-8 `char* error` | `artifacts/*/bin/oneocr_portable.dll` 或 `artifacts/*/lib/liboneocr_portable.so` |
| 原生 C++ RAII：`oneocr::Engine` | `std::runtime_error`，当前不保留整数状态码 | `native/include/oneocr_portable.hpp` |
| 原生 Windows 兼容接口：`initModel`、`ocrImage*` | 旧式整数返回值，部分原生码被合并 | `artifacts/win32-x64/bin/oneocr_wrapper.dll` |
| 微软 DLL 的旧包装接口：`initModel`、`ocrImage*` | `OCR_ERR_*` 整数返回值 | `bin/oneocr_wrapper.dll` 或 `bin/photos-2026.11090.22001.0/oneocr_wrapper.dll` |
| 原生 CLI：`ocr.cmd`、`ocr.sh`、`artifacts/*/bin/ocr[.exe]` | 进程退出码 `0` / `1`；错误说明在 stderr | 推荐的命令行入口 |
| DSH 插件 | JavaScript `Error` / Promise rejection | `src/index.js` |
| JavaScript / WebAssembly | 原生失败为 `OneOcrError`，保留 `code` / `stage`；加载与 Worker 失败也可能是普通 `Error` | `artifacts/wasm32/`，见 [Wasm 接入](wasm.md) |
| Android JNI/Java 示例 | Java 异常，当前不透传整数状态码 | `native/android/`；目标平台尚未验证 |

宿主连动态库都未加载成功时，应记录系统加载器或 FFI 的异常；此时没有调用 C ABI，不能把系统错误号解释为下表的 OneOCR 状态码。

## 原生 C ABI：全部状态码

新接入推荐使用此接口，保留错误详情。`0` 表示调用成功；空白图返回 `lines: []` 也是成功，不是错误。

| 数值 | 常量 | 当前含义与触发条件 | 接入处理 |
| --- | --- | --- | --- |
| `0` | `ONEOCR_OK` | 初始化或识别成功 | 使用返回结果；按下文释放内存 |
| `-12` | `ONEOCR_INVALID_ARGUMENT` | 缺少必要指针/路径；参数越界；BGRA 步长或缓冲区长度非法；资源相对路径非法；图像超过解码器防御性上限 | 修正参数或输入；不要原样循环重试 |
| `-13` | `ONEOCR_IMAGE_ERROR` | 文件无法打开/读取、文件为空或过大、格式不受解码器支持、图像损坏或解码失败 | 检查路径、权限及文件内容；必要时在调用前显式转码 |
| `-15` | `ONEOCR_UNSUPPORTED_IMAGE` | 图像被解码器识别，但通道数不是 1、3、4；典型为灰度＋透明度的双通道 `LA` PNG | 在业务层显式转为 RGB/RGBA，或正确构造 BGRA 缓冲区；转换后的图是新的输入 |
| `-18` | `ONEOCR_IMAGE_SIZE_ERROR` | 已进入 OCR 尺寸检查，宽或高不在 **50..10000** 像素范围内 | 拒绝输入或在业务层明确缩放/补边，再作为新输入调用 |
| `-19` | `ONEOCR_ALLOCATION_ERROR` | 捕获到 `std::bad_alloc`，包括结果字符串分配失败 | 降低并发、释放资源；根据实际资源状态决定是否重试 |
| `-30` | `ONEOCR_MODEL_ERROR` | 捕获到其他 `std::exception`：常见为模型/字表/清单缺失、SHA-256 不符、格式或版本不匹配；也包括几何处理等其他标准异常 | **必须记录 error 文本**；不能仅凭该码认定是模型损坏 |
| `-31` | `ONEOCR_RUNTIME_ERROR` | 捕获到 `Ort::Exception`，或未分类的非标准异常；可能在加载网络或推理时发生 | 核对运行库、模型配套关系和完整错误详情；不要自动替换 OCR 模型 |

`-12`、`-19`、`-30`、`-31` 均可能发生在初始化阶段。模型资源路径非法会返回 `-12`，ORT 加载模型失败可能返回 `-31`，因此“初始化失败”不固定对应某一个值。当前固定 ORT 版本检查失败抛出标准异常，归入 `-30`。

`-15` 专指通道限制，不是所有“格式不支持”的统称；无法识别的编码通常返回 `-13`。普通尺寸超限通常为 `-18`；但解码器更早检查到非正尺寸、单边超过 32768 或像素总数超过 100,000,000 时，返回 `-12`。同时违反多项要求时，以最先触发的检查为准。

参数范围：`max_lines=0` 使用默认 1000，其他值须为 1..1000；缩放宽高同时为 0 时使用 1152×768，否则两者都须为 32..8192。BGRA 需要有效缓冲区，`stride >= width*4`，`data_size >= (height-1)*stride + width*4`，并通过溢出检查；不接受负步长。Alpha 与原接口一样不参与识别。

### 错误字符串、句柄与内存

- `context`、`json`、`error` 的接收变量先初始化为 `NULL`。旧值须先按规则释放，函数不会替调用方释放旧值。
- `oneocr_create` 失败会把输出句柄置空；识别失败会把 JSON 输出置空。失败时不得读取或沿用上一次成功结果。
- `error` 输出参数可传 `NULL`；即使请求了错误文本，极端内存不足时文本也可能为 `NULL`。整数状态码才是主要判断依据。
- C ABI 返回的 JSON 和错误字符串均用 **`oneocr_free()`** 释放，不用调用方的 `free`、`delete[]` 或语言默认释放器。
- `oneocr_destroy()` 和 `oneocr_free()` 返回 `void`，没有成功/失败码；可传 `NULL`。有效句柄只销毁一次，销毁前须等待使用该实例的识别结束。
- `oneocr_capabilities()` 返回借用的静态字符串，不能传给 `oneocr_free()`。

错误说明是诊断文本，不是稳定的机器协议。业务分支按“接口＋阶段＋整数码”判断；保留文本供排查，不依赖精确英文字符串做分类。

### C 接入示例

```c
#include <stdio.h>
#include "oneocr_portable.h"

int main(void) {
    oneocr_context *ctx = NULL;
    char *json = NULL, *error = NULL;
    int code = oneocr_create("models", &ctx, &error);
    const char *stage = "create";
    if (code != ONEOCR_OK) goto done;

    stage = "recognize_file";
    code = oneocr_recognize_file(ctx, "test.png", NULL, &json, &error);
    if (code == ONEOCR_OK) puts(json);

done:
    if (code != ONEOCR_OK)
        fprintf(stderr, "%s failed: code=%d, detail=%s\n",
                stage, code, error ? error : "(no error text)");
    oneocr_free(json);
    oneocr_free(error);
    oneocr_destroy(ctx);
    return code == ONEOCR_OK ? 0 : 1;
}
```

FFI 应保留原始返回指针再交给 `oneocr_free()`；例如 ctypes 不要用会自动转为 Python `bytes` 的返回类型丢失指针。C++ RAII 封装当前只抛出错误文本；需要按数字分类时，直接使用 C ABI。

## 本项目针对原微软 DLL 的旧包装接口：全部公开定义

这些名称来自根目录 `oneocr.h`。`initModel` 的目录包含原 DLL 和 `.onemodel`，与原生实例接口所用的准备后 `models/` 目录不同。成功值为 `OCR_OK (0)`。

| 数值 | 常量 | 含义/发生阶段 |
| --- | --- | --- |
| `0` | `OCR_OK` | 操作成功 |
| `-1` | `OCR_ERR_LOAD_DLL` | `initModel` 加载 `oneocr.dll` 或其依赖失败；检查 DLL 路径、架构与配套运行库 |
| `-2` | `OCR_ERR_RESOLVE_FUNC` | 原 DLL 缺少必需导出函数，通常是版本/接口不匹配 |
| `-3` | `OCR_ERR_INIT_OPTIONS` | 原 DLL 的 `CreateOcrInitOptions` 失败 |
| `-4` | `OCR_ERR_DELAY_LOAD` | 设置模型延迟加载选项失败 |
| `-5` | `OCR_ERR_CREATE_PIPE` | `CreateOcrPipeline` 失败，检查模型、配套 DLL、路径及容器兼容性 |
| `-11` | `OCR_ERR_NOT_INIT` | 未成功初始化，或已释放模型；先调用并检查 `initModel` |
| `-12` | `OCR_ERR_INVALID_PARAM` | 识别入口的图像路径、像素指针或分配回调为空；裸缓冲区宽高非正等 |
| `-13` | `OCR_ERR_LOAD_IMAGE` | 文件打开或 stb_image 解码失败 |
| `-14` | `OCR_ERR_ALLOC_BGRA` | 分配 BGRA 中间缓冲区失败 |
| `-15` | `OCR_ERR_UNSUPPORTED` | 文件通道数不在 1、3、4 中，典型为双通道灰度＋Alpha |
| `-16` | `OCR_ERR_PROC_OPTIONS` | `CreateOcrProcessOptions` 失败 |
| `-17` | `OCR_ERR_MAX_LINES` | 调用原 DLL 设置最大识别行数失败 |
| `-18` | `OCR_ERR_RUN_PIPELINE` | 原 DLL 的 `RunOcrPipeline` 返回非零；**汇总管线错误，不只代表尺寸问题** |
| `-19` | `OCR_ERR_ALLOC_JSON` | 调用方提供的 `ALLOC_FUNC` 返回空指针，无法交付 JSON |
| `-20` | `OCR_ERR_SET_RESIZE` | `ocrImageEx` 调用原 DLL 设置缩放分辨率失败 |
| `-90` | `OCR_ERR_LOAD_WRAPPER` | `oneocr.h` 的 `OneOcr` 辅助类无法调用包装 API：包装 DLL 加载/解析失败，或请求的可选入口不存在；先检查 `isOk()` |
| `-91` | `OCR_ERR_RESOLVE_API` | 头文件定义的 API 解析失败宏；**当前 `OneOcr` 辅助类没有实际返回此值的分支**，相关调用实际返回 `-90` |

`-1..-20` 是包装 DLL 的状态；`-90/-91` 属于头文件辅助层，不是微软 OCR 内部状态码。旧接口的 `ocrImageEx` 会把非法 `max_lines` 回退为 1000，不能据此期待一定收到 `-17`；只有宽高均为正且存在对应导出时才调用原 DLL 的 resize 设置。

旧接口输出指针应由调用方先置空；失败路径不保证替调用方清空。成功 JSON 由调用方的 `ALLOC_FUNC` 分配，必须使用**与该分配器配对的释放函数**。不要拿这类回调分配的结果调用原生 C ABI 的 `oneocr_free()`。直接调用旧包装 DLL 时，不能把它视作具有新 C ABI 相同的异常隔离与完整缓冲区验证。

`OneOcr` 辅助类接收的 `std::string& utf8_json` 在失败时也可能保留上一次成功内容；必须先检查返回码，再使用字符串。

本次检索未找到微软 DLL 内部返回值的官方完整枚举。历史 9.0.103 的尺寸拒绝曾观测到内部 `RunOcrPipeline=3`，旧包装层将其转成 `-18`，指定版本的研究记录保存在仓库 `docs/2026-11-02_oneocr-parity-phase2.md`；这是观测结果，不能推广为官方状态定义。

## 原生 Windows 兼容适配 DLL：实际映射

这里指 `artifacts/win32-x64/bin/oneocr_wrapper.dll`，它调用源码引擎，不加载原微软 `oneocr.dll`。同名 DLL 的存放路径决定后端。

| 入口/来源 | 对外返回值 | 说明 |
| --- | --- | --- |
| `initModel` 成功 | `0` | 输入目录应为准备后的 `models/` |
| `initModel` 中 `oneocr_create` 返回任意错误 | `-5` | 初始化错误被合并；不保留原生的 `-12/-19/-30/-31` 等细分信息 |
| `initModel` 路径转换或外层 C++ 异常 | `-12` | 包括空路径、无效宽字符路径等；该 catch 为汇总分支 |
| `ocrImage*` 未初始化 | `-11` | 先初始化模型 |
| `ocrImage*` 参数检查、路径转换或外层异常 | `-12` | 以当前适配器的检查为准 |
| 识别时原生返回 `-30` 或 `-31` | `-18` | 合并为旧式管线错误，原生错误文本在适配层释放，不对外传出 |
| 识别时原生返回 `-12/-13/-15/-18/-19` | 原值 | 直接透传；这里的 `-18` 也可能来自原生尺寸错误 |
| 原生结果为空、或调用方 JSON 分配回调失败 | `-19` | 无法交付 JSON |
| `releaseModel` | 通常 `0`；外层异常为 `-12` | 重复释放空的全局模型可返回成功 |

当前适配器不会生成旧表中的 `-1/-2/-3/-4/-14/-16/-17/-20`。头文件辅助层的 `-90` 仍可能出现，`-91` 仍只是定义。`max_lines` 越界会回退为 1000；resize 则按原生范围验证，失败为 `-12`。需要完整诊断与实例隔离时，使用 `oneocr_portable.h`。

## CLI、插件和 JNI 的错误形式

### 原生 CLI

- `0`：处理成功，或成功执行 `--help` / `--capabilities`。空白图成功仍为 `0`。
- `1`：参数、初始化、读图、推理或输出文件写入等失败，通常 stderr 为 `ERROR: <说明>`；Windows 入口参数 UTF-8 转换失败也可能直接退出 `1`。

**CLI 不将 `-12/-15/-18/-30` 等 C ABI 状态码作为进程退出码暴露。** `ocr.cmd`、`ocr.sh` 正常启动后透传原生 CLI 的退出状态；`ocr.sh` 对未提供预编译包的平台也返回 `1`。进程未启动、动态库加载器失败、被信号/超时终止等应按操作系统或宿主的错误记录，不能套用 C ABI 表。

`--quiet` 不屏蔽错误。一次调用处理多张图时，原生 CLI 在首个失败处退出，前面的图片可能已经写入结果。接入时为每次调用使用新的输出路径，并检查退出状态、文件存在性及 JSON 解析结果，避免把残留文件当作本次成功结果。

旧 `bin/ocr.exe` / `ocr_legacy` 的批处理行为不同：单图 `ocr_one()` 失败会继续下一张，整个循环可能仍退出 `0`，输出写入也没有完整错误检查。旧 CLI 的 `0` 不能单独作为每张图成功的依据；推荐原生 CLI 或直接接 C ABI。

### DSH JavaScript 插件

当前没有独立的数字错误码枚举，也不返回 `oneocr_status`。失败以 JavaScript `Error` 拒绝调用，类型包括参数校验、平台不支持、进程启动失败、非零退出、超时、取消、stdout/stderr 或结果文件超过大小限制，以及结果文件读取/JSON 解析失败。

典型消息：`OneOCR call aborted`、`OneOCR timed out after ... ms`、`... exceeded ... bytes`、`OneOCR exited with code ...: ...`。消息不是稳定错误码协议；Node 的 `ENOENT` 等是进程/文件系统错误，不属于 OCR 枚举。若业务必须取得原生整数码，接 C ABI，或在宿主中定义独立的业务错误协议。

### Android 示例

Java 包装层对已关闭实例抛 `IllegalStateException`，非 direct BGRA 缓冲区抛 `IllegalArgumentException`；JNI 将原生错误文本转为 `IllegalStateException`，当前不保留整数码，无法取得文本时使用 `Native OCR failed`。JVM 自身的空指针、库加载、内存异常另按 Java/JNI 规则处理。本示例的目标平台运行仍未验证。

### JavaScript / WebAssembly

`OneOcrError.code` 保留同一 C ABI 的整数码，`stage` 标明创建、识别或 JS 参数/资源检查阶段；错误消息仍是诊断文本。JS 侧参数校验也可能抛 `-12`，可信清单摘要不符或不支持的清单版本为 `-30`。关闭的直接实例调用返回 `-12`。

Worker 会传回原生数字码；模块加载、文件/HTTP、Worker 退出、取消和超时也可能以普通 `Error` 拒绝 Promise，没有 OCR 数字码。Node 的 `ENOENT` 等字符串 `code` 保留其系统错误含义。取消或超时会终止 Worker，后续调用必须新建实例；不把这些情况伪造成 `-31`。Wasm CLI 同样成功退出 `0`、失败退出 `1`，不透传负数状态码。

## 三个回归拒绝样本

| 图片（相对 `samples/ocr-classics/`） | 原因 | 原生 C ABI | 旧包装接口 | 原生 CLI |
| --- | --- | --- | --- | --- |
| `images/cn-single-line-mixed.jpg` | 256×32，低于最小高度 | `-18` | `-18`（汇总） | 退出 `1`，`Image dimensions must be 50..10000 pixels` |
| `thumbnails/cn-single-line-mixed.jpg` | 同为 256×32 | `-18` | `-18`（汇总） | 同上 |
| `references/quality-border-comparison.png` | 双通道灰度＋Alpha | `-15` | `-15` | 退出 `1`，`Unsupported image channel count` |

三项计为“拒绝行为一致”，不计为识别成功。补边或通道转换可由接入方显式选择，但会改变输入及坐标关系，需保存转换信息；现有严格回归保留原图，未自动修图。
