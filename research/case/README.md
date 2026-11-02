# 本次会话研究快照

本目录从本机 `work/oneocr-portable/` 保存 scope、workitems、timeline、field journal、证据、早期报告及固定回归输入/基准。历史记录保留当时的失败结果和机器路径，不将它们改写为最终通过结果；最新验收见 `reports/oneocr-all-samples-2026-11-02/`。

历史相对引用中的 `work/oneocr-portable/` 可映射到本目录阅读。原 27 张生成图片和 DLL 基准位于 `corpus/` 与 `corpus-legacy/`，105 行书页的原 DLL 基准位于 `baseline/ocr-book.json`。回归脚本在本机工作目录缺失时自动使用这些已提交文件。

原始 DLL 与容器已由原仓库跟踪在 `bin/`，最终独立资源位于根目录 `models/`。多次重复提取的模型、数十 GB 实验张量、Ghidra 数据库、下载工具和编译缓存不属于运行依赖，没有复制到本快照。必要的最终行为合约、逐图结果与来源指纹均已保存，内部观测可通过固定输入与已提交研究脚本重新生成。

本目录是历史证据，不是运行时配置。模型来源与权利不因本地提交而变化。
