# Case Scope

## meta
- case_id: curve-boundaries
- created: 2026-11-03T14:45:29.9348209+08:00
- operator: local
- project_root: F:\pythonx\myocr\win11-oneocr
- primary_skill: ghidra-reverse/SKILL.md
- primary_id: R22
- lead_role: lead
- specialist_roles: []
- hint: Ghidra offline oneocr.dll curve fallback and polygon clipping source recovery
- preset: offline-sample

## auth
- status: granted
- basis: own_system
- evidence_of_auth: User explicitly requested recovery of degenerate curve fallback and extreme line clipping, local probes, source builds, strict regression and local commit
- MUST NOT proceed if status != granted

## in_scope
- assets:
  - F:\pythonx\myocr\win11-oneocr\bin\photos-2026.11090.22001.0\oneocr.dll
  - F:\pythonx\myocr\win11-oneocr\bin\oneocr.dll
  - F:\pythonx\myocr\win11-oneocr\bin\photos-2026.11090.22001.0
  - F:\pythonx\myocr\win11-oneocr\models
  - F:\pythonx\myocr\win11-oneocr\native
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
  - [x] single agent performs lead/static/dynamic/testing roles; no delegation

## ops_refs
- skills/ops/scope-contract.md
- skills/ops/evidence-finding-path.md
- skills/ops/role-map.md
- skills/ops/timeline-workitem.md
- skills/ops/IDENTITY.md