# Case Scope

## meta
- case_id: word-box-zip-p2
- created: 2026-11-03T17:09:42.7098714+08:00
- operator: local
- project_root: F:\pythonx\myocr\win11-oneocr
- primary_skill: ghidra-reverse/SKILL.md
- primary_id: R22
- lead_role: lead
- specialist_roles: []
- hint: Ghidra offline OneOCR curve evidence follow-up
- preset: offline-sample

## auth
- status: granted
- basis: own_system
- evidence_of_auth: User explicitly requests both P2 fixes, negative tests, strict full saved-output verification, local commits and refreshed offline packages; no model or tolerance changes, no push.
- MUST NOT proceed if status != granted

## in_scope
- assets:
  - F:\pythonx\myocr\win11-oneocr\native\tests\boundary_test.cpp
  - F:/pythonx/myocr/win11-oneocr/native/tests
  - F:/pythonx/myocr/win11-oneocr/scripts
  - F:/pythonx/myocr/win11-oneocr/test
  - F:/pythonx/myocr/win11-oneocr/reports
  - F:/pythonx/myocr/win11-oneocr/artifacts
  - F:/pythonx/myocr/win11-oneocr/native
  - F:/pythonx/myocr/win11-oneocr/docs
  - F:/pythonx/myocr/win11-oneocr/research
  - F:/pythonx/myocr/win11-oneocr/README.md
  - F:/pythonx/myocr/win11-oneocr/.gitattributes
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
  - [x] out_of_scope reviewed
  - [x] one agent performs scope, implementation, tests and evidence roles; no delegation

## ops_refs
- skills/ops/scope-contract.md
- skills/ops/evidence-finding-path.md
- skills/ops/role-map.md
- skills/ops/timeline-workitem.md
- skills/ops/IDENTITY.md