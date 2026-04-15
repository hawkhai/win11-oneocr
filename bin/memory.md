# win11-oneocr 项目关键信息备忘

## 项目结构

```
F:\pythonx\myocr\win11-oneocr\
├── oneocr_wrapper.cpp      # 主 DLL 封装，加载 oneocr.dll，暴露 OCR API
├── oneocr_wrapper.h
├── bcrypt_hook.cpp         # 12 个 BCrypt 函数的 MinHook 钩子 + 日志
├── bcrypt_hook.h
├── onnx_dump.cpp           # 从 BCryptDecrypt 明文提取并保存 ONNX 子模型
├── onnx_dump.h
├── oneocr_test.cpp         # 测试程序
├── CMakeLists.txt
├── cmake/
│   └── copy_release_artifacts.cmake   # x64 Release 后处理拷贝脚本
├── bin/                    # 运行目录（x64 Release 产物）
│   ├── oneocr.dll
│   ├── oneocr.onemodel
│   ├── oneocr_wrapper.dll
│   ├── oneocr_test.exe
│   ├── MinHook.x64.dll
│   ├── logdir/             # BCrypt hook 日志
│   │   └── bcrypt_dump_<PID>.log
│   ├── onnx_dump/          # 解密出的 ONNX 子模型文件
│   └── verify_onnx.py      # ONNX 模型校验脚本（跳过容器 header + 截断尾部）
└── ../../note/minhook/     # MinHook 库（相对路径）
```

---

## 依赖路径（CMakeLists.txt）

| 变量 | 值 |
|---|---|
| `MINHOOK_DIR` | `${CMAKE_SOURCE_DIR}/../../note/minhook` |
| `MINHOOK_INCLUDE_DIR` | `${MINHOOK_DIR}/include` |
| `MINHOOK_LIB` (x64) | `${MINHOOK_DIR}/bin/MinHook.x64.lib` |
| `MINHOOK_DLL` (x64) | `${MINHOOK_DIR}/bin/MinHook.x64.dll` |
| 产物输出目录 | `${CMAKE_SOURCE_DIR}/bin` |

---

## BCrypt Hook 架构

### 钩住的 12 个函数（均来自 `bcrypt.dll`）

1. `BCryptOpenAlgorithmProvider`
2. `BCryptCloseAlgorithmProvider`
3. `BCryptGetProperty`
4. `BCryptSetProperty`
5. `BCryptGenerateSymmetricKey`
6. `BCryptDestroyKey`
7. `BCryptEncrypt`
8. `BCryptDecrypt`
9. `BCryptCreateHash`
10. `BCryptDestroyHash`
11. `BCryptHashData`
12. `BCryptFinishHash`

### 死锁修复（关键）

**问题**：所有 detour 最初在锁内调用原始函数，导致同线程递归死锁（`std::mutex` 不可重入）。

**修复原则**（所有 detour 统一模式）：
```cpp
// 1. 需要记录的输入 buffer → 先 memcpy 到栈上 snapshot
// 2. 调用原始函数（锁外）
NTSTATUS st = orig_XXX(...);
// 3. 加锁，写日志，解锁
std::lock_guard<std::mutex> lk(g_log_mutex);
if (g_log) { ... }
```

对于 `BCryptEncrypt`/`BCryptDecrypt`/`BCryptSetProperty`/`BCryptGenerateSymmetricKey`/`BCryptHashData`/`BCryptCreateHash`，需要在调用前 snapshot 输入 buffer（因为 CFB 模式会原地更新 IV，调用后 buffer 内容已变）。

---

## oneocr.dll 的加密机制（逆向分析结论）

### 算法
- **AES-256-CFB**
- **IV 固定**：`"Copyright @ OneO"`（ASCII，16字节）
- **密钥派生**：`AES_Key = SHA256(输入材料)`

### 密钥派生流程（每次加密/解密前固定执行）

```
BCryptOpenAlgorithmProvider(AES)
BCryptSetProperty(ChainingMode = "ChainingModeCFB")   ← UTF-16LE
BCryptSetProperty(MessageBlockLength = 16)
BCryptOpenAlgorithmProvider(SHA256)
BCryptCreateHash(SHA256)
BCryptHashData(输入材料)        ← 见下文
BCryptFinishHash → 32字节      ← 即 AES-256 密钥
BCryptGenerateSymmetricKey(key = SHA256 digest)
BCryptDecrypt / BCryptEncrypt
BCryptDestroyKey / BCryptCloseAlgorithmProvider
```

### 初始化阶段密钥材料（调用 1-17）

HashData 输入 = **48 字节**：
- 前 32 字节：源码中的硬编码 key 常量
  ```
  kj)TGtrK>f]b[Piow.gU+nC@s""""""4
  ```
- 后 16 字节：来自 `.onemodel` 文件头

派生出的 AES 密钥用于解密 **目录表**（22624 字节），目录表明文包含所有子模型的偏移和加密块。

### 子模型解密（循环，共约 234 轮）

HashData 输入 = **16 字节**（两个 uint64，对应子模型数据的偏移/大小信息），派生密钥解密对应的 ONNX protobuf 数据块。

解密后的明文（protobuf 格式）内部包含子模型的原始构建路径，例如：
```
\\?\C:\__w\1\s\CoreEngine\build\Models\Package\Model_Edge\Detector\Universal\ONNX\b512-SyncBN-x4_rpn_batch_quant_if.onnx
```
路径前缀 `C:\__w\1\s\` 是微软 CI/CD 构建机器路径（硬编码在 onemodel 中）。

### 路径校验（Encrypt，每个子模型一次）

Encrypt 加密的是路径字符串（144~160 字节，NUL padding），用于完整性校验，与模型内容解密**独立**，顺序上在 Decrypt 之后。

---

## ONNX 子模型保存（onnx_dump）

### 文件
- `onnx_dump.h` / `onnx_dump.cpp`

### 接口（两个函数）
```cpp
// 1. Decrypt 大块时调用：保存明文，尝试从 protobuf 中提取 .onnx 文件名
void OnnxDump_TrySave(const unsigned char *pbPlaintext, unsigned long cbPlaintext);

// 2. Encrypt 路径校验时调用：用路径中的 .onnx 文件名重命名上一个 _decrypt.bin
void OnnxDump_RenameLastIfMatch(const unsigned char *pbPlaintext, unsigned long cbPlaintext);
```

### 触发条件

**`det_BCryptDecrypt` 中**（保存所有大块解密数据）：
```cpp
if (BCRYPT_SUCCESS(st) && cbIV > 0 && pbOutput && pcbResult && *pcbResult > 512)
    OnnxDump_TrySave(pbOutput, *pcbResult);
```
- `cbIV > 0`：真实解密（排除 BCrypt 两阶段调用中的大小查询阶段）
- `*pcbResult > 512`：排除小数据块

**`det_BCryptEncrypt` 中**（路径校验到来时回溯重命名）：
```cpp
if (BCRYPT_SUCCESS(st) && cbIV > 0 && snapPTLen > 0 && cbInput <= 512)
    OnnxDump_RenameLastIfMatch(snapPT, snapPTLen);
```
- `cbInput <= 512`：Encrypt 路径校验的缓冲区很小（144~160 字节）

### 子模型保存调用时序

每个子模型在 oneocr.dll 中的处理顺序：
```
密钥派生A → BCryptDecrypt（模型 protobuf 数据，大块）→ DestroyKey
密钥派生B → BCryptEncrypt（路径字符串校验，小块）→ DestroyKey
```
- Decrypt 先发生，此时保存为 `NNNN_decrypt.bin`（或含 `.onnx` 名则直接命名）
- Encrypt 后发生，从路径明文中提取 `.onnx` basename，重命名上一个 `_decrypt.bin`
- 注意：**大部分** Decrypt 的 protobuf 是纯模型权重，不含路径字符串
- 只有少数配置块的明文内嵌了 `.onnx` 路径

### 文件名提取算法 (`find_onnx_basename`)

1. **从前往后**扫描明文中的 `.onnx` 子串（大小写不敏感）
2. 找到后，从 `.` 位置**往前**找路径分隔符 `\` 或 `/`
3. 遇到非打印 ASCII 字符（`< 0x20` 或 `> 0x7e`）立即停止反向扫描
4. 提取 basename 后验证所有字符都是可打印 ASCII
5. 返回第一个有效结果

**关键坑点**：
- 必须**从前往后**扫描，因为模型权重二进制数据尾部可能碰巧包含 `2e 6f 6e 6e 78`（`.onnx`）字节序列
- 反向扫描会命中这些假匹配，提取出乱码文件名（如 `+%`）
- Encrypt 明文前 8 字节是 protobuf header（`4a 1a 08 2b 25 00 00 00`），不是路径

### 输出目录
进程工作目录下的 `onnx_dump\`（即 `bin\onnx_dump\`），文件名格式：
```
bin\onnx_dump\
    0001_b512-SyncBN-x4_rpn_batch_quant_if.onnx    ← 从 protobuf 或 Encrypt 获得名字
    0002_decrypt.bin                                ← 纯权重块，无路径 → 后被 Encrypt 重命名
    0002_confidence_model_...onnx                   ← 重命名后
    ...
```

---

## oneocr 容器格式（解密后的子模型结构）

每个解密后的数据块并非裸 ONNX protobuf，而是 oneocr 自定义容器格式：

```
[8 bytes magic: 4a 1a 08 2b 25 00 00 00]   ← 所有子模型统一的固定头
[ONNX protobuf 或其他资源数据]         ← 实际内容
[1~15 bytes 尾部校验/签名数据]          ← 非 protobuf 的额外字节
```

### 特殊情况
- **目录表**（0001，22624字节）：前 16 字节是两个 uint64 LE（`0x5844`, `0x585c`），不使用通用 magic
- **尾部校验数据**：大小不固定（1~15字节），导致 `onnx.load` 直接加载失败

### 正确提取 ONNX 的方法
1. 跳过前 8 字节 magic header
2. 解析 protobuf 顶层 field tag，检查：
   - field number 必须在 ONNX ModelProto 有效集合中（{1,2,3,4,5,6,7,8,14,25}）
   - tag varint 必须是**最小编码**（非规范 varint 如 `a8 00` 代替 `28` 表示尾部校验数据开始）
3. 遇到无效 field 或非规范 varint 时截断，剩余是尾部校验数据
4. 用 `onnx.load_from_string()` 加载截断后的数据

### ONNX 模型验证结果（73 个 dump 文件）

| 类型 | 数量 | 说明 |
|---|---|---|
| 有效 ONNX | **34** | 全部 `onnx.load` 成功，含自定义算子 |
| 加载失败 | **0** | 无 |
| 非 ONNX 资源 | **39** | LogPrior表、字符映射表、配置数据、目录表 |

### 自定义算子域

| 域名 | 说明 |
|---|---|
| `com.microsoft.oneocr` | OneOCR 专用算子（如 `OneOCRFeatureExtract`） |
| `com.microsoft` | 微软通用算子 |
| `com.microsoft.mlfeaturizers` | ML 特征提取算子 |
| `com.microsoft.nchwc` | NCHW->NCHWc 布局算子 |
| `com.microsoft.experimental` | 实验性算子 |

### 模型分类

| 模型 | 大小 | producer | 用途 |
|---|---|---|---|
| `b512-SyncBN-x4_rpn_batch_quant_if.onnx` | ~11MB | onnx.quantize | 文字检测器 (Detector) |
| `checkpoint.040.onnx` | ~3.5MB | PyTorch | 识别器 (Recognizer) |
| `checkpoint.075_quant.onnx` | 0.3~3.7MB | PyTorch/onnx.quantize | 量化分类器 |
| `rejection_model_*.onnx` | 27KB×10 | pytorch | 拒绝模型（10个语言变体） |
| `confidence_model_*.onnx` | 29KB×12 | pytorch | 置信度模型（12个语言变体） |

### onnxruntime 加载
所有 ONNX 均因自定义算子 `com.microsoft.oneocr:OneOCRFeatureExtract` 未注册而无法用标准 onnxruntime 加载，需要 oneocr.dll 自带的 onnxruntime 提供自定义算子实现。

---

## 常见错误及解决

| 错误 | 原因 | 解决 |
|---|---|---|
| `initModel failed: -5` (`OCR_ERR_CREATE_PIPE`) | BCrypt hook 死锁导致 `CreateOcrPipeline` 超时/失败 | 修复所有 detour 为先调原始函数再加锁记日志 |
| `onnx_dump` 目录为空（v1） | 依赖 Encrypt 设置 pending name，但实际 Encrypt 在 Decrypt 之后 | 改为直接从 Decrypt 明文中搜索 `.onnx` 提取文件名 |
| `onnx_dump` 只有1个文件（v2） | `base.empty()` 时 `return` 跳过保存；大部分 Decrypt 块是纯权重不含路径 | 去掉 empty return，所有大块都保存为 `_decrypt.bin`，由后续 Encrypt 重命名 |
| 文件名乱码 `0001_+%`（v2） | `find_onnx_basename` 从**后**往**前**扫描，命中二进制权重中的假 `.onnx` 字节序列 | 改为**从前往后**扫描，第一个有效匹配即返回 |
| `verify_onnx.py` 0055 加载失败（v3） | 尾部校验数据以非规范 varint `a8 00` 开头，解码为合法 field 5，未截断 | 新增非规范 varint 检测：tag 字节数 > 最小编码字节数时视为尾部数据 |

---

## 构建命令

```powershell
# 配置（首次）
cmake -B build -G "Visual Studio 17 2022" -A x64

# 编译 Release
cmake --build build --config Release --target oneocr_wrapper oneocr_test

# 产物自动拷贝到 bin/ 由 cmake/copy_release_artifacts.cmake 处理
```

---

## BCrypt 日志文件

- 位置：进程工作目录下 `logdir\bcrypt_dump_<PID>.log`
- 本次分析的日志：`bin\bcrypt_dump_19900.log`（旧路径），共 **4709 次**调用，session 正常完成
- 新版本日志输出到 `bin\logdir\` 子文件夹（`log_open()` 中 `CreateDirectoryA("logdir", nullptr)` 自动创建）
