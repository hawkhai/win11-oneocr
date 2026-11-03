# 翻转后 layout 的紧凑原始证据

五份 float32 小端原始数组来自本轮真实手写 grid-10 图片的 Photos classifier 调用 24、layout 调用 61 与 C++ line-33。`manifest.json` 记录形状、精确字节长度和 SHA-256。Photos layout 输入是 classifier 输入逐通道旋转 180° 的结果；C++ layout 输入与输出分别逐字节等于 Photos 数组。

执行 `python scripts/verify-layout-witness.py` 可在任意平台用标准库复核这些已保存数组；它不执行推理，不替代五环境全量回归。完整 95 次调用的匹配结果在上级 `tensor-equivalence.json`，每个匹配输入/输出的原始字节摘要在 `tensor-bindings.json`。这里保留决定调用顺序的小数组，整批大张量可从固定 DLL 和图片重新采集。

Windows 原始张量重新采集（独立子进程，输出到新的模型/trace 目录，保留正式模型）：

```powershell
.research-python/python.exe -X utf8 scripts/research.py extract --bin bin/photos-2026.11090.22001.0 --models work/reproduce-grid10-models --trace samples/options-handwriting/red-grid-jason-rotate-10.png --trace-label reproduce-grid10
$env:ONEOCR_TRACE_DIR = (Join-Path (Get-Location) 'work/reproduce-grid10-cpp')
artifacts/win32-x64/bin/ocr.exe samples/options-handwriting/red-grid-jason-rotate-10.png --model-dir models --quiet --output work/reproduce-grid10-result.json
Remove-Item Env:ONEOCR_TRACE_DIR
.research-python/python.exe -X utf8 scripts/compare-line-tensors.py --native work/oneocr-portable/traces/reproduce-grid10 --cpp work/reproduce-grid10-cpp --output work/reproduce-grid10-tensors.json
```

固定 Photos 11.0.15 / 当前模型摘要见主报告；不同版本不得套用此次对象偏移。DLL 和模型原文件未修改，trace 仅为研究过程。
