# OneOCR JavaScript + WebAssembly 回归

全量回归通过：**Node 与 Chrome 各 138/138**，分别覆盖 135 项识别输出及 3 项预期拒绝行为。两端全部 138 份原始结果/错误 JSON **逐字节相同**，见[交叉校验](evidence/node-vs-chrome.json)。没有替换模型、修改原图或放宽阈值。

| 环境 | 构建 | 运行及一致性 |
| --- | --- | --- |
| Wasm32 / Emscripten 4.0.8，WSL Linux 构建 | 通过 | 同一静态模块用于下列环境 |
| Windows Node 24.14.0 / V8 13.6.233.17-node.41 | 使用上述模块 | [138/138 通过](node/summary.json) |
| Windows Chrome 156.0.8078.79 / 浏览器 Worker | 使用上述模块 | [138/138 通过](chrome/summary.json) |
| Linux Node 20.18.0 / 无网络命名空间 | 使用上述模块 | [106 行书页通过](offline/summary.json)，未执行 Linux 全量集 |
| Safari、Firefox、移动设备及其他 JS 引擎 | 未单独验证 | 未验证 |

实现提交：`b0153c0`。使用 Photos 2026.11090.22001.0 / OneOCR 11.0.15 的冻结 DLL 基准；模型来源 SHA-256 为 `f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`。生产模型、字表、索引映射和比较器均未修改。

## 范围与标准

全量为 138 项：全部 110 张样本、固定 27 项和 106 行书页。135 项比较真实识别 JSON，3 项核对原图的预期拒绝行为；拒绝不计为识别成功。文字、行词结构、阅读顺序、手写类别一致，坐标误差不超过 1 像素、角度不超过 0.1 度、置信度不超过 `1e-3`。使用原有 `scripts/compare-results.py`，没有放宽阈值。

两端各比较了 2,837 行、11,561 个词。[实际最大误差](evidence/numerical-limits.json)均为：行框 `0 px`，词框 `0.00048828125 px`，角度 `1.1920928955078125e-7°`，词置信度 `0.0009335875511169434`，手写/印刷置信度 `0`。这些是相对冻结 DLL 的误差；Node 与 Chrome 之间的原始输出另按字节核对。

## 已完成的检查

- 106 行书页在 Windows Node 和隔离 Linux Node 中均通过原阈值比较。
- [隔离运行](offline/summary.json)：Linux Node 20.18.0，`unshare --net` 创建无网络命名空间；临时目录仅含 Wasm、JS、模型和原图，无 DLL、SO、EXE、dylib，也未调用 Python 识别后端。
- [接口测试](evidence/wasm-api.log)：真实模型、错误码、非法图像、损坏模型、BGRA 步长、重复关闭、Worker 排队、取消、超时和 DSH 插件调用。
- [浏览器示例](evidence/demo/summary.json)：选图、取消、重新加载后识别；输出通过[严格比较](evidence/demo/parity.json)。
- [干净 Git 克隆](evidence/clean-checkout/summary.json)：从已提交文件加载模型与 Wasm，通过拉丁样本比较；[CLI](evidence/clean-cli.parity.json) 从无关工作目录运行，不依赖 npm 安装或研究目录。
- [Windows 原生入口检查](evidence/native-delivery.json)：全部交付文件校验及 106 行书页比较通过。

## 数值适配与复现

Wasm 静态链接同一份 C++20 引擎、ORT 1.23.2、OpenCV 4.10.0 和 ICU 74.2，工具链固定为 Emscripten 4.0.8。JS 仅负责文件加载、内存释放和 Worker 通信；图像使用同一 C++ 解码器处理，未经过 Canvas 转码或自动修图。

初版 Wasm 在书页上存在 98 个比较超限项。修复 OpenCV 最近偶数舍入后剩 37 项；重现 AVX-512 NCHWc 卷积的通道块顺序和精确 float32 FMA 后剩 20 项；补齐 LSTM sigmoid/tanh 的融合多项式运算后书页无超限项。详见[各阶段原始比较](evidence/book-numeric-stages.json)、[卷积候选顺序](evidence/conv-order-candidates.json)和[生产张量对比](evidence/latin-production-tensors.json)。这次修改未替换模型或系数。

精确 FMA 已与独立的标量 IEEE `fmaf` 比较边界矩阵和 1,000,000 组随机值；SIMD 与尾部图像转换也有实测。完整构建日志以 gzip 保存于 `evidence/`，解压前后的对应摘要见 [compressed-logs.json](evidence/compressed-logs.json)。

重新观察内部输出时，可用 `ONEOCR_TRACE_DIR` 获取原生行张量、`web/trace.mjs` 获取 Wasm 张量，再用 `scripts/prepare-wasm-numeric-probe.py` 生成只供诊断的图。它保留原权重与算子、暴露中间输出，并依据冻结行张量固定 `seq_lengths`；产物限定于 `work/`，不会覆盖 `models/`。`oneocr_tensor_probe` / 可选的 `ONEOCR_WASM_RESEARCH_PROBE` 导出中间张量后，可用 `scripts/probe-wasm-conv-order.py` 比较累加候选。

## 使用与重跑

```powershell
node web/cli.mjs ocr-book.jpg --output work/wasm-book.json --quiet
node test/wasm-test.mjs
python scripts/run-wasm-parity.py --output work/wasm-node-new --jobs 4
python scripts/run-wasm-parity.py --browser "C:/Program Files/Google/Chrome/Application/chrome.exe" --output work/wasm-chrome-new --jobs 4
```

浏览器回归工具使用 Playwright Core 1.55.0，所有请求限定到临时的本地静态站点。模型和图像不会上传。每个独立进程保留输入清单、原始结果、耗时、环境版本，最终报告汇总全部 138 项。新一轮运行必须使用空输出目录。

构建、API、TypeScript、Worker、示例和插件入口见 [Wasm 接入说明](../../docs/wasm.md)。WebAssembly SIMD128、BigInt、单线程和最大 4 GiB 线性地址空间的配置不依赖 relaxed SIMD 或 SharedArrayBuffer。

精确数值适配明显慢于原生实现，长文档需要数分钟。并行任务中的耗时包含 CPU 争用，不能当作独占 CPU 的性能基准；可用 `--jobs 1` 降低内存需求。未实测的浏览器、移动设备及其他 JS 引擎仍未验证，`engine.parity` 保留全项目的 `incomplete` 状态。

本次最慢样本为总览拼图：Node `362.654 s`、Chrome `351.644 s`。Node 的 4 个工作进程中最大进程峰值 RSS 为 `720,412,672` 字节（约 `687 MiB`），并非多进程同时占用量；浏览器测试未采集 RSS。每项耗时与 Node 内存记录保存在相应的 `actual/measurements.json`。

模型权利仍归微软；项目和工具的开源许可不覆盖微软权重。此次仅本地提交，没有 push、发布或上传模型。
