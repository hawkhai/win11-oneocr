# 2026-11-03 OneOCR 离线处理还原

## 场景与目标

二进制分析 / 原生兼容性。补充非默认参数和真实中文手写，按相同严格比较器还原原 DLL 的处理，不更换模型。

## Scope 摘要（脱敏）

- auth_basis: 用户明确授权本地研究、修复、样本获取、回归、commit 和打包
- network_profile: DLL/模型观测 offline；公开图片独立获取，不上传输入或模型
- asset_types: Windows DLL、独立模型、本地样本、C++/Wasm 源码
- lead_role: lead
- specialists: []

## 执行链路

1. 固定 reverse-skill、原 DLL、模型、比较器及来源摘要，保存 scope 和路由。
2. 获取两个有公开来源声明的真实纸面手写图像版本，生成八张变换图与两个明确标注的控制图。
3. 67 个参数组合逐项记录请求参数、有效参数、图像和黄金输出摘要；独立子进程生成完整原始基准。
4. Ghidra 只读分析预算批次、粗包围框和方向候选；版本约束探针调用原函数、记录实际对象，结束前还原进程内改动。
5. 用张量/原函数选择和最终 JSON 双重校验，修复四处处理；保留失败诊断。
6. Windows/Linux 原生与三个 Wasm 环境全新推理，随后独立只读复核、封存报告、干净提交及精确 HEAD 打包。

## Evidence 链摘要

| E-id | severity | status | source_type | 可复用命令模式 | 关联 Finding |
|---|---|---|---|---|---|
| E-001 | n/a_re | validated | command | `python scripts/capture-options-oracles.py --output work/fresh-oracles` | F-001 |
| E-002 | n/a_re | validated | command | 同上，调用原 NMS | F-002 |
| E-004 | n/a_re | validated | command | 同上，实际 LineResult 观测 | F-004 |

## Finding / Path

- top_finding: 小行数参数是软批次预算，先完成整组和批内方向抑制，不能截断。
- path_type: solve
- path_one_liner: 固定版本 → 隔离原始基准 → 静态/动态观测 → 张量校验 → 源码修复 → 全新严格回归。

## 踩坑与工具链

| 问题 | 原因 | 处理 |
|---|---|---|
| 手写整行被局部框替代 | 只有 polygon IoU，缺少整数 bbox 粗筛 | 还原原函数粗筛，保留真实候选对 |
| 角点与顺序不同 | CJK layout 在 flip 前执行 | 还原调用顺序，逐字节核对张量 |
| 候选方向选错 | 使用粗拒识概率 | 捕获实际输出词置信度加权分数 |
| Python 测试发现为零 | unittest discover 默认跳过带连字符文件名 | 显式加载测试模块并拒绝零测试 |
| Windows 新检出可能改变字节 | 原始参数/证据摘要绑定 CRLF | 明确原始数据和相关工具的 -text 属性，核对 Git blob |

版本：Ghidra 11.3.2、JDK 21.0.8+9、ORT 1.23.2、Node 24；目标 Photos OneOCR 11.0.15。固定 RVA 探针不适用于其他版本。

## 可复用模式与进化动作

版本摘要检查必须先于 hook；原函数实际输入输出比重写算法的猜测更可靠；同图不同参数的身份必须进入输入清单与存档复核。包内报告索引应递归覆盖所有原始证据，最终包摘要与最终 commit 放在可再生产物侧避免自引用。

本次只写项目本地案例与本地索引，不修改固定版本的 reverse-skill，也不上传社区。更多书写者、参数和未验证平台仍需独立实测。
