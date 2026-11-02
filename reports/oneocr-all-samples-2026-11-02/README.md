# OneOCR 全目录严格回归：110/110

Windows x64 与 WSL Linux x64 均构建通过、运行通过，`samples` 下全部 110 张图片通过兼容性检查：107 张严格识别结果一致，3 张原接口错误契约一致。没有换模型、修改样本尺寸或放宽比较阈值。完整跨平台目标仍未完成，Android、macOS 和 ARM64 未标记为已验证。

样本提交：`13de74a`。本轮新增 18 张能力场景图，并提交此前未提交的 7 张真实手写与旋转图。全部图片的固定清单在 [samples/manifest.json](../../samples/manifest.json)，官方能力依据与来源在 [补充样本说明](../../samples/oneocr-capabilities/README.md)。既有经典样本、缩略图、总览拼图和参考图全部参加识别，没有只选清单中的 30 张输入。

| 检查 | Windows x64 | Linux x64 / WSL |
| --- | --- | --- |
| 全目录样本 | 110/110 | 110/110（断网命名空间） |
| 其中严格识别 | 107/107 | 107/107 |
| 其中原接口错误契约 | 3/3 | 3/3 |
| 原固定合成回归集 | 27/27 | 27/27 |
| 原书页全部字段 | 105/105 行 | 105/105 行 |
| CTest | 8/8 | 8/8 |
| 单图耗时中位数（含进程启动/模型加载） | 0.856 秒 | 1.420 秒 |
| 全样本中最高单进程峰值工作集 | 376.25 MiB | 391.33 MiB |

两个平台每端合计 138 项用例：135 项识别结果比较，3 项错误契约。耗时受并发测试、Windows/WSL 文件系统和机器负载影响，不作为性能基准。逐例原始测量见 [Windows suite.json](windows/suite.json) 和 [Linux suite.json](linux/suite.json)。Linux 安装目录的 [断网与模块验证](linux/offline-verification.json) 确认联网失败，实际识别进程未加载原微软 OCR DLL 或 Wine。

## 严格标准与错误用例

沿用 `scripts/compare-results.py`：文字、行数、顺序、词切分、手写类别完全一致；每个行/词四边形坐标误差不超过 1 px，图像角度不超过 0.1 度，置信度不超过 `1e-3`。`file` 与额外的引擎诊断字段不参与比较。原 DLL 的识别错误也照实保留，这不是识别准确率评测。

三张错误图没有被计为识别成功：单行中英原图与它的缩略图均为 256×32，被 DLL 的 50..10000 尺寸契约拒绝（wrapper -18）；`quality-border-comparison.png` 是灰度加透明通道，被旧文件包装层拒绝（-15）。源码文件接口恢复相同拒绝行为；原图没有补边或改通道。该通道限制来自包装层，不证明模型不支持图片中的内容。

## 本轮修复与证据

初次穷举 92 张既有图片发现总览拼图不一致；加入 18 张新图后为 109/110。随后恢复了下列默认流程细节：

1. 检测抑制使用完整曲线多边形的面积与相交面积，不能仅比较四个角。
2. DLL 在两次组哈希遍历之后、全局分数排序之前，排除二次系数绝对值超过 0.05 的过弯曲线。过滤位置影响相同分数的排序与后续框合并。该数值来自 DLL 常量与静态/动态证据，是被恢复的算法规则，不是验收容差。
3. 四点多边形的中心线只有两个点，应执行两点拟合；用三阶法方程偶然求得的数值解会改变覆盖判断。
4. 原生三角形裁剪、坐标行列式及面积累加都保留原 DLL 的 float 计算顺序。通用 OpenCV 相交面积在总览图的大坐标处产生最高约 0.192 的中间面积差；复现原算法后，2,112 项原生多边形探针的最大面积差为 0，72 项识别后抑制合约通过。
5. 动态观测显式设置最大识别行数 1000，避免 DLL 原生默认值截断总览图的观测；正式 CLI 的默认值一直保持 1000。

总览拼图最终 281/281 行全部字段通过。原生探针输入与输出固定在 [overview-polygon-nms-contracts.json](../../research/overview-polygon-nms-contracts.json)，逐图最终差分文件位于两个平台子目录，冻结的原始结果与错误记录位于 `legacy/`。每个基准绑定图片、原 DLL、wrapper、运行库和模型的 SHA-256；`summary.json` 还记录最终原生运行库、模型清单和测试结果摘要。

## 离线复跑

准备同源独立模型和按项目说明构建的 CLI 后，在仓库根目录运行：

```powershell
.research-python/python.exe -X utf8 scripts/run-all-samples.py --inventory samples/manifest.json --baseline reports/oneocr-all-samples-2026-11-02/legacy --binary build/source-runtime/native/Release/ocr.exe --output work/oneocr-portable/replay-all-windows
```

```sh
sudo unshare -n python3 scripts/run-all-samples.py --inventory samples/manifest.json --baseline reports/oneocr-all-samples-2026-11-02/legacy --binary build/linux-x64/native/ocr --output work/oneocr-portable/replay-all-linux
```

复跑不调用微软 DLL；只在首次生成新的 Windows 基准时使用 `--freeze-dll`，并在独立进程执行。程序穷举图片并检查清单，遗漏、增减或哈希变化都会报错。保留每例耗时、峰值内存和所有失败字段，失败退出非零。

## 仍未完成

有限集通过不能证明任意图片、全部参数或目标平台等价。退化曲线的备用求解、极长行裁剪、更多方向/歧义配置仍需继续恢复；真实手写目前主要来自一个英语来源，其他文字的真实手写仍有覆盖缺口。`engine.parity` 继续为 `incomplete`。模型权利声明不变；正式源码引擎运行无需账号、证书或授权服务。
