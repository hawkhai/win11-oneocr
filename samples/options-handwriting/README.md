# 真实中文手写与参数控制图

`red-grid-jason.jpg`、`red-calligraphy-jason.jpg` 为 jason hu (@hujason) 发布的真实纸面照片，均由 Unsplash CDN 的 `w=1600` 版本获取。摄影者没有单独注明书写者，不能据此声称覆盖两个不同书写者。原图、来源页面、许可地址、SHA-256 和派生操作保存在 [provenance.json](provenance.json)。

各保留 90°、180°、270° 的像素转置图，以及扩展白底、BICUBIC 插值的 10° 旋转图。两张来源照片加八张派生图共十张手写输入；它们不是十份独立来源。照片中本来存在的印刷表头、格线、透视、裁边和遮挡一并保留。

`twelve-printed-lines.png` 是明确标记为印刷体的十二行中英混排控制图，`parameter-blank.png` 是空白控制图。它们用于行预算与 resize 边界检查，不作为手写覆盖证据。

重建派生图：先按 provenance 中的 URL 获取两张来源照片，使用 `scripts/prepare-options-handwriting.py --acquired <目录>`；印刷控制图另需记录中相同的本机字体。运行已提交的回归样本无需下载照片或安装字体。仓库不分发字体文件。

识别验收目标是与原 DLL 一致，不等于人工转录正确率；照片中的真实手写仍可能被原 DLL 标为 printed，此类输出也必须一致。
