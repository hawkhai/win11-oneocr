# Timeline (append-only)

## 2026-11-03T17:56:23.3935848+08:00 | lead | init
- action: case-init
- command_or_ref: skills/scripts/case-init.ps1
- result_summary: case directory created; scope ready_for_act=true
- artifacts: [scope.md, workitems.md]
- evidence_ids: []
- decision_delta: [case_initialized]
- carry_forward_refs: [scope.md]
- next: open PRIMARY SKILL.md and ACT within scope
## 2026-11-03T19:09:47.831071+08:00 | lead | validated source recovery
- action: freeze 221 original-DLL cases; recover coarse bbox gate, batch budgets, flipped layout input and confidence aggregation
- artifacts: oracles/index.json; reports/options-handwriting-2026-11-03/reference
- evidence_ids: [E-001, E-002, E-003, E-004]
- result_summary: Windows native 221/221; Windows and Linux CTest 10/10; Wasm build and four C++ test executables passed; 53 Python core tests passed; other fresh full executions in progress
- next: collect every environment, independently verify immutable reports, commit and regenerate local packages
