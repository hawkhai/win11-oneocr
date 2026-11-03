# 执行证据绑定与结果清单修复

本轮修复 Review 发现的两处问题，不修改模型、推理源码、运行文件或数值阈值。坐标仍为 `1 px`，角度 `0.1°`，置信度 `1e-3`。

- 新格式 `execution.json` 不能再用自身重复的构建、用例、基准摘要、调度或浏览器等待预算覆盖历史。加载时将这些值与摘要校验通过的 `execution-build.json`、`execution-cases.json`、`cases.json` 和 `summary.json` 逐项核对，再使用原件的值。布尔、整数和浮点数不会因 Python 相等规则而混同；格式版本只接受整数 `1`。有外层索引时，同时核验 `execution.json` 本身。
- 单环境和五环境复核现在共用结果清单检查。成功结果旁混入错误 JSON、拒绝项混入成功结果、结果缺失或出现未登记 JSON，都会失败；Windows/Linux 原生结果也适用。失败发生在输出报告之前，不能留下通过报告。

## 验证结果

**本轮为 690 项历史记录的全量复核，另有 1 项全新推理；没有重新执行 690 次推理。**

| 环境 | 严格复核 | 数据来源 |
| --- | --- | --- |
| Windows x64 原生 | 138/138 | 原封存执行记录 |
| Linux x64 原生 | 138/138 | 原封存执行记录 |
| Windows Node 24 / Wasm | 138/138 | 原封存执行记录 |
| Chrome 156 / Wasm | 138/138 | 原封存执行记录 |
| Linux Node 24 / Wasm | 138/138 | 原封存执行记录 |

每环境含 135 项识别结果和 3 项预期拒绝，拒绝项不计为识别成功。[五环境报告](all-five/verification.json)保留逐项结果。三个 Wasm 环境还分别通过独立入口复核，见 `saved/`。两份先前新格式的 Node/浏览器单样本执行记录通过封存一致性校验，见 `sealed/`。

Windows Node 24 又真实执行 `corpus-latin-36`，严格比较通过，并由只读入口再次验证新写入的 `execution.json`，见 `fresh/node/` 和 `fresh-verification/node/`；该次运行明确标记 `full_regression: false`。

30 个自动测试全部通过，日志在 `evidence/`：比较器 8 项、模型目录保护 7 项、执行记录绑定 10 项、只读 CLI 2 项、完整归档 CLI 3 项。归档测试在临时副本中实际添加冲突与额外 JSON，验证两个入口均非零退出且不发布输出；也验证正常归档在两个入口通过。原始归档不参与写入。

上一轮两个报告分别有 1810、870 个索引文件，本轮逐文件核验均保持原样；155 个模型文件和 252 个交付文件摘要有效。Wasm 缓存构建通过，核心 SHA-256 仍为 `70ad89615c37319c83e74935f8cf5dc9b039d503fc2328e22251ecb98a47739e`。构建只更新验证脚本的来源摘要及交付说明，数值运行文件相同。[完整性记录](evidence/integrity-checks.json)给出摘要与检查范围；压缩构建日志为 `evidence/wasm-build.log.gz`。

## 重跑

```powershell
python test/compare-results-test.py
python test/model-preparation-test.py
python test/verification-state-test.py
python test/compare-only-test.py
python test/archive-verification-test.py
python scripts/verify-recorded-parity.py --report reports/review-fixes-2026-11-03 --output work/new-bound-five-verification
python scripts/run-wasm-parity.py --compare-only --output reports/review-fixes-2026-11-03/linux-node --verification-output work/new-bound-linux-verification
```

复核输出使用原执行目录以外的全新目录，不能改写执行历史。回归基准仍绑定 Photos `2026.11090.22001.0` 模型，来源 SHA-256 为 `f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`，不切换模型或放宽标准。

macOS、Android、Linux ARM64 原生，Safari、Firefox、移动设备，以及 Node 20 完整集继续标记未验证；`engine.parity` 保持 `incomplete`。本轮源码、交付元数据及报告只做本地 commit，不 push、不发布、不上传模型。离线 Git bundle 与源码 ZIP 为提交后的本地派生产物。
