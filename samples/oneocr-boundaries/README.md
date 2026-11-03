# 曲线与极长行样本

四张图片由 `scripts/make-boundary-samples.py` 在本机生成，原图、尺寸、生成用途和 Arial 字体摘要保存在 `manifest.json`；字体文件不随项目分发。图片不预设正确识别文字，以相同原始字节分别提交给冻结 DLL 和源码引擎。

- `curved-line.png`：沿抛物线排布的印刷拉丁文字。
- `near-collinear-line.png`：带一像素基线扰动的小字体。
- `very-long-line.png`：宽度达到公开接口上限 10000 像素的单行；同时覆盖检测缩放面积比例低于 0.04 的 AREA 分支。
- `normalizer-grid.png`：供内部几何预言机使用的确定性 RGB 图案，也参加全图识别回归。

图片的外观不能证明内部求解器或宽度截断分支被执行。精确秩亏及裁剪前、等于和跨越上限的几何输入另在 `research/curve-boundaries/contract.json`，由独立 DLL 子进程产生系数、像素和词框基准，Windows、Linux、Wasm 使用相同测试。内部测试将最大归一化宽度设为 297，仅用于直接调用内部函数；不改变公开接口的合法缩放范围或默认参数。
