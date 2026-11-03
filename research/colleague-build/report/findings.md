# Findings and Path

### F-001
- title: 依赖就绪的 Windows 构建不再强制研究 Python 或 WSL
- severity: n/a_re
- category: other
- status: validated
- evidence_ids: [E-001, E-002]
- confidence: high
- location: scripts/build-native.ps1; docs/colleague-build.md
- impact: 显式外部 SDK/源码/数据路径；只有 SourceRuntime 才准备依赖。

### F-002
- title: 两端重编 C++ 在当前全部登记范围内严格通过
- severity: n/a_re
- category: other
- status: validated
- evidence_ids: [E-002]
- confidence: high
- location: reports/colleague-build-2026-11-04
- impact: 两端各 269 项，532 识别与 6 拒绝；没有改模型、算法或阈值。

### P-001
- title: 源码与外部依赖的交付验证
- path_type: callflow
- evidence_ids: [E-001, E-002]
- finding_ids: [F-001, F-002]
- steps:
  1. 自行准备固定依赖并显式提供目录 — evidence: E-001 — finding: F-001
  2. 干净源码构建并运行原生测试 — evidence: E-002 — finding: F-002
  3. 全量严格回归、提交文本文件和刷新包后收手 — evidence: E-002 — finding: F-002
- residual_risks: 其他平台、系统和编译器未由本轮验证；有限样本不证明任意输入。
