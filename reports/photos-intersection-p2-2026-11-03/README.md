# Photos 交集断言 P2 修复与 C++ 原生回归

本轮按[验收铁律](../../docs/acceptance-standard.md)执行：C++ 原生与固定 Photos DLL 逐项严格对齐；Wasm 仅作诊断。运行结果汇总见 `summary.json`，完整原生输出与比较记录分别在 `windows-native/`、`linux-native/`。记录基准是现有 Photos 11.0.15 版本绑定的 DLL 输出，本轮未冒称重新采集原 DLL。

**Windows/Linux 原生全新严格回归各 221/221，合计 442/442 通过**：436 项识别输出严格一致、6 项预期拒绝契约通过。CTest 两端各 26/26，Python 完整套件 65/65。独立保存结果复核为 442/442，不冒充另一次新推理。模型和比较器保持原样，完整跨平台目标仍为 incomplete。

## 修复与负例

此前新增 Photos 路径只更新最大面积误差，未将超差计入失败；Review 在隔离副本把一项期望面积改错 1e9，测试仍退出 0。现在旧 oracle 与新增 Photos oracle 共用一个实际断言：实际值、期望值、差值都必须有限，且误差不得超过既有 **`.01f` 面积容差**。超差和非有限值均增加失败计数，正常测试因此退出非零。这里的面积契约与 OCR 坐标容差不是同一个单位，两个既有阈值都没有改变。

测试从第一项真实 Photos 多边形对注入标量数值，经过同一断言和失败计数，检查恰好得到预期失败数；没有改写冻结 oracle，也没有修改几何输入或生产识别代码。若移除 Photos 路径的断言，负例会失败。

| 类型 | 用例 |
|---|---|
| 13 个负例 | 正/负方向刚超过 `.01f`、Review 的 1e9 差值、实际值 NaN/+Inf/-Inf、期望值 NaN/+Inf/-Inf、双 NaN、同号双 +Inf/-Inf、有限数相减溢出 |
| 3 个正例 | 正/负方向恰好 `.01f`、相等的大有限数 |

Windows/Linux CTest 各包含原有 10 项与新增 16 项，共 26 项。原始日志和每个用例的 stdout/stderr 在 `evidence/*ctest*.log`，其中负例的 PASS 表示预期错误被检出，不是接受错误数值。

两套原生默认 NMS 测试均核对 54,748 项多边形 probe 和 79 组识别 NMS，面积最大误差为 0，见 `evidence/unit-tests.json`。Windows 详细 CTest 日志的本地化日期头使用 GB18030，Linux 使用 UTF-8；归档保留原始字节，编码也随记录保存。

另外在隔离 oracle 副本上重现 Review 的原始步骤：未改数据时默认 NMS 进程退出 0；将 `calligraphy-270.json` 的一项期望面积从 0 改为 1e9 后，同一个默认进程现在退出 **1**，并报 `Intersection mismatch Photos`。这项检查没有使用负例自验证参数，见 [default-exit-repro.json](evidence/default-exit-repro.json)。Linux Ninja 构建后可复现：

```sh
python3 scripts/reproduce-nms-oracle-rejection.py --output work/fresh-photos-rejection
```

## 验收范围与身份

每个原生环境完整运行 **126 张样本 + 27 项固定语料 + 书页 + 67 组参数 = 221 项**。每端 218 项为识别输出，另 3 项为原有预期拒绝的错误契约。参数包含 66 个非默认组合与一个默认对照，真实手写来源和变换仍见[原有来源清单](../../samples/options-handwriting/provenance.json)。本轮没有减少范围或新增豁免。

- 实现/测试修复提交：`75cc751dae63892eaba10ca28b4a42e7735591d6`。
- 模型 SHA-256：`f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`；模型 manifest SHA-256：`8e6d5b973aad14590890ce78de39e5414b7297df4350d65e7a2990ab9b9c929a`。
- 原 DLL SHA-256：`0a5e99041af902d6825f0af04ab25e62d2e2378ace69d10175f0dcc22816aae0`。
- 比较器 SHA-256：`06d137cc93680f766031d783da9526064808fea1135124348e180bb8607f4869`，没有修改。
- OCR 要求：文字、顺序、切分、类型、计数、索引及既有字段语义一致；坐标 ≤1px、角度 ≤0.1°、置信度 ≤1e-3，拒绝非法字段和非有限值。

`evidence/bindings.json` 记录原始四资产与实现提交；`evidence/execution-cases.json` 将图像、请求参数、预期错误和黄金输出摘要绑定到本轮范围。`evidence/source-bindings.json` 保存修复与验证源码的标准化摘要。原生运行文件和独立模型仍由统一 delivery manifest 管理，ORT 固定 1.23.2；本次只修改测试断言及测试注册，没有换模型或修改生产识别算法。

## 新推理、独立复核与 Wasm 诊断

两套原生目录为本轮完整新推理；Linux 命令运行在 `unshare --net` 中。封存后用 `scripts/verify-native-results.py` 独立重新比较保存结果，输出到[单独的原生复核目录](../photos-intersection-review-2026-11-03/README.md)，不会改写本轮原始执行，也不依赖 Wasm 的数值比较结果。

Wasm 仅在 Windows Node 24 对一张中文手写旋转图和一组真实手写非默认参数作抽查，结果见 [diagnostics/wasm/summary.json](diagnostics/wasm/summary.json)。该目录明确标记 `diagnostic_only=true`、`wasm_is_acceptance_gate=false`，不将有限抽查称为 Wasm 全量回归。

本轮未重建 Wasm；所保留资产的构建 manifest 仍指向先前构建的测试/CMake 源码快照，没有伪造为包含本次新增原生测试的 Wasm 构建。生产识别源码、JS/Wasm 二进制和模型未变。未来需 Wasm 新构建时可独立执行其构建流程，不能将它当作本次 C++ 验收前置条件。

Python 测试原始结果见 `evidence/python-full.json`。新增原生验收入口的负例覆盖：坐标超差仍失败、成功输出与错误文件冲突仍拒绝、Wasm 诊断出现差异不阻塞 C++。

## 复现与交付

```powershell
cmake --build build/source-runtime --config Release
ctest --test-dir build/source-runtime -C Release --output-on-failure
python scripts/run-native-parity.py --binary artifacts/win32-x64/bin/ocr.exe --output work/fresh-native-windows
python scripts/run-python-tests.py --report work/fresh-python-tests.json
python scripts/verify-native-results.py --report reports/photos-intersection-p2-2026-11-03 --output work/fresh-native-review
```

Linux 使用 `build/linux-x64` 与 `artifacts/linux-x64/bin/ocr`，完整构建前提见[原生构建说明](../../docs/portable.md)。`FILES.sha256.json` 覆盖本报告全部文件，源码 ZIP 依索引收齐。历史五环境报告、模型、样本和原有证据索引保持原样。

源码、运行资产和报告本地提交；最终干净 HEAD 后重新生成 bundle、源码 ZIP 与 `artifacts/OFFLINE_PACKAGE.json`，该忽略的侧车记录实际最终提交和包摘要，避免包含自身提交的循环。未 push、发布或上传。

Linux ARM64、macOS x64/arm64、Android arm64-v8a 原生构建/运行仍未验证；Safari、Firefox、移动浏览器与 Node 20 全范围也未在本轮验证。现有 Linux x64 预编译包要求 glibc ≥2.43。有限图像、参数、书写者和其他手写文字类别仍有覆盖缺口，整体目标保持 incomplete。
