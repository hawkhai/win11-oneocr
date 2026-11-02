# OneOCR 全样本回归

`manifest.json` 穷举本目录下全部 110 张图片并固定相对路径、尺寸、像素模式和 SHA-256：经典样本 85 张（30 张输入、16 张参考图、39 张缩略图/拼图），真实手写组 7 张，微软公开能力场景补充组 18 张。参考图上原有识别框和答案也作为图片像素输入，不把它们当文字真值。

原 DLL 输出是兼容性基准，不是识别准确率真值。文字、行数、阅读顺序、词切分、手写类别要求完全一致；坐标容差 1 px，角度 0.1 度，置信度 `1e-3`，由未改变的 `scripts/compare-results.py` 检查。

三个原接口错误用例单独计数，不冒充识别成功：

- `ocr-classics/images/cn-single-line-mixed.jpg`：256×32，DLL 拒绝尺寸，wrapper -18。
- `ocr-classics/thumbnails/cn-single-line-mixed.jpg`：256×32，同上。
- `ocr-classics/references/quality-border-comparison.png`：灰度加透明通道，旧 wrapper 拒绝，-15；不是 OCR 模型拒绝。

准备原计划中的独立 `models` 和源码构建的 CLI 后，从仓库根目录重跑：

```powershell
.research-python/python.exe -X utf8 scripts/run-all-samples.py --inventory samples/manifest.json --baseline reports/oneocr-all-samples-2026-11-02/legacy --binary build/source-runtime/native/Release/ocr.exe --output work/oneocr-portable/replay-all-windows
```

Linux 对应 `python3` 与 `--binary build/linux-x64/native/ocr`，不需要原微软 DLL。验证程序拒绝新增、遗漏或哈希不符的图片，并验证冻结基准的结果哈希；每例记录耗时与峰值内存。`--freeze-dll` 仅限 Windows，在独立进程生成缺失基准，绑定 DLL、wrapper、运行库和模型的哈希，不覆盖已有基准。`--refresh-inventory` 明确更新全目录清单（需要 Pillow），不能用于排除失败图片。

补充样本的官方依据、来源、生成操作与许可见 [oneocr-capabilities/README.md](oneocr-capabilities/README.md)。完整验证报告见 [全样本报告](../reports/oneocr-all-samples-2026-11-02/README.md)。所有原图保持原始尺寸，不为通过回归而补边、裁剪或换模型。
