# 真实手写与旋转回归

原图为微软 Azure-Samples 仓库固定提交中的真实英文手写图片。`manifest.json` 记录原始来源、SHA-256 以及六张旋转派生图的操作。`LICENSE.azure.txt` 保留来源仓库的 MIT 声明；不将 OCR 模型权重标记为 MIT。

离线重建旋转图：运行 `scripts/make-handwriting-corpus.py`。这七张图用于检验原 DLL 与源码版的文字、几何、词切分、方向与手写判断是否一致，不将相同错误视作识别准确率证明。
