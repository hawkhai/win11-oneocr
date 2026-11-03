# Timeline (append-only)

## 2026-11-03T14:45:29.9348209+08:00 | lead | init
- action: case-init
- command_or_ref: skills/scripts/case-init.ps1
- result_summary: case directory created; scope ready_for_act=true
- artifacts: [scope.md, workitems.md]
- evidence_ids: []
- decision_delta: [case_initialized]
- carry_forward_refs: [scope.md]
- next: open PRIMARY SKILL.md and ACT within scope

## 2026-11-03 | lead | static + dynamic recovery
- action: analyze the fixed Photos DLL with Ghidra 11.3.2 headless; export LU/SVD, normalization and mapper functions; cross-check the old DLL
- artifacts: static/, assets.json, contract.json, oracle-old.json, oracle-crosscheck.json
- result: 8 polynomial cases, 7 normalization cases and 56 word boxes cross-version identical
- decision: retain exact-zero LU switching, zero target tail and all source triangles

## 2026-11-03 | lead | failing long-line scene and repair
- action: fresh DLL/native inference on all 114 images; compare detector input tensors and native resize call chain
- result: initial 627 differences on very-long-line.png; recovered AREA interpolation at area ratio <= 0.04; repaired image strict comparison zero failures
- artifacts: long-detector-input.f32; report diagnostics/windows-initial; evidence/long-area-comparison.json

## 2026-11-03 | lead | branch and full regression
- action: native Windows/Linux CTest and Wasm C++ oracle tests; fresh full 142 cases per environment
- result: branch tests passed on Windows, Linux and Wasm; full native Windows/Linux passed 142/142 each
- next: finish Windows/Linux Node and Chrome fresh runs, seal reports, verify offline package and commit
