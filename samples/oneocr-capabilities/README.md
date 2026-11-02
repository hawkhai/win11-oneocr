# 微软公开能力场景补充样本

18 张新增图片。原图、生成图、派生图在 `manifest.json` 中分别标注；每张记录 SHA-256 和尺寸。此前增加的真实手写原图及六个旋转版本位于 `../oneocr-parity/`。

## 能力依据与适用范围

| 来源 | 可核查的说明（概述） | 对应样本 |
| --- | --- | --- |
| [微软研究员 ICDAR 2019 OneOCR 主题演讲摘要，第 7 页](https://iapr.org/archives/icdar2019/wp-content/uploads/2019/08/MainConfAdv_forWeb.pdf) | 拍摄图中的印刷、手写、英语混合行及任意方向；列举票据、名片、菜单、标签、海报等场景 | `azure-printed-photo.jpg`、`printed-and-real-handwriting.png`、`../oneocr-parity/` 的真实手写旋转组；既有 `../ocr-classics/images/` 中的相应场景 |
| [微软 Snipping Tool Text Actions 公告，2023-09-14](https://blogs.windows.com/windows-insider/2023/09/14/new-updates-for-snipping-tool-and-phone-link-begin-rolling-out-to-windows-insiders/) | 截图文字选择、复制；邮件地址和电话快速遮盖 | `dark-contact-screenshot.png` |
| [微软 Snipping Tool Copy as table 公告，2024-10-17](https://blogs.windows.com/windows-insider/2024/10/17/announcing-windows-11-insider-preview-build-27729-canary-channel/) | 从单表格截图复制结构化数据 | `single-table.png` |
| [Windows OCR 平台能力卡](https://learn.microsoft.com/en-us/windows/ai/cards/text-recognition-ocr-platform-card) | 本地运行，印刷与常见手写，语言自动识别，行与边界；图像质量、复杂布局和字体会影响结果 | `azure-multilingual.png`、`script-*.png`、`mixed-nine-scripts.png`、`low-contrast-small-blur.png` |
| [Windows OCR API 入门](https://learn.microsoft.com/en-us/windows/ai/apis/text-recognition) | 词、行、多边形边界、置信度 | 全部文字样本都比较这些输出 |

核查日期：2026-11-02。2019 演讲明确点名 OneOCR，但不证明其模型与本地 9.0.103 相同。Windows AI API 文档描述当前 NPU 产品，也不能证明与本地 DLL 同版。上述材料用来选择测试场景；不把其他产品的支持语言数量、表格导出、遮盖功能或性能宣称移植为本项目已实现能力。表格和联系方式样本测试的是底层 OCR 输出一致性，未实现 Excel 结构导出或自动遮盖。

九类文字来自本地提取模型与字表的实际角色，不是微软对当前 DLL 的公开语言保证。生成字体不算真实手写。英文手写之外的真实多语种手写仍是覆盖缺口。

## 来源和重建

三个 Azure 原图固定到 [Azure-Samples/cognitive-services-sample-data-files 的提交 65af3b9](https://github.com/Azure-Samples/cognitive-services-sample-data-files/tree/65af3b93cab5f45c76594f3265210646d4b3809e/ComputerVision/Images)，保留原始字节：`MultiLingual.png`、`printed_text.jpg`、`dog.jpg`。这是公开样本的来源，不代表 Azure 与当前 DLL 是同一引擎。沿用仓库 MIT 声明，副本见 `../oneocr-parity/LICENSE.azure.txt`；不涉及模型权重许可。

其余 15 张由 `scripts/make-capability-samples.py` 在本地使用 Pillow 和已安装字体绘制，手写混排图无缩放地粘贴已有真实手写图。清单记录字体文件校验值；不分发字体。生成文本不是精标真值，识别错误也必须忠实复现原 DLL。

从仓库根目录运行：

```powershell
.research-python/python.exe -X utf8 scripts/make-capability-samples.py
```

该命令离线校验三个原图后重新生成派生图和清单，不下载或替换任何模型。
