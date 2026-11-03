# Review 修复与增强严格回归

**五个环境的完整回归全部通过，各 138/138，合计 690 项。** 实现已提交为 `aa52f6b`。本轮使用补齐计数、索引与类型检查后的严格比较器，进行了全新推理；中断与部分运行另外保留，不计为通过。

## 修复范围

1. 模型准备默认读取活动 DLL 基准绑定的来源，当前为 Photos 2026.11090.22001.0 的 `oneocr.onemodel`；在写入前核对 SHA-256，并拒绝将其他版本写入正式 `models/` 或覆盖来源不同的既有目录。[真实准备验证](evidence/model-preparation.json)得到 155 个逐字节相同的文件，正式模型未变。
2. 严格比较器补齐 `line_count`、`word_count`、行/词 `index`、字段类型及非有限数值检查，基准本身也必须合法。[四种篡改复现](evidence/metadata-mutation-tests.json)均被拒绝。坐标 `1 px`、角度 `0.1°`、置信度 `1e-3` 的原阈值没有改变；主机文件路径和后端专属 `engine` 元数据按设计不要求一致。
3. JS 正确规范化相对 `runtimeUrl`，再解析自定义 `wasmUrl`；Node Worker 接受 `file:` URL 字符串及 URL 对象，并保留原文件系统路径用法。[两种 Review 复现](evidence/url-reproductions.json)均由失败变为通过；[接口测试](evidence/wasm-api.log)使用真实模型验证识别、步长、生命周期、排队、取消、超时及插件调用。
4. 原生回归入口为 Python 子进程显式开启 UTF-8，避免 Windows GBK 日志输出在 emoji 用例中断。浏览器测试工具使用独立的等待预算，支持并行负载下的慢任务，并在关闭前采集当前浏览器进程树的 OS 峰值工作集。产品默认超时和精度阈值均保持不变。

## 固定资产与范围

使用原 Photos 2026.11090.22001.0 / OneOCR 11.0.15 的冻结基准。模型来源 SHA-256 仍为 `f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`，模型、字表、映射、原图和原 DLL 基准均未替换。

每个完整集均为 138 项：110 张样本、27 项固定集及 106 行书页。其中 135 项比较识别 JSON，3 项为原接口拒绝行为，不计为识别成功。使用新比较器进行全新推理，不以重新比较旧输出代替完整回归。

| 环境 | 本轮状态 | 证据 |
| --- | --- | --- |
| Windows x64 原生 | 138/138 通过 | [summary](windows-native/summary.json) |
| Linux x64 原生，无网络命名空间 | 138/138 通过 | [summary](linux-native/summary.json) |
| Windows Node 24.14.0 / Wasm | 138/138 通过 | [summary](windows-node/summary.json) |
| Chrome 156 / Wasm Worker | 138/138 通过 | [summary](chrome/summary.json) |
| Linux Node 24.14.0 / Wasm，无网络命名空间 | 138/138 通过 | [summary](linux-node/summary.json) |
| Linux Node 20 完整 Wasm 集 | 未完成；只保留部分运行 | `linux-node20-partial/` |
| Linux ARM64、macOS、Android 原生；Safari、Firefox、移动设备 | 未验证 | 保留原配置与示例，不声称构建或运行通过 |

Linux Node 24.14.0 的下载来源、官方摘要校验与版本见 [runtime](evidence/linux-node-runtime.json)。Node 本身作为标准宿主运行环境单独安装，不嵌入 OCR Wasm。

每端比较 2,837 行、11,561 个词，所有计数和索引均合法。相对原 DLL 的[实际最大误差](evidence/numerical-limits.json)：行框为 `0 px`，词框为 `0.00048828125 px`；原生角度误差为 `0`，Wasm 为 `1.1920928955078125e-7°`；原生词置信度误差为 `1.7285346984863281e-6`，Wasm 为 `0.0009335875511169434`；手写/印刷置信度误差均为 `0`，全部符合原阈值。

[跨运行时复核](evidence/cross-runtime.json)：Windows Node 与 Chrome 的 138 份结果/错误 JSON 逐字节相同；Linux Node 的全部识别字段与 Windows Node 逐值相同，原始 `file` 字段保留 `/mnt/...` 主机路径，未为制造字节相同而改写输出。三端的逐对严格比较也全部通过。

## 中断记录

- `windows-native-attempt1/`：原生识别比较未出现差异，但旧子进程日志编码在 emoji 文件名中断；修复 UTF-8 后已从空目录完整重跑。
- `chrome-attempt1/`：并行负载下请求超过旧测试工具 600 秒等待时限，Worker 依约终止；后续错误未作为识别成功。修复测试工具等待预算后从空目录重跑，未更改产品默认超时或数值阈值。
- `linux-node20-partial/`：因性能成本改用与 Windows 相同版本的 Linux Node 24 运行完整集。Node 20 的部分结果保留，不作全量验收依据；历史独立书页记录仍在上一轮报告。

## 接口与换机检查

原生 C ABI 的文件/BGRA 等价性、并发实例、重复调用、错误资源及 `max_lines=1` 检查均通过：[Windows](evidence/windows-native-api.json)、[Linux](evidence/linux-native-api.json)。

从 `aa52f6b` 干净 Git 克隆运行 [Node 回归入口](evidence/clean-checkout/summary.json)，并从无关工作目录启动 [CLI](evidence/clean-cli.parity.json)；无需克隆原机器的 `work/`、`node_modules/` 或编译缓存。浏览器取消及重新加载的实际 UI 测试另保存在 `evidence/demo/`。

[独立目录断网验证](offline/summary.json)使用 Linux Node 24.14.0：目录中仅保留 Wasm/JS、同一份模型和原图，不含 DLL/SO/EXE/dylib，106 行书页通过新比较器。完整 Linux 原生及 Linux Node 回归还分别在无网络命名空间中运行。

## 重跑

```powershell
python test/compare-results-test.py
python test/model-preparation-test.py
node test/wasm-test.mjs
python scripts/run-native-parity.py --binary artifacts/win32-x64/bin/ocr.exe --output work/native-new
python scripts/run-wasm-parity.py --jobs 4 --output work/node-new
python scripts/run-wasm-parity.py --browser "C:/Program Files/Google/Chrome/Application/chrome.exe" --jobs 3 --output work/chrome-new
```

Linux 使用 `python3`，原生二进制为 `artifacts/linux-x64/bin/ocr`，Wasm 的 `--node` 可指定本地 Linux Node。可用 `unshare -n` 在独立无网络命名空间中执行，失败不会回退联网。`--schedule-from` 只利用同模型、同输入摘要的历史耗时分配工作量，不复用识别输出。所有新推理须使用空输出目录。

浏览器 OS 内存指标是各个仍存活进程的历史峰值工作集；不同进程的峰值不必同时出现，因此峰值之和只表示上界。并行任务耗时包含 CPU 争用，不是独占 CPU 的性能基准。

本轮各 Wasm 环境最慢用例均为总览拼图：Windows Node `671.620 s`、Chrome `523.849 s`、Linux Node `601.730 s`。Node 工作进程中的最大峰值 RSS 分别为 `728,477,696`、`1,046,966,272` 字节；Chrome 记录到的最大单进程峰值工作集为 `1,011,294,208` 字节。这些指标不是整个并行测试同时占用量。长图耗时可能超过产品默认超时，接入方须按实际负载显式配置；测试工具等待预算不是精度阈值。

模型权利仍归微软，未改标为 MIT。本轮仅本地提交，没有 push、发布或上传模型。`engine.parity` 继续为 `incomplete`，不将有限输入与已测环境的通过扩展为任意输入、参数、硬件或平台均完全等价。
