# Case Scope

## meta
- case_id: options-handwriting
- created: 2026-11-03T17:56:23.3935848+08:00
- operator: local
- project_root: F:\pythonx\myocr\win11-oneocr
- primary_skill: ghidra-reverse/SKILL.md
- primary_id: R22
- lead_role: lead
- specialist_roles: []
- hint: Ghidra offline OneOCR parameter and Chinese handwriting parity
- preset: offline-sample

## auth
- status: granted
- basis: own_system
- evidence_of_auth: User explicitly requests nondefault options and real Chinese handwriting coverage, fixed-model strict full DLL regression, local commits and package refresh. Public sample acquisition is authorized separately from offline DLL/model observations. No model replacement, tolerance changes or upload.
- MUST NOT proceed if status != granted

## in_scope
- assets:
  - F:\pythonx\myocr\win11-oneocr\bin\photos-2026.11090.22001.0\oneocr.dll
  - F:/pythonx/myocr/win11-oneocr/native
  - F:/pythonx/myocr/win11-oneocr/scripts
  - F:/pythonx/myocr/win11-oneocr/web
  - F:/pythonx/myocr/win11-oneocr/test
  - F:/pythonx/myocr/win11-oneocr/samples
  - F:/pythonx/myocr/win11-oneocr/reports
  - F:/pythonx/myocr/win11-oneocr/artifacts
  - F:/pythonx/myocr/win11-oneocr/research
- surfaces: []
- activities: []

## out_of_scope
- assets: []
- activities: [dos, phishing_real_users, unrestricted_exfil]

## network_profile
- mode: offline
- notes: |
    offline | lab_only | authorized_target_only | unrestricted_lab
    Change mode only after auth.status = granted.

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
  - [ ] out_of_scope reviewed
  - [ ] roles assigned (see skills/ops/role-map.md)

## ops_refs
- skills/ops/scope-contract.md
- skills/ops/evidence-finding-path.md
- skills/ops/role-map.md
- skills/ops/timeline-workitem.md
- skills/ops/IDENTITY.md