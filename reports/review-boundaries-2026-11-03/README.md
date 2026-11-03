# 第二次 Review 边界修复

本轮补齐三处边界，不修改推理引擎、模型、字表、原图或精度阈值。坐标仍为 `1 px`，角度 `0.1°`，置信度 `1e-3`。

## 修复

- `--compare-only` 必须指定独立、全新的 `--verification-output`，不再写回执行目录。历史输入路径、进程数、调度、耗时及构建记录保持原样，输出明确标记 `fresh_inference: false`。新推理会保存带文件摘要的 `execution.json`；旧报告通过完整索引校验。运行文件、模型、输入、基准或已封存的输出发生变化时拒绝复用。
- 非空模型输出目录缺少有效清单时，写入前即拒绝；不猜测遗留文件的版本，不删除旧内容。仍允许新建目录、已有空目录和同源且清单有效的目录。
- 比较误差前，基准和结果的角度须满足 `[0,360)`，词与手写/印刷置信度须满足 `[0,1]`。`360°` 不再作为 `0°` 合法通过，负数或大于 1 的置信度也不能借误差容限通过。合法范围内原有误差阈值不变。

## 验证范围

本轮验证以[上一轮已封存的五端真实推理](../review-fixes-2026-11-03/README.md)为依据。**这是新校验器对 690 项已保存记录的全量复核，不是重新执行 690 次推理。** 推理二进制与全部 155 个模型文件均先验证摘要一致，原始报告索引和内容保持原样。

| 环境 | 新校验器复核 | 原始执行 |
| --- | --- | --- |
| Windows x64 原生 | 138/138 | 上一轮全新推理 |
| Linux x64 原生 | 138/138 | 上一轮断网全新推理 |
| Windows Node 24 / Wasm | 138/138 | 上一轮全新推理 |
| Chrome 156 / Wasm | 138/138 | 上一轮全新推理 |
| Linux Node 24 / Wasm | 138/138 | 上一轮断网全新推理 |

每端仍含 135 项识别输出和 3 项预期拒绝行为，后者不计为识别成功。[最终全量复核报告](final/all-five/verification.json)包含各项结果。三份独立 Wasm 最终复核另见 `final/saved/`，保留原执行元数据，不改写原文件。根目录 `all-five/`、`saved/` 是开发阶段首次验证，`final/` 在交付清单冻结后重新只读复核；均不声称发生了新推理。

另外真实执行了原生、Node 和浏览器的拉丁样本识别，均通过；Node/浏览器仅为单样本运行，明确标记非完整回归。Node 的新执行封存记录又经过独立只读复核。相关数据见 `fresh/`。

23 个自动测试覆盖：范围合法性、原精度阈值、无清单非空目录、执行记录不可变、文件篡改、模型/基准/运行版本变化拒绝，以及真实 Linux 历史记录的跨主机复核。没有把旧的 3 进程 LPT 执行改成当前命令默认的 4 进程轮询，也未把 `/mnt/...` 输入路径改写成 Windows 路径。

## 重跑

```powershell
python test/compare-results-test.py
python test/model-preparation-test.py
python test/verification-state-test.py
python test/compare-only-test.py
python scripts/verify-recorded-parity.py --report reports/review-fixes-2026-11-03 --output work/new-five-record-verification
python scripts/run-wasm-parity.py --compare-only --output reports/review-fixes-2026-11-03/linux-node --verification-output work/new-linux-record-verification
```

复核输出目录必须为空且位于原执行目录之外。历史报告需连同索引一起保留；如运行资产或模型已变更，应使用新的执行目录进行全新推理。

macOS、Android、Linux ARM64 原生，Safari、Firefox、移动设备，以及 Node 20 完整集仍未验证；不扩大上一轮已验证范围，`engine.parity` 保持 `incomplete`。本轮仅本地提交，不 push、不发布、不上传模型。
