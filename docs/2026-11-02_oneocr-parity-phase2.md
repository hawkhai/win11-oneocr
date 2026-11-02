# OneOCR 严格对齐：第二阶段

本阶段继续使用固定 OneOCR 9.0.103、同一份原始权重和原有严格比较器。原 DLL、模型及配套运行库未修改。阶段开始按要求完成本地提交 `07e230e`；此后的恢复工作留在工作区，没有 push、发布或模型上传。

## 验证规则

文字、行数、阅读顺序、词切分和手写类别要求一致；匹配行词的四边形最大坐标误差 1 像素，图像角度 0.1 度，置信度 `1e-3`。`scripts/compare-results.py` 的阈值未修改。原始失败记录保留在案件目录，当前统计单独记录在 `research/verification.json` 的 `phase2` 中。

固定测试包括原 27 项、原始书页、30 张经典样例和 7 项真实手写旋转用例，共 65 项。经典样例中的 256×32 图片被原 DLL 拒绝：`RunOcrPipeline` 返回 3、原 wrapper 返回 -18。静态函数 `180029960` 明确限定两个维度各在 50～10000 之间。原生接口补齐该边界，将这张原图作为错误契约测试，不补边、不替换图片，也不计为文字识别通过。

| 验证集 | Windows x64 | WSL Linux x64 |
| --- | --- | --- |
| 原固定集 | 27/27 | 27/27 |
| 原始书页 | 105/105 行，全部字段通过 | 105/105 行，全部字段通过 |
| 经典样例识别 | 29/29 | 29/29 |
| 原 DLL 尺寸错误契约 | 1/1 | 1/1 |
| 真实手写与旋转 | 7/7 | 7/7 |
| 单元合约 | 8/8 | 8/8 |

两个平台各通过 65 项回归，其中 64 项为严格识别结果比较，1 项为错误契约。独立实例并发、重复初始化释放、异常步长、无效图像与损坏模型检查通过。Linux 识别另外在断网命名空间和不含原始微软 OCR DLL 的安装目录中执行。

真实手写来自 [微软 Azure 官方示例](https://github.com/Azure-Samples/cognitive-services-sample-data-files/blob/65af3b93cab5f45c76594f3265210646d4b3809e/ComputerVision/Images/handwritten_text.jpg)，固定提交、源图 SHA-256、MIT 声明和旋转关系见 `samples/oneocr-parity/manifest.json`。原图的两行在原 DLL 中均被判为手写。测试另覆盖 0.5°、3°、10°、90°、180°、270°，生成脚本核对固定源图摘要。

## 已恢复的处理

- 检测尺寸取整、RGB 输入、18 个检测头、SegLink 联接、片段去重与连接链、曲线拟合、真实多边形和区域合并。连接链使用 365 组原 DLL 合约验证；不把额外低分片段简单丢弃来拟合样例。
- 矩形与曲线区域的两种原始校正路径、边界与浮点求值次序、带步长输入、四边形词框反投影和空白区对齐。
- DLL 使用的阅读顺序比较器及 MSVC 排序流程。候选分组中的两次 FNV 哈希表遍历也用源码恢复，避免 Linux STL 或哈希冲突改变同分框顺序；原生探针包含桶扩容和非连续组编号。
- 文字类别、连续帧回退、全图投票、手写三态和方向判断。保留未知手写状态在内部的语义；JSON 按原 wrapper 的映射输出。201 组分类和 80 组全图决策合约覆盖分支边界。
- 原始 21 项拒识/置信度特征、大小写逻辑字符归并、20 路 CTC 候选、保留内部空格的词概率合并、字节编辑距离、同源置信度网络和手写校准表。UTF-8 字节距离另有 225 组原 DLL 探针，包含乘号、中文、希腊文、阿拉伯文和 emoji。
- 识别后的横竖方向去重、拒识均值与字节加权分数。识别去重使用 72 组原生合约。ICU 74.2 的源码与数据支持泰语字典切分和阿拉伯语/希伯来语视觉顺序还原。

OpenCV 4.10.0 与 ICU 74.2 从固定提交编译，模型保持独立文件。Windows 与 Linux 的 ORT 1.23.2 均已从固定提交构建。Linux 使用 GCC 15.2，源码增加显式 `<cstdint>` 标准头文件，并关闭隐式浮点融合；模型及算子中显式的 FMA 保留。

官方 Linux SDK 在两个固定用例上出现置信度超限。逐层探针定位到 `Add_22_quant`：隐式融合将一个中间值从 `75.5000229` 改成 `75.4999924`，量化输出随之从 76 变为 75。重建后，96 个检测中间张量与 Windows 逐字节一致，固定集恢复 27/27。数值契约见 `research/numerical-contracts.json`；旧失败记录保留，比较阈值未改。

## 可复现入口

```powershell
.research-python/python.exe scripts/run-corpus.py --backend native --binary build/source-runtime/native/Release/ocr.exe --output-dir work/oneocr-portable/replay-native
.research-python/python.exe scripts/summarize-parity.py --actual work/oneocr-portable/replay-native --output work/oneocr-portable/replay-parity/corpus-parity.json
.research-python/python.exe scripts/run-parity-suite.py --manifest samples/oneocr-parity/manifest.json --baseline work/oneocr-portable/handwriting-legacy --binary build/source-runtime/native/Release/ocr.exe --output work/oneocr-portable/replay-handwriting
```

手写原 DLL 基准可逐张通过 `scripts/run-legacy.py` 在独立进程生成。经典样例沿用 `reports/oneocr-classics-2026-11-02/legacy`，本阶段不改动其他任务生成的报告。其错误契约单独保存在案件的 `classics-error-baseline` 中；本地完整证据目录仍为 `work/oneocr-portable/`。

## 尚未完成的边界

有限回归通过不代表所有输入等价。退化曲线的备用求解器仍有显式未实现路径；当前主要验证默认参数，其他缩放参数、极长行裁剪、连接/方向歧义配置还需扩大探针覆盖。真实手写本阶段覆盖一个拉丁字母来源，中文及其他文字的真实手写仍需独立扩展集。

Windows x64 和 Linux x64 的构建/运行状态分别记录；macOS、Android、Linux ARM64 仍无目标环境实测。保留 `engine.parity: incomplete`，完整跨平台目标继续标记为未完成。原模型来源和权利声明保持原样，未改为 MIT，也不依赖账号、证书或在线授权服务。
