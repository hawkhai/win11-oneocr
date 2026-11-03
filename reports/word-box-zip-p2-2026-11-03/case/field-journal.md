# 词框与源码 ZIP P2 跟进

固定 reverse-skill cab634bd855fc287f6e420c1f36fd1a6b9245960，继续 R22 离线证据链。本轮使用既有 Ghidra 导出与固定 DLL 预言机，不重复改变或重新分析同一二进制。

NaN 的 abs 差值与阈值比较为 false，不能用仅 >tolerance 的断言证明最终坐标有效。测试层统一要求 actual 与 golden 的每个 x/y 有限；原数值引擎为退化三角形保留的 NaN 分支选择未改动。相同真实预言机循环注入 NaN，旧提交退出 0，修复后退出 1；移除新有限性 guard 时，新增负例会明确失败。

SOURCE_BUILD 自洽只能证明 ZIP 内已有内容，不代表包含外部封存报告需要的全部证据。按各 FILES.sha256.json 预检并收齐所有扩展名，再从 ZIP 内逐索引验证。旧包缺少 10 个 gzip 日志；负例用自洽 SOURCE_BUILD 构造缺失/篡改证据，防止同一问题回归。

源码根 Git 目录由 Windows 管理，WSL 变异编译使用 Windows Git 导出的旧测试源码副本，不修改共享 Git 配置。全部推理运行文件、模型、图片与比较器保持字节一致，故本轮做完整保存结果复核，明确 fresh_inference=false；不计为新的 710 次推理。正式包在报告提交之后生成，不把包含自身提交的包嵌套进 Git。
