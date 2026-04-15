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
│   ├── bcrypt_dump_<PID>.log
│   └── onnx_dump/          # 解密出的 ONNX 子模型文件
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

### 接口
```cpp
void OnnxDump_TrySave(const unsigned char *pbPlaintext, unsigned long cbPlaintext);
```

### 触发条件（在 `det_BCryptDecrypt` 中）
```cpp
if (BCRYPT_SUCCESS(st) && cbIV > 0 && pbOutput && pcbResult && *pcbResult > 512)
    OnnxDump_TrySave(pbOutput, *pcbResult);
```
- `cbIV > 0`：真实解密（排除 BCrypt 两阶段调用中的大小查询阶段）
- `*pcbResult > 512`：排除小数据块

### 文件名提取
从明文中搜索最后一个 `.onnx` 子串，反向找路径分隔符提取 basename。

### 输出目录
进程工作目录下的 `onnx_dump\`（即 `bin\onnx_dump\`），文件名格式：
```
bin\onnx_dump\
    0001_b512-SyncBN-x4_rpn_batch_quant_if.onnx
    0002_confidence_model_calibrated_2023_12_20_11_14_35.onnx
    ...
    XXXX_decrypt.bin   ← 明文不含 .onnx 时（如目录表）
```

---

## 常见错误及解决

| 错误 | 原因 | 解决 |
|---|---|---|
| `initModel failed: -5` (`OCR_ERR_CREATE_PIPE`) | BCrypt hook 死锁导致 `CreateOcrPipeline` 超时/失败 | 修复所有 detour 为先调原始函数再加锁记日志 |
| `onnx_dump` 目录为空 | 依赖 Encrypt 设置 pending name，但实际 Encrypt 在 Decrypt 之后 | 改为直接从 Decrypt 明文中搜索 `.onnx` 提取文件名 |

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

- 位置：进程工作目录下 `bcrypt_dump_<PID>.log`
- 本次分析的日志：`bin\bcrypt_dump_19900.log`，共 **4709 次**调用，session 正常完成
