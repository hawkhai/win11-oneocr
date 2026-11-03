# Findings and Path

### F-001
- title: 旋转手写、多语种及版式参数交集已登记并实测
- severity: n/a_re
- category: reverse_algo
- status: validated
- evidence_ids: [E-002, E-003]
- confidence: high
- location: research/parameter-cases-2026-11-04.json; Photos 0x180497500/0x180490ca0/0x1804a4c90 (既有锚点)
- impact: 旧 67 组全部保留，增加 48 组；本轮无新书写者或新源码反编译结论。

### F-002
- title: 未改生产算法的 C++ 引擎通过扩展后的 538 项原生严格比较
- severity: n/a_re
- category: other
- status: validated
- evidence_ids: [E-001, E-002, E-003]
- confidence: high
- location: reports/parameter-expansion-2026-11-04; native/src/recognition_budget.hpp; native/src/engine.cpp
- impact: 532 个识别输出与 6 个拒绝契约；原有阈值、模型和平台缺口不变。

### F-003
- title: 历史快照复核不能代替当前全量验收
- severity: n/a_re
- category: other
- status: validated
- evidence_ids: [E-003]
- confidence: high
- location: scripts/verify-native-results.py; scripts/verify-recorded-parity.py; scripts/run-wasm-parity.py
- impact: 历史模式显式标记 current_scope=false；旧范围默认失败；Wasm 新推理拒绝历史快照。

### P-001
- title: 版本绑定的联合参数兼容性验证
- path_type: callflow
- evidence_ids: [E-001, E-002, E-003]
- finding_ids: [F-001, F-002, F-003]
- steps:
  1. 核验离线包、模型和原 DLL — evidence: E-001 — finding: F-001
  2. 固定图片和请求参数，独立进程调用原 DLL — evidence: E-002 — finding: F-001
  3. 两端 C++ 原生新推理并逐字段严格比较 — evidence: E-003 — finding: F-002
  4. 显式历史范围复核与当前范围反例 — evidence: E-003 — finding: F-003
- residual_risks: 有限图片、书写者和参数，ARM64/macOS/Android 未验证。
