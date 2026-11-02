# Case Scope

## meta
- case_id: oneocr-portable
- created: 2026-11-02T10:52:06.8916884+08:00
- operator: local
- project_root: F:\pythonx\myocr\win11-oneocr
- primary_skill: ghidra-reverse/SKILL.md
- primary_id: R22
- lead_role: lead
- specialist_roles: [cre, cce, cae, doc]
- hint: offline oneocr.dll native OCR source recovery
- preset: offline-sample

## auth
- status: granted
- basis: own_system
- evidence_of_auth: User requested implementation of the reviewed local offline research and native source plan
- MUST NOT proceed if status != granted

## in_scope
- assets:
  - F:\pythonx\myocr\win11-oneocr\bin\oneocr.dll
  - F:\pythonx\myocr\win11-oneocr\bin\oneocr.onemodel
  - F:\pythonx\myocr\win11-oneocr\bin\onnxruntime.dll
  - F:\pythonx\myocr\win11-oneocr\bin\oneocr_wrapper.dll
- surfaces: [local_binary, local_model, native_source]
- activities: [static_analysis, local_instrumentation, model_extraction, source_build, differential_tests, report]

## out_of_scope
- assets: []
- activities: [push, publish, upload_models, replace_original_samples]

## network_profile
- mode: offline
- notes: |
    Target research and inference remain local/offline. Public source and
    development-tool downloads are separate setup operations disclosed in chat.

## deliverables
- report: true
- field_journal: true
- diagrams: true
- timeline: true

## constraints
- timebox: {}
- stealth: low
- data_handling: anonymize

## signoff
- ready_for_act: true
- checklist:
  - [x] auth.status = granted
  - [x] in_scope.assets non-empty OR offline sample path set
  - [x] network_profile.mode chosen
  - [x] out_of_scope reviewed
  - [x] roles assigned (single agent performs the documented roles)

## ops_refs
- skills/ops/scope-contract.md
- skills/ops/evidence-finding-path.md
- skills/ops/role-map.md
- skills/ops/timeline-workitem.md
- skills/ops/IDENTITY.md
