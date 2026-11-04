# Wasm 网页能力对齐验证

2026-11-04，Asia/Shanghai。用户根据交接另行要求确认 Wasm 网页能力。本次仅补齐网页参数入口、构建选择和验证证据；Windows/Linux C++ 已完成的收手结论保持不变。

Chrome 156.0.8078.80 当前 **269/269 全量新推理通过**，其中 266 项识别、3 项预期拒绝，合计 3544 行、19551 词；新增 48 组参数全部覆盖。保存结果另行独立复核 **269/269**，明确 `fresh_inference=false`。以下功能检查均使用真实 Wasm 和当前模型。

## 能力与实现

| 能力 | 接口及验证 |
|---|---|
| 原始图像输入 | 网页直接将原文件字节交给 Worker，不经 Canvas 转码；浏览器回归逐图核对 SHA-256 |
| 行预算、缩放 | 新增网页 `maxLines`、`resizeWidth`、`resizeHeight` 输入；默认零别名、整数及范围校验与引擎约定一致 |
| 软行预算 | 十二行图在默认缩放、`maxLines=1` 下返回 4 行，未在网页裁成 1 行 |
| 完整结果 | 保留文字、阅读顺序、行词四边形、词置信度、图像角度、手写/印刷类型、计数及索引 |
| 取消及恢复 | 初始化期间取消、已初始化实例识别期间取消、取消后重建、无效图像错误后恢复均通过 |
| BGRA | 浏览器和 Node 均实际调用带长度接口；测试行尾填充、步长、短缓冲区和顺序排队 |
| 生命周期与加载错误 | 重复关闭、关闭后拒绝调用、超时、清单校验失败及 HTTP 404 均通过 |
| Node 与插件 | 真实 API、资源摘要、输入所有权、相对运行模块/wasm URL、文件 URL 字符串 Worker、插件调用均通过 |

网页非默认缩放宽高须同时在 32..8192，两个零表示默认 1152×768；行预算零表示默认 1000。原 DLL 的共同缩放范围仍为 1152×768，扩展范围不冒称 DLL 等价。网页始终保留完整 JSON。

## 构建和资产身份

实际执行了 `scripts/build-wasm.py`，使用锁定的 ORT 1.23.2、OpenCV 4.10.0、ICU 74.2、Emscripten 4.0.8；复用已配置的依赖构建目录，执行增量 configure/build、ORT 归档重新打包、边界测试及安装。不是清空依赖后的全新编译：引擎与 ORT 已是最新，FMA 测试重新编译。[构建日志](evidence/build.log)记录完整命令与输出。

构建阶段最近偶数舍入、1000000 随机 FMA 通道及边界矩阵、曲线/裁剪 DLL 预言机、非有限及超差词框负例全部通过。

新安装输出位于忽略的 `work/wasm-web-alignment-2026-11-04/runtime/`，新二进制没有加入 Git。其 8 个运行文件（核心、JS/Worker、CLI 和声明）与已提交 `artifacts/wasm32/` **逐字节一致**，全部 252 个登记模型/运行资产摘要保持原样，见[身份核验](evidence/runtime-identity.json)。[本次真实构建指纹](evidence/rebuilt-BUILD.json)与历史交付 BUILD 分开保存，没有手改旧指纹冒充新构建。

模型仍为 Photos 2026.11090.22001.0 / OneOCR 11.0.15，来源 SHA-256 `f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`。比较器仍为 `06d137cc93680f766031d783da9526064808fea1135124348e180bb8607f4869`，未换模型、改图、放宽阈值或覆盖历史报告。

## 验证记录

- [Chrome 当前全部 269 项新推理](chrome/summary.json)、[完整执行封存](chrome/execution.json)、[范围及新增 48 组核对](evidence/coverage.json)、[保存结果独立复核](evidence/browser-verification.json)。独立复核直接选择已提交的原有运行资产；其 8 个运行文件与本次构建相同，不依赖忽略目录的新二进制。
- [网页和浏览器 Worker 功能检查](ui/summary.json)，Chrome 156.0.8078.80；[实际页面截图](ui/page.png)。
- [Node API/插件检查](evidence/api.log)，Node 24.14.0；[Node 单项新推理](node-subset/summary.json)明确 `full_regression=false`，不冒充 Node 当前全量。
- [Python 当前完整套件](evidence/python-current-tests.json)：71/71；[日志](evidence/python-current-tests.log)。另有[构建目录选择负例](evidence/selected-delivery-tests.log)：篡改所选目录的 Worker 必须失败，不能退回默认运行文件继续通过。
- [页面及测试源码指纹](evidence/source-files.json)按 CRLF 转 LF 记录，完整报告文件由 `FILES.sha256.json` 绑定。页面默认、软预算和缩放输出分别通过[默认比较](ui/default-parity.json)、[软预算比较](ui/soft-budget-parity.json)、[缩放比较](ui/resized-parity.json)。

完整浏览器范围为当前 126 样本＋27 固定语料＋1 书页＋115 参数＝269 项，其中 266 项识别、3 项预期拒绝。新增 48 组参数也须逐项登记在输出中。沿用原比较器的坐标 ≤1px、角度 ≤0.1°、置信度 ≤1e-3；其他既有字段一致、拒绝 NaN/Inf。拒绝输出单独核对错误契约，不计作识别成功。运行时 `engine.parity=incomplete` 保持原样。

## 复现

依赖自行取得并按固定版本准备，Linux/WSL 构建可选择独立目录：

```sh
python3 scripts/build-wasm.py --ort-source <pinned-ort-source> --opencv-source <pinned-opencv-source> --icu-source <pinned-icu-source> --build-dir <build-directory> --output work/wasm-local/runtime --jobs 6
```

已有相同固定 ORT 配置时才加 `--reuse-ort-config`。Windows 验证使用 Python 3.11+（本机研究 Python 为 3.12.8），Node 24.14.0，以及已准备的 `playwright-core@1.55.0`：

```powershell
.research-python/python.exe -X utf8 scripts/run-wasm-parity.py --wasm-dir work/wasm-local/runtime --browser 'C:/Program Files/Google/Chrome/Application/chrome.exe' --output work/wasm-browser-new --jobs 4
node test/wasm-demo-test.mjs 'C:/Program Files/Google/Chrome/Application/chrome.exe' work/wasm-ui-new work/wasm-browser/node_modules/playwright-core/index.mjs work/wasm-local/runtime
$env:ONEOCR_WASM_DIR = 'work/wasm-local/runtime'
node test/wasm-test.mjs
.research-python/python.exe -X utf8 scripts/run-python-tests.py --report work/wasm-python-new.json
```

`--wasm-dir` 核验所选目录的 BUILD、每个输出文件和源码指纹；默认仍为 `artifacts/wasm32`，不会跳过过期指纹检查。浏览器测试仅监听 127.0.0.1，并阻止其他 origin 请求，图像及模型不上传。

独立复核本报告，不执行推理：

```powershell
.research-python/python.exe -X utf8 scripts/run-wasm-parity.py --compare-only --output reports/wasm-web-alignment-2026-11-04/chrome --verification-output work/wasm-saved-review-new
```

该命令检查当前 269 项输入/参数、原 DLL 基准、封存文件及当前原有运行资产身份。它没有跳过新推理入口的源码过期检查；新推理仍须先重建到独立目录。

此次不把历史 Windows/Linux Node 的 221 项扩写成当前 269 项，也不标记 Safari、Firefox、移动浏览器或其他原生平台已验证。Wasm 比较属于诊断和质量记录，仍不作为 C++ 收手门槛。只作本地提交与离线包刷新，没有 push、公开发布或上传模型。
