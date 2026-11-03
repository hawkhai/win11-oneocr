# 非默认参数与真实中文手写严格回归

本轮固定原 DLL、模型和严格比较器，在五个环境分别全新推理，**221/221 × 5 = 1105/1105 通过**。其中 1090 项为识别输出严格一致，15 项为原有三类预期拒绝在五环境的错误契约通过；这些拒绝没有被伪造为识别成功。两套原生与三个 Wasm 环境均保存实际输入、参数、输出、运行资产摘要与比较结果。

| 实测环境 | 构建 | 本轮全新运行 | 识别 / 预期拒绝 |
|---|---|---|---|
| Windows x64 原生，MSVC 19.44 / 源码 ORT 1.23.2 | 通过 | 221/221 | 218 / 3 |
| Linux x64 原生，Ubuntu 26.04 / GCC 15.2 / 源码 ORT 1.23.2 | 通过 | 221/221 | 218 / 3 |
| Windows Node 24.14.0，Wasm SIMD128 | 通过 | 221/221 | 218 / 3 |
| Chrome 156.0.8078.79，Wasm SIMD128 | 同一 Wasm 构建 | 221/221 | 218 / 3 |
| Linux Node 24.14.0，Wasm SIMD128 | 同一 Wasm 构建 | 221/221 | 218 / 3 |

范围为 **126 张样本 + 27 项固定语料 + 书页 + 67 个参数组合 = 221 项/环境**。本轮为所有 221 项重新采集了固定 Photos DLL 的原始基准。有限集合内的通过不等于所有图像和参数都已完成；`engine.parity=incomplete`、`goal_complete=false` 继续保留。

## 新增覆盖与修复

新增 12 张图片：两张真实纸面中文手写照片、各自 90/180/270/10 度变换共八张、十二行打印控制图与空白控制图。摄影者为 jason hu，来源、许可、实际 CDN 图像摘要和变换记录见 [provenance](../../samples/options-handwriting/provenance.json)。摄影者未说明书写者，不能宣称多书写者覆盖；原 DLL 的类型输出照原样比较，实际手写照片并不保证被它判为 handwritten。

67 组是 **66 个非默认组合与一个完整默认对照**：打印图 11 种行预算 × 5 种尺寸共 55 项、两张真实手写各三组共六项、零别名两项、空白边界四项。逐项请求参数、输入摘要见 [parameter-cases.json](../../research/parameter-cases.json)；有效参数与原 wrapper 实参见 `reference/parameters/*.contract.json`。同图不同参数的身份纳入运行和复核，不能互相替代。

使用固定 reverse-skill `cab634bd855fc287f6e420c1f36fd1a6b9245960` 的 R22 / Ghidra 离线样本路由，恢复四处处理：整分量的软行预算批次、polygon IoU 前的整数包围框粗筛、flip 校正后的 CJK layout 输入、输出词置信度按 UTF-8 字节数加权的方向候选分数。[正式逆向报告](../../docs/2026-11-03_reverse-oneocr-options-handwriting-report.md)、[E/F/P 记录](../../research/options-handwriting/report/findings.md)与六份版本绑定的原始 oracle 随项目保存。

95 次 classifier / recognizer / layout 调用的输入与输出逐字节一致，见 [tensor-equivalence.json](evidence/tensor-equivalence.json) 和 [字节摘要](evidence/tensor-bindings.json)。决定 flip/layout 顺序的五份小数组另随报告保存，可用 `python scripts/verify-layout-witness.py` 独立核验。整批大张量的重新采集命令见 [证据说明](evidence/flip-layout-witness/README.md)。

旧的 API 集成测试曾将 max-lines 当作硬上限，现改用十二行图与原始 DLL 冻结结果严格比较软预算，Windows/Linux 均通过。单元测试、API、源码绑定、构建和性能原始记录在 `evidence/`；完整 Python 负例套件状态见 [python-full.json](evidence/python-full.json)。

Windows/Linux CTest 各 10/10；Wasm 四个 C++ 测试程序（舍入与像素边界、百万次 FMA、退化曲线与极长裁剪、48 个非有限词框与八个有限超差负例）均通过。Windows/Linux 的真实 Wasm API、取消和超时测试通过；Python 完整套件 62/62 通过，包含参数漂移与源码 ZIP 索引负例。

## 固定验收与身份

- 文字、阅读顺序、词切分、手写类型、计数和索引完全一致；坐标最大误差 1px，角度 0.1°，置信度 1e-3。NaN/Inf、非法类型与范围均拒绝。比较器没有修改。
- 原模型 SHA-256：`f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`。
- Photos 11.0.15 DLL SHA-256：`0a5e99041af902d6825f0af04ab25e62d2e2378ace69d10175f0dcc22816aae0`。
- 比较器 SHA-256：`06d137cc93680f766031d783da9526064808fea1135124348e180bb8607f4869`。
- 运行实现提交：`dfa477b8320d4e485aa9b23ffdb48735dbef8ac3`。其后的 API 核验脚本、文档与报告提交不改变这些执行的引擎二进制；原生 bin/lib 和 Wasm 实际资产摘要由独立复核检查。

原始失败与中间修复保留在 [diagnostics](diagnostics/README.md)，没有用于代替最终执行。`evidence/model-preparation.json` 是现有 155 个模型资源的摘要核验，未冒称再次提取模型。每项实际耗时和内存见 [performance.json](evidence/performance.json)；多进程互有 CPU 竞争，耗时总和不等于墙钟时间，浏览器进程峰值也不冒充同时总峰值。

## 封存、断网与复现

`FILES.sha256.json` 封存本目录所有产物。独立只读复核写入单独的 [review 报告](../options-handwriting-review-2026-11-03/README.md)，检查 1105 项保存输出、当前输入/参数/基准身份、模型资源、运行资产和源实现提交，明确标为 saved-results-verification，不冒充另一次新推理。原执行记录保持不变。

Linux 原生和 Wasm 都在隔离目录、`unshare --net` 中执行书页识别并严格匹配 106 行。原生隔离目录无微软 OneOCR DLL/onemodel，实际映射模块检查为无；Wasm 隔离目录无原生 DLL/SO/EXE，仅 Wasm、JS 与独立模型。见 [原生断网](evidence/native-offline/book.verification.json)、[原生严格比较](evidence/native-offline/book.parity.json)与 [Wasm 断网](evidence/wasm-offline/summary.json)。

```powershell
.research-python/python.exe -X utf8 scripts/run-native-parity.py --binary artifacts/win32-x64/bin/ocr.exe --output work/fresh-options-native
.research-python/python.exe -X utf8 scripts/run-wasm-parity.py --jobs 3 --output work/fresh-options-node
.research-python/python.exe -X utf8 scripts/run-python-tests.py --report work/fresh-python-tests.json
.research-python/python.exe -X utf8 scripts/verify-recorded-parity.py --report reports/options-handwriting-2026-11-03 --output work/fresh-options-review
```

源码、样本、资产、报告全部本地 commit；不 push、发布或上传。最终干净 HEAD 之后生成 `artifacts/win11-oneocr-offline.bundle`、`artifacts/oneocr-source.zip` 与摘要。包精确 HEAD、大小、SHA-256 在 `artifacts/OFFLINE_PACKAGE.json`；这些为可再生忽略产物，避免包包含自身提交的循环。bundle 包含已提交的模型与运行文件，源码 ZIP 保留样本与索引覆盖的全部研究/报告证据，排除模型权重和编译文件。模型权利声明不变。

## 未完成与未验证

Photos resize setter 共同有效上限为 1152×768；超限的原 wrapper 返回 -20。原生 API 的 32..8192 是扩展，共同范围之外不声明 DLL 等价。零别名使用有效默认值 1000 / 1152×768 作对照，不冒称与 raw wrapper 跳过 setter 的语义相同。更多参数、歧义分支、书写者与其他手写文字类别仍需样本与实测。

Linux ARM64、macOS x64/arm64、Android arm64-v8a 的原生构建和运行，以及 Safari、Firefox、移动浏览器、Node 20 全范围仍未验证。Emsdk 所带 Node 20 运行的构建期数学测试不能算该版本全量通过。
