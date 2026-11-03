# 参数与真实中文手写的封存结果独立复核

按当前严格比较器重新比较五环境保存输出，**1105/1105 通过**；这是 `saved-results-verification`，不冒充新的推理执行。新推理报告见[原始执行](../options-handwriting-2026-11-03/README.md)。

[verification.json](verification.json)记录原始执行索引摘要、实现提交、比较器及验证脚本摘要，并核对当前输入、参数身份、黄金输出、模型和实际运行资产。独立输出目录避免改写原执行证据。

复现命令：

```powershell
.research-python/python.exe -X utf8 scripts/verify-recorded-parity.py --report reports/options-handwriting-2026-11-03 --output work/fresh-options-independent-review
```

原始执行索引 SHA-256 为 `2589a0b01d1d4d262e794e9eb53ca3cb52da3935a4718acb5d58bc34708c232e`，源码运行实现提交为 `dfa477b8320d4e485aa9b23ffdb48735dbef8ac3`。原生 bin/lib 与 Wasm 实际文件均与执行身份一致，文档更新不改变推理结果。核验工具源码摘要见 [tool-bindings.json](tool-bindings.json)。

整体目标继续 incomplete。Linux ARM64、macOS x64/arm64、Android arm64-v8a 原生构建/运行，Safari、Firefox、移动浏览器及 Node 20 全范围仍未验证。有限参数与两个真实手写来源也不能证明所有选项和书写者等价。最终离线包精确提交和摘要由提交后生成的 `artifacts/OFFLINE_PACKAGE.json` 提供；不上传或发布。
