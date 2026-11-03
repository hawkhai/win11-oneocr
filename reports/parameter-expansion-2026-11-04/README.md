# 联合参数覆盖扩展与 C++ 原生严格回归

2026-11-04，按 reverse-skill 固定提交 `cab634bd855fc287f6e420c1f36fd1a6b9245960` 的 R22/Ghidra 离线流程继续。用户选择扩大输入与参数覆盖。**Windows/Linux 原生全新推理各 269/269，合计 538/538 通过**：532 项识别输出严格一致，6 项预期拒绝单独核对错误契约。两端 CTest 各 26/26，Python 完整套件 67/67。原生算法、模型、运行二进制与比较器未改；本轮没有重建原生运行库或执行新的 Wasm 推理。

## 本轮范围与原 DLL 基准

保留全部 126 张登记图片、27 项固定语料和书页，将参数从 67 组扩为 **115 组**，每端合计 269 项。新增 48 组均登记在 [参数清单](../../research/parameter-cases-2026-11-04.json)：

| 新增组别 | 数量 | 覆盖 |
|---|---:|---|
| 旋转真实中文手写 | 32 | 两张既有照片的 90°/180°/270°/10° 图，分别结合预算 1/5/6/1000 与 1152×768、640×384、768×512、33×65 |
| 多语种打印控制 | 12 | 阿拉伯文、希伯来文、天城文、泰文，结合预算 1/2/0 与常规、非整倍数 resize |
| 多栏与竖排控制 | 4 | 多栏、竖排中文分别结合预算 1/5 与 640×384、1151×767 |

新增用例中 46 组原 DLL 返回非空文字，2 组返回合法空结果；全部保留。`max_lines=1` 的多栏图返回四行，部分手写图返回两到三行，继续验证软批次预算而非硬截断。使用的照片、旋转派生图和打印语料已在历史来源记录中保存；本轮**没有增加图片文件、来源照片或书写者**。多语种控制不冒称真实手写或人工准确率真值。[coverage.json](evidence/coverage.json) 保存逐项输出行数；照片来源见[原 provenance](../../samples/options-handwriting/provenance.json)。

在独立进程对 **全部 269 项重新采集原 DLL 基准**，不是复制旧输出。基准仍为 Photos `2026.11090.22001.0` / OneOCR `11.0.15`，四资产绑定在 [bindings.json](evidence/bindings.json)。旧范围 221 项与本次重采输出另作严格核对，221/221 通过，见 [reference-recapture.json](evidence/reference-recapture.json)。重采原 DLL、原生新推理和保存结果复核分别记账。

## 验收与历史隔离

继续使用原始 `scripts/compare-results.py`，SHA-256 为 `06d137cc93680f766031d783da9526064808fea1135124348e180bb8607f4869`。文字、切分、顺序、类型、计数、索引及字段语义一致；坐标 ≤1px、角度 ≤0.1°、置信度 ≤1e-3，并拒绝 NaN/Inf 与非法字段。三个原有拒绝输入没有删除、转换或改计识别成功。

当前活动范围指向新的版本化参数清单和本报告的原 DLL 基准。旧 67 组清单、五环境报告及其索引保持原样。保存结果工具新增 `--reference` 显式历史快照，输出 `current_scope=false`，仅能证明对应历史范围；默认入口仍要求当前 269 项。新 Wasm 推理拒绝历史 `--reference`，原生新推理不提供此参数。Python 负例确认旧 221 项报告不能充当当前范围通过，也确认原生超差、输出冲突和 Wasm 非门槛规则仍有效。

Wasm 历史封存数据由 Python 测试读取复核，没有执行新的 Wasm 推理，也没有增加 Wasm 联合验收要求。新的 48 组 Wasm 输出未验证，不得声称其通过。原生 C++ 主线不因此受阻。

## 证据链与构建身份

案件 scope、Evidence→Finding→Path、时间线与经验索引见 [research/parameter-expansion](../../research/parameter-expansion/README.md)，正式报告见 [续接报告](../../docs/2026-11-04_reverse-oneocr-parameter-expansion-report.md)。本轮复用既有 Ghidra 静态锚点和已验证原生算法，没有声称新反编译或新增中间张量逐字节证明。新增用例未发现需要修改生产算法的差异。

`evidence/execution-cases.json` 绑定全部图像、选项、预期拒绝和基准摘要；`evidence/source-bindings.json` 绑定本轮验证工具的规范化源码摘要；`implementation_commit` 表示未改动的原生运行资产/生产源码基准，不冒称该提交已包含本轮新工具。`windows-native/` 与 `linux-native/` 为本轮全新推理，Linux 在 `unshare --net` 内运行。`evidence/unit-tests.json` 保存两端 CTest 日志摘要，`evidence/python-full.json` 和日志记录完整 67 项测试。模型和运行文件继续由 delivery manifest 的 252 项资产身份保护。

## 复现

```powershell
python scripts/capture-options-reference.py --parameter-manifest research/parameter-cases-2026-11-04.json --output work/fresh-expanded-reference --jobs 3
python scripts/run-native-parity.py --binary artifacts/win32-x64/bin/ocr.exe --output work/fresh-expanded-windows
python scripts/run-python-tests.py --report work/fresh-expanded-python.json
python scripts/verify-native-results.py --report reports/parameter-expansion-2026-11-04 --output work/fresh-expanded-review
```

第一条仅生成新的原 DLL 观测目录，不自动切换已绑定的活动基准。Linux 用 `artifacts/linux-x64/bin/ocr`；本机断网命令为 `wsl -d Ubuntu -u root -- bash -lc 'cd /mnt/f/pythonx/myocr/win11-oneocr && unshare --net -- python3 scripts/run-native-parity.py --binary artifacts/linux-x64/bin/ocr --output work/fresh-expanded-linux'`。

历史复核示例：`python scripts/verify-native-results.py --report reports/photos-intersection-p2-2026-11-03 --reference research/references/options-handwriting-2026-11-03.json --output work/historical-native-review`。它不执行新推理，也不验证当前新增范围。

报告在生成全部结果后封存索引，独立保存结果复核另存 [parameter-expansion-review-2026-11-04](../parameter-expansion-review-2026-11-04/README.md)。本地提交后刷新 bundle、源码 ZIP 和忽略的 `artifacts/OFFLINE_PACKAGE.json`；无 push、公开发布或上传。

Linux ARM64、macOS x64/arm64、Android arm64-v8a 仍未验证；更多书写者、自然手写风格、文字类别和联合参数仍有缺口。有限范围通过不等于任意输入等价，整体状态继续为 incomplete。
