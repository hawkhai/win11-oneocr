# 固定研究来源

这些来源只作为本地研究与构建依据，不改变模型权利声明。完整目标仍未完成。

| 来源 | 固定版本与使用范围 |
| --- | --- |
| [reverse-skill](https://github.com/zhaoxuya520/reverse-skill/tree/cab634bd855fc287f6e420c1f36fd1a6b9245960) | 路由、离线 scope、Ghidra 分析、证据图和阶段报告流程；本地副本在 `work/vendor/reverse-skill` |
| [oneocr-onnx-python](https://github.com/bropines/oneocr-onnx-python/tree/75cc12666503425ffd6ea0cb052c0bcaaff21058) | 参考提取与推理流程；未调用下载器。其识别类别、词表清洗和几何近似差异已记录，不能当成 DLL 等价算法 |
| [oneocr-native](https://github.com/shiyori/oneocr-native/tree/6020d4339ce2feb835393e2512e433baec29f8d3) | 容器格式参考 `onemodel_container.go`，派生的独立准备工具采用 AGPL-3.0-only；当前样本 CFB128 由本地动态观察核实 |
| [ONNX Runtime 1.23.2 源码](https://github.com/microsoft/onnxruntime/tree/a83fc4d58cb48eb68890dd689f94f28288cf2278) | CPU 推理运行库；Windows 已从源码构建，Linux 本轮使用同版本官方 SDK |
| [Ghidra 11.3.2 发布](https://github.com/NationalSecurityAgency/ghidra/releases/tag/Ghidra_11.3.2_build) | 原 DLL 静态分析；导出脚本在 `scripts/ghidra/ExportPipeline.java` |

模型来源是本地 `bin/oneocr.onemodel`。图、字表、组合字符与配置以资源名和 SHA-256 绑定；未使用其他 OCR 权重。

依赖与 SDK 摘要见 [dependencies.json](dependencies.json)，私有层转换合约见 [portable-model-contracts.json](portable-model-contracts.json)，实测状态见 [verification.json](verification.json)。
