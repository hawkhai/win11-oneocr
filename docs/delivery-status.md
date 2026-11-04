# 原生交付状态与复核入口

2026-11-04。本轮范围和阈值仍以 AGENTS.md、acceptance-standard.md 及 acceptance-policy.json 为准：Windows x64 / Linux x64 各 269 项，共 538 项；532 项识别输出、6 项预期拒绝。坐标 ≤1px、角度 ≤0.1°、置信度 ≤1e-3，其余既有字段严格一致。已完成并收手；本次增强交付与验证工具，不扩大登记范围或平台。

| 身份 | 权威记录与含义 |
|---|---|
| 当前交付源码 | `git rev-parse HEAD`；`artifacts/OFFLINE_PACKAGE.json.commit` 必须相同 |
| 最新完整新推理 | `reports/colleague-build-2026-11-04/`，两端各 269/269；其 source-bindings 和 rebuilt_outputs 固定实际源码与新编程序身份 |
| 包内原有运行程序 | `artifacts/{win32-x64,linux-x64}/BUILD.json` 及 `research/delivery-manifest.json`；没有被最新重编程序覆盖；当前登记范围证据位于 parameter-expansion 报告 |
| 原 DLL / 模型 / 参数 | `research/delivery-manifest.json.reference`、模型清单及活动 115 组参数清单；不换版本 |
| 保存结果复核 | `reports/cross-improvement-2026-11-04/`；538 项复核不是新推理 |
| 本次补充运行检查 | 上述报告的两端 isolation 文件；既有书页的成功→错误→成功恢复及模块加载检查，不等于又执行一次 269 项全量 |
| 提交后包身份 | 打包器生成 `artifacts/DELIVERY_STATUS.json`，由 OFFLINE_PACKAGE.json 绑定摘要；保留原 `parity=incomplete` |

## 已保存结果与补充契约检查

以下命令在项目根执行，输出路径必须全新；它们不增加图片、参数或平台。

```powershell
python -X utf8 reports/colleague-build-2026-11-04/evidence/review-build-results.py --report reports/colleague-build-2026-11-04 --output work/new-saved-review
python -X utf8 scripts/check-native-isolation.py --runtime artifacts/win32-x64 --output work/new-windows-isolation.json
```

```sh
python3 scripts/check-native-isolation.py --runtime artifacts/linux-x64 --output work/new-linux-isolation.json
```

隔离工具使用已登记书页及原 DLL 基准，复制原生引擎、ORT 和配套运行库到临时目录，清理动态库搜索环境，从无关工作目录调用 C ABI。它核对实际加载模块路径和摘要，以及同一实例错误前后的完整成功结果、错误清空和原有严格比较。Python 只用于检查，不成为 C++ 运行依赖。原兼容 wrapper 不参与此项检查。

## 提交后打包

```powershell
python -X utf8 scripts/package-offline.py
python -X utf8 scripts/package-source.py --verify artifacts/oneocr-source.zip
```

打包要求干净 HEAD，先独立复核最新保存的 538 项结果，再生成 bundle/源码 ZIP，结束时重新检查 HEAD 和工作树；生成状态表分别记录包内原有程序与最新重编程序。最终检查 OFFLINE_PACKAGE.json 的 commit、两个包的 SHA-256 和 DELIVERY_STATUS.json 摘要。生成物仍在忽略目录，不提交自身、不 push 或上传模型。

本次没有改变引擎、模型、运行资产、比较器或登记范围。相关身份或行为以后发生变化时，仍须按原规则重新执行受影响两端的完整当前范围回归。
