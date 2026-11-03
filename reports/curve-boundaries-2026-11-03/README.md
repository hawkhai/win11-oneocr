# 曲线与极长行补齐：全新严格回归

本轮五个环境全部重新推理，各 **142/142**，共 **710/710**。每端 139 项识别输出、3 项预期拒绝，合计 **695 项识别、15 项拒绝**；拒绝不计为识别成功。模型、原有图片与比较器均未修改，数值阈值保持坐标 1 px、角度 0.1°、置信度 0.001，文字、阅读顺序、词切分和手写类型要求一致。

实现及运行资产提交为 `076e85a030eab931c04d323f7f21f5bc9340dee2`；原生运行文件按该提交核验，Wasm 按执行时保存的 BUILD 和逐文件摘要核验。后续提交补充文档、打包脚本、当前状态、样本说明与执行报告，未变更已验收的推理代码或运行二进制。原始 DLL 基准也在本轮重新捕获，见 [reference](reference/README.md)。

## 实测范围与结果

114 张 samples 图片（原 110 张全部保留，加 4 张曲线/长行场景）+ 27 项固定语料 + 106 行书页，每端 142 项。完整结果在各环境目录，汇总在 [summary.json](summary.json)。

| 环境 | 严格通过 | 最大坐标差 px | 最大角度差 ° | 最大置信度差 | 逐例耗时之和 s | 最大进程峰值 MiB |
| --- | --- | --- | --- | --- | --- | --- |
| windows-native | 142/142 | 0.0009765625 | 0 | 1.7285347e-06 | 164.42 | 377.85 |
| linux-native | 142/142 | 0.0009765625 | 0 | 1.7285347e-06 | 260.90 | 396.03 |
| windows-node | 142/142 | 0.0009765625 | 1.1920929e-07 | 0.000933587551 | 7216.84 | 663.36 |
| chrome | 142/142 | 0.0009765625 | 1.1920929e-07 | 0.000933587551 | 6443.57 | 910.89 |
| linux-node | 142/142 | 0.0009765625 | 1.1920929e-07 | 0.000933587551 | 7718.41 | 873.13 |

Windows/Linux Node 均为 24.14.0；浏览器为 Windows Chrome 156。三个 Wasm 环境各使用 3 个独立实例并行推理，逐例耗时之和含 CPU 争用，不是整轮墙钟时间或纯计算 CPU 时间。原生逐次启动 CLI，Node/浏览器复用各自实例；性能数字不能直接等同于吞吐比较。内存来自各环境原始测量；Node 为 worker 最大 RSS，Chrome 为关闭前仍存活浏览器进程的 OS 峰值工作集。各进程峰值之和只记作上界，不表示同时占用峰值，细节见各执行 summary 的 workers。

三项拒绝是两张高度不足 50 的单行图（`-18`）和一张 LA 通道图（`-15`），与原 DLL 拒绝行为一致。没有删除、缩放或替换这些输入。源码生成场景用于同图比较，不以人工预设文字作为本轮准确率真值。

## 修复与直接分支证据

- 退化曲线：保留原 LU 运算顺序，仅在零主元时使用 SVD 正规方程备用解。8 组系数含 6 个备用分支和两个 LU 控制项，不用 epsilon 替换原决策。
- 超长多边形：截断位置控制图宽，源点全部保留，目标尾部为零，仍按原顺序覆盖全部三角形；同步还原裁剪后的词框逆映射和翻转。
- 7 组几何、56 个词框与原始归一化像素由固定 DLL 内部调用产生；包含曲线与偏移顶点、跨上限、恰好到达上限、首段跨越、未截断控制以及两种方向。内部系数、像素和检测张量逐元素相等，词框断言 0.0001 px。新旧 DLL 内部预言机结果相同。
- 10000×140 的长行 PNG 初次出现 627 项差异，保存于 [初次失败执行](diagnostics/windows-initial/)。原 DLL 检测输入与初始源码有 102249 个 float 不同，最大差 197.314437866；静态映射证据定位到面积比例 ≤0.04 时的 AREA 插值。修复后 110592 个 float 完全一致，整图严格比较零失败，见 [张量差异](diagnostics/detector-input/comparison.json)。0.04 是还原的算法常量，不是放宽验收阈值。

PNG 外观不用于证明内部秩亏或最大归一化宽度分支；内部几何预言机将最大宽度设为 297，单独覆盖这些路径，没有改变公开 resize 合法范围或默认 1152×768 / 1000 行参数。完整算法、RVA、工具版本与 Evidence→Finding→Path 见[技术报告](../../docs/2026-11-03_reverse-oneocr-curve-boundaries-report.md)和[案件证据](../../research/curve-boundaries/README.md)。

## 测试、复核与交付

Windows/Linux CTest 各 **9/9**；Wasm 构建的舍入、FMA 和 DLL 分支预言机测试全部通过，要求实际出现 C++ 成功标记。Python 共 **47/47**；其中保存结果入口的 8 个测试覆盖当前输入被修改、增删范围、冲突/未登记输出及原目录只读行为，另有执行状态单元测试核验重复元数据绑定。Wasm API、Worker、取消、超时、所有权、BGRA 步长及插件测试通过。见 [测试清单](evidence/tests.json)与对应日志。

新推理结束后，五端保存结果又独立严格复核 **710/710**；三端单独 compare-only 也各 142/142，输出在 [verification](verification/)。它们标记 `fresh_inference=false`，不重复计为新推理。复核绑定的原始执行索引保存在 `evidence/execution-index.sha256.json`；最终 `FILES.sha256.json` 再封存全部交付报告，避免索引自引用。模型 155 个资源、交付 252 个文件、原 110 张图片及四轮旧报告索引均已核验，见 [不变项证据](evidence/unchanged-bindings.json)。旧报告没有改写或冒充新执行。

Linux 原生与 Wasm 分别在独立临时目录、`unshare --net` 中识别书页通过。原生目录只含源码运行库、模型和书页，进程无微软 OCR DLL；Wasm 目录连原生 DLL/SO/EXE 都没有，见 [原生隔离](offline-native-isolated/summary.json)及 [Wasm 隔离](offline-wasm/summary.json)。实现提交另从独立 Git 克隆校验 252 个资产并识别 106 行书页，见 [克隆证据](evidence/clean-checkout.json)；不是在该克隆重复整轮 142 项。

源码、模型、运行文件、输入、预言机和报告均纳入本地 Git；本轮无 push、上传或发布。完整 Git 包和纯源码 ZIP 由 `scripts/package-offline.py` 在提交完成后重建，包本身作为可再生交付物不嵌套进 Git。最终 HEAD、长度及摘要在本机 `artifacts/OFFLINE_PACKAGE.json` 和两个 `.sha256` 文件。源码 ZIP 含当前报告、输入和预言机，排除模型与编译文件，独立模型由 Git 包提供。

## 平台与剩余边界

| 目标 | 构建 | 运行 |
| --- | --- | --- |
| Windows x64 原生 | 通过 | 142/142 |
| Linux x64（Ubuntu 26.04 / WSL）原生 | 通过 | 断网 142/142 |
| Wasm32 | 通过 | Windows Node / Linux Node / Chrome 各 142/142 |
| Linux ARM64 | 未验证 | 未验证 |
| macOS x64 / arm64 | 未验证 | 未验证 |
| Android arm64-v8a | 未验证 | 未验证 |

Safari、Firefox、移动浏览器及 Node 20 全量集未验证。非默认参数、连接/方向歧义组合、独立真实中文及更多文字的手写场景仍需扩展覆盖；研究用 Python 参考实现未被作为完整替代品或本轮全量通过端。`engine.parity=incomplete` 和整体目标未完成状态保留，模型权利声明不变。

## 复跑

在项目根目录使用 Python 3（仅为测试驱动）和对应平台运行文件；每次使用全新的输出目录：

```powershell
python -X utf8 scripts/run-native-parity.py --binary artifacts/win32-x64/bin/ocr.exe --output work/replay-native
python -X utf8 scripts/run-wasm-parity.py --jobs 3 --output work/replay-node
python -X utf8 scripts/run-wasm-parity.py --jobs 3 --browser "C:/Program Files/Google/Chrome/Application/chrome.exe" --playwright-path work/wasm-browser/node_modules/playwright-core/index.mjs --output work/replay-chrome
python -X utf8 scripts/verify-recorded-parity.py --report reports/curve-boundaries-2026-11-03 --output work/recheck-curve-records
```

浏览器复跑需要已准备 Playwright Core 1.55.0；Linux 使用 python3、Linux Node 24.14.0 与原生 Linux 路径，断网回归由 `unshare --net -- python3 scripts/run-native-parity.py ...` 或 `run-wasm-parity.py --node /absolute/path/to/linux/node ...` 启动。正式识别使用 `ocr.cmd` / `ocr.sh` 或 Wasm CLI，不需要 Python。
