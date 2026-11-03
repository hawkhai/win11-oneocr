# 源码还原 Finding / Path

背景与复现入口见[正式报告](../../../docs/2026-11-03_reverse-oneocr-options-handwriting-report.md)；结果见[严格回归](../../../reports/options-handwriting-2026-11-03/README.md)。这是兼容性研究，不赋予漏洞风险等级。

### F-001 行预算在整组识别及批内方向抑制后判断
- severity: n/a_re
- evidence_ids: E-001
- confidence: high
- location: Photos 180497500;180490ca0;1804a4c90; native/src/recognition_budget.hpp; native/src/engine.cpp
- status: validated

先移除 nontext，按整数外接框 minY/minX 排序，将异轴相交分量作为不可分组。批量为 max(4,trunc(round(remaining)*1.1))，完成本批识别与方向候选抑制后检查预算。十二行图 max-lines=1..4 均可返回四行，max-lines=11 可返回十二行；直接截断与 DLL 不同。

### F-002 多边形 NMS 之前有整数包围框粗筛
- severity: n/a_re
- evidence_ids: E-002
- confidence: high
- location: Photos 1805841e0; native/src/nms.cpp; native/test/nms_test.cpp
- status: validated

粗筛使用 floor/ceil 后包含 +1 边界的包围框。省略会让中文斜行局部框抑制整行；六组真实 Photos 候选和既有几何 oracle 构成回归，模型算法的 0.3 门限保持原值。

### F-003 CJK layout 输入应在 flip 校正后产生
- severity: n/a_re
- evidence_ids: E-003
- confidence: high
- location: native/src/engine.cpp; oracles/grid-10.json
- status: validated

倒置候选先旋转 180 度再执行版式网络；反向执行造成文字相同而框角点循环顺序与阅读顺序不符。原 DLL 与修复后源码的 classifier、recognizer、layout 中间张量另作逐字节校验。

### F-004 方向候选分数来自输出词置信度
- severity: n/a_re
- evidence_ids: E-004
- confidence: high
- location: Photos 180491610; LineResult+0x5c; native/src/engine.cpp
- status: validated

按输出词 UTF-8 字节数加权，float 乘法后用 double 累加；粗拒识概率不等于此分数。观测实际 LineResult、候选源索引和原函数选择，随后严格比较 DLL 最终输出，排除探针改变结果。

### P-001 版本约束的离线还原链
- path_type: solve
- evidence_ids: E-001, E-002, E-003, E-004
- finding_ids: F-001, F-002, F-003, F-004

固定 DLL/模型摘要 → 照片与有限参数矩阵 → 独立子进程生成基准 → Ghidra 只读静态分析 → 版本限定的进程内观测 → 张量/原函数选择校验 → C++ 修复与负例 → 五环境全新推理 → 相同严格比较器 → 封存索引与独立复核。失败阶段保留在 diagnostics，不替代最终执行。
