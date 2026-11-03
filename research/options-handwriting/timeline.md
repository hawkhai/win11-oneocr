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

## 2026-11-03T19:55:30+08:00 | lead | full inference and negative tests
- action: finish five fresh environments and full Python negative suite
- artifacts: reports/options-handwriting-2026-11-03; implementation commit dfa477b8320d4e485aa9b23ffdb48735dbef8ac3
- evidence_ids: [E-001, E-002, E-003, E-004]
- result_summary: all five environments 221/221; 1090 recognition outputs plus 15 expected rejection contracts; 62/62 Python tests; 10/10 native CTest on each host; real API and isolated offline book parity passed
- decision_delta: preserve incomplete overall goal and every unverified target; do not alter model or comparator
- next: seal final report and independently recompare 1105 saved outcomes; commit all records; then create exact-HEAD packages and verify a clean bundle checkout

## 2026-11-03T19:58:00+08:00 | lead | read-only re-verification
- action: independently compare sealed outputs and check scope/runtime identities
- artifacts: reports/options-handwriting-review-2026-11-03/verification.json
- evidence_ids: [E-001, E-002, E-003, E-004]
- result_summary: 1105/1105 saved outcomes passed; fresh_inference=false; full Python suite 62/62; case-review strict hash verification PASS with no warnings
- next: final local commit followed by package creation, ZIP index verification and fresh bundle checkout smoke tests; exact final-HEAD package proof is generated outside Git to avoid self-reference
