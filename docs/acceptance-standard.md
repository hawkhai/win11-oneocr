# OneOCR 当前验收标准（铁律）

2026-11-03，用户明确要求“只要逆向的 C++ 版本和原版绝对一致就可以了。网页汇编不用绝对一致”，并进一步确认数值口径为“沿用此前严格阈值”。这条规则取代此前将五个环境全部严格一致作为联合验收门槛的要求。未经用户明确调整，不得改变。

| 对象 | 与原 DLL 的一致性要求 | 是否阻塞 C++ 验收 |
|---|---|---|
| 逆向 C++ 原生引擎，包括各原生平台 CLI、动态库及适配接口 | 按原有严格标准逐项对齐 | 是；未通过或未验证须明确记录 |
| JS / WebAssembly，包括 Node 与浏览器 | 不要求绝对一致；比较结果保留为诊断记录 | 否 |

## C++ 的严格口径

使用相同模型版本、原始图像和参数，对照原微软 DLL。文字、阅读顺序、行词切分、手写类型、计数、索引及既有字段语义必须一致；坐标最大误差 **≤1px**，图像角度误差 **≤0.1°**，置信度误差 **≤1e-3**。拒绝 NaN/Inf、非法类型和范围。这里的“绝对一致”是用户确认的既有严格验收，不是另设零误差或 JSON 文本逐字节相同。

继续使用 `scripts/compare-results.py`，保留其对宿主文件路径及后端 engine 元数据的既有排除规则。不得换模型、放宽 C++ 阈值、漏跑登记样本/参数、伪造输出或把错误样本改计识别成功。原 DLL 的预期拒绝单独核对错误契约；共同有效参数范围之外的原生扩展不冒称 DLL 等价。

当前登记范围仍为 126 张样本、27 项固定语料、书页和 67 组参数（66 非默认与一个默认对照），共 221 项/原生环境。此规则不减少 C++ 回归范围，也不豁免上一轮 Review 发现的 Photos 交集测试断言缺口。新增样本和参数须加入同一原生严格回归。

Windows 原生完整新推理：

```powershell
python scripts/run-native-parity.py --binary artifacts/win32-x64/bin/ocr.exe --output work/fresh-native-acceptance
```

Linux 原生完整新推理：

```sh
python3 scripts/run-native-parity.py --binary artifacts/linux-x64/bin/ocr --output work/fresh-native-acceptance-linux
```

每个平台分别报告构建与运行结果。现有已验证原生环境为 Windows x64 / Linux x64；Linux ARM64、macOS x64/arm64、Android arm64-v8a 仍未验证，不得因为 WebAssembly 能运行就标记对应原生平台通过。整体目标继续保留 incomplete，直到剩余目标有实测证据。

## WebAssembly 与历史记录

`scripts/run-wasm-parity.py` 和五环境 `scripts/verify-recorded-parity.py` 仍可用于严格诊断及历史封存复核。它们的 PASS/FAIL 描述该次比较结果；WebAssembly 部分 FAIL 不自动等于 C++ 验收失败。不得再要求先消除 Wasm 的所有数值差异才能完成原生 C++ 工作。

不额外编造一套未经用户确认的 Wasm 数值阈值。实际差异、崩溃、加载错误和接口行为仍须如实记录，不能声称未通过的结果一致。既有取消、超时、资源管理和接口契约也不因取消绝对一致要求而改写成虚假成功。

过去 1105/1105、旧五环境联合结果及其源码摘要、报告索引均保留原样，作为当时执行的证据；它们不再定义当前的联合验收门槛。当前权威入口为根目录 `AGENTS.md`、本文与 `research/acceptance-policy.json`。离线包和源码 ZIP 应随最终提交更新，源码 ZIP 必须包含根目录铁律。
