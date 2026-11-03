# 当前输入绑定与参考缓存校验修复

修复本次 Review 的两个 P2，保留当前 Photos 模型、数值运行文件和严格比较器。坐标阈值仍为 `1 px`，角度 `0.1°`，置信度 `1e-3`；文字、阅读顺序、词切分与手写类型仍要求一致。

## 修复内容

- 五环境与单环境入口共用 `scripts/regression_inputs.py`，从当前文件重新生成范围和输入摘要，再与原执行比较。`samples/` 所有图片必须登记且逐个核验，固定语料登记项和书页也重新计算摘要。修改样本、固定语料或书页，新增未登记样本，增删固定语料登记项，均不能用旧输出宣称通过。原执行目录的路径、调度和摘要保持原样；五环境报告新增当前输入摘要、清单摘要及校验器来源记录。
- `scripts/run-reference.py` 使用 `reference_inputs.py`，在导入参考引擎、创建缓存或写入结果之前，先完整校验源图、字表和已有缓存。缺失必需的模型登记、异版本或损坏的缓存会被拒绝，原缓存保留。新文件使用独立副本，不创建指向正式模型的可写硬链接；`--cache-dir` 可指定全新缓存。输出旁的 `.inputs.json` 记录来源模型、清单及全部 20 个参考输入的摘要。参考项目的历史语言别名仍明确映射到实际资源，不根据文件字节大小猜测角色。

## 验证

**本轮全量复核 690 项已保存执行记录，没有重新执行 690 次推理。** 另外保存 Node/Wasm 与参考实现各一个真实样本运行，明确属于单样本检查。

| 环境 | 最终严格复核 | 原始输出范围 |
| --- | --- | --- |
| Windows x64 原生 | 138/138 | 135 项识别输出＋3 项预期拒绝 |
| Linux x64 原生 | 138/138 | 135 项识别输出＋3 项预期拒绝 |
| Windows Node 24 / Wasm | 138/138 | 135 项识别输出＋3 项预期拒绝 |
| Chrome 156 / Wasm | 138/138 | 135 项识别输出＋3 项预期拒绝 |
| Linux Node 24 / Wasm | 138/138 | 135 项识别输出＋3 项预期拒绝 |

拒绝项不计为识别成功。[最终五环境报告](final/all-five/verification.json)包含逐项结果及当前 138 项输入摘要。三个 Wasm 环境也分别经单环境入口复核通过，见 `final/saved/`。根目录 `all-five/`、`saved/` 为开发阶段首次复核；`final/` 在交付说明及清单冻结后复核，均标记 `fresh_inference: false`。

此前三份新格式 Node/浏览器单样本记录，以及本轮新 Node 记录，分别通过独立只读复核，见 `sealed/`。本轮 `fresh/node/` 为真实 Wasm 识别并通过严格比较，明确标记 `full_regression: false`。`fresh/reference.json` 和 `fresh/reference.inputs.json` 为参考实现的实际运行及模型输入凭据；它仍缺少原 DLL 的手写等能力，不计作 DLL 完整等价通过。

40 个自动测试全部通过，日志在 `evidence/`：比较器 8 项、模型目录 7 项、执行记录 10 项、只读 CLI 2 项、归档 CLI 6 项、参考缓存 7 项。新增测试覆盖两个真实复核入口对隔离输入副本的检查，包括图片修改、样本新增、固定语料登记项增删；正常副本也必须通过，避免负例因测试环境本身损坏而误通过。缓存测试覆盖异版本权重、改写字表、源文件损坏、清单升级、缺少必需登记及副本独立性。

另在完整隔离检出副本中，实际执行 5 项 CLI 负例：图片被修改或增加样本时，两种复核入口都非零退出；分类器替换为权重不同但有效的 ONNX 时，参考入口在运行前拒绝。全部未发布输出，详见 [负例记录](evidence/isolated-cli-negatives.json)。真实 Wasm API、Worker、取消、超时和插件接入测试也通过。

三个原始报告分别有 1810、870、444 个索引文件，逐文件校验均未变化；252 个交付文件及 155 个模型文件摘要有效。缓存 Wasm 构建通过，核心摘要仍为 `70ad89615c37319c83e74935f8cf5dc9b039d503fc2328e22251ecb98a47739e`；仅验证脚本来源摘要和交付说明更新。[完整性凭据](evidence/integrity-checks.json)及压缩构建日志保留在 `evidence/`。

回归范围为 `samples/` 全部 110 张图片、固定清单中的 27 项及书页。研究目录保留的历史 `bengali-36.png` 不在固定清单中，不计作已验证用例；本轮没有增删原图、模型或固定集。

## 重跑

```powershell
python test/compare-results-test.py
python test/model-preparation-test.py
python test/verification-state-test.py
python test/compare-only-test.py
python test/archive-verification-test.py
python test/reference-inputs-test.py
node test/wasm-test.mjs
python scripts/verify-recorded-parity.py --report reports/review-fixes-2026-11-03 --output work/new-input-bound-verification
python scripts/run-wasm-parity.py --compare-only --output reports/review-fixes-2026-11-03/linux-node --verification-output work/new-input-bound-linux
python scripts/run-reference.py research/case/corpus/latin-36.png work/new-reference.json --cache-dir work/new-reference-inputs
```

复核输出目录须独立且全新。参考实现命令需要已准备的固定参考源码与研究依赖，使用项目研究 Python 亦可；模型不下载、不替换。Photos 来源 SHA-256 仍为 `f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`。

## 未完成与未验证

退化曲线备用求解、极长行多边形裁剪、更多非默认选项探针，以及中文和其他文字的独立真实手写集仍未完成。原生 macOS、Android、Linux ARM64，Safari、Firefox、移动设备，以及 Node 20 完整集仍未验证；不扩大既有构建或运行通过的范围。`engine.parity` 保持 `incomplete`，完整跨平台目标未完成。

本轮源码、交付元数据和报告全部本地 commit；不 push、不发布、不上传模型。提交后更新完整离线 Git bundle 与源码 ZIP，后者仍不包含模型及编译产物。
