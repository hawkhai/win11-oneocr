# 非默认参数与真实中文手写证据

使用固定 reverse-skill `cab634bd855fc287f6e420c1f36fd1a6b9245960`，走 R22 / Ghidra 离线样本路由。`scope.md`、路由、时间线与证据记录随项目保存。公开照片获取独立于离线 DLL／模型观测；没有联网 OCR、模型替换或上传。

基准仍为 Photos 2026.11090.22001.0 / OneOCR 11.0.15，DLL SHA-256 `0a5e99041af902d6825f0af04ab25e62d2e2378ace69d10175f0dcc22816aae0`，模型 `f6cef38b839012cd824abf8b854ee9fa4f87d4c5265440661d66b23f4fab5155`。原始 wrapper、配套 ORT 与模型摘要都记录在每份 oracle 中，正式源码运行仍用 ORT 1.23.2。

67 个参数组合在 [parameter-cases.json](../parameter-cases.json) 中逐项登记路径、图片摘要和请求参数：十二行控制图上 11 种行预算 × 5 种有效 resize，共 55 项；两张真实手写照片各三组，共 6 项；零别名 2 项；空白边界 4 项。不是对所有整数参数的穷举。

Ghidra 静态位置与动态观测共同确定四处处理：

- `180497500`、`180490ca0`、`1804a4c90`：先剔除 nontext，按整数行框左上角排序，保持横竖相交分量完整；实际批量为 `max(4, trunc(round(remaining)*1.1))`，每批做方向候选抑制后再判断预算。因此 max-lines=1 可返回四行，不能直接截断为一行。
- `1805841e0`：多边形 IoU 之前仍需包含 +1 边界的外接矩形粗筛。省略它会让局部斜框错误压掉更长的真实手写行。
- 翻转后的实际 layout 输入张量：CJK 版式网络在 flip 校正后执行。原实现的位置颠倒，曾造成同一词框的角点顺序和阅读顺序差异。
- `180491610` 的实际 LineResult+0x5c：方向候选使用输出词置信度按 UTF-8 字节数加权，非粗词组拒识概率。`choices` 保存实际 DLL 输入、聚合分数和保留的源索引。

`oracles/index.json` 将六次观测绑定工具源码、图片和冻结基准。`scripts/capture-options-oracles.py` 在独立进程运行版本限定的探针，并再次严格比较观测后的 DLL 最终 JSON，确认观测没有改变输出。探针仅修改子进程内的 vtable／一个已核对的 callsite，调用原函数并在退出前还原；磁盘 DLL 与模型不改写。固定 RVA 不得用于其他版本。

这批证据加入 Windows／Linux 的排序、几何、方向候选 CTest；完整图片另走同一严格比较器。执行状态见[本轮报告](../../reports/options-handwriting-2026-11-03/README.md)。源码 ZIP 按报告索引收齐证据；既有词框 NaN 负例和旧报告保持保留。

当前 DLL 的 resize setter 上限为 width=1152、height=768，超过时 wrapper 返回 -20。原生 API 既有的 32..8192 范围属于扩展，超出共同范围不声明 DLL 输出等价。零别名的基准明确使用有效默认值 1000 / 1152×768，不冒称 raw wrapper 跳过 resize setter 的调用语义相同。更多参数、歧义分支、书写者与文字类别仍待覆盖；未实测平台保持未验证，`engine.parity` 继续为 incomplete。
