# Case Scope

## meta
- case_id: windows-linux-closeout-2026-11-04
- created: 2026-11-04T00:37:41.3830897+08:00
- operator: local
- project_root: F:\pythonx\myocr\win11-oneocr
- primary_skill: ghidra-reverse/SKILL.md
- primary_id: R22
- lead_role: lead
- specialist_roles: []
- hint: Ghidra offline OneOCR Windows Linux native acceptance closeout
- preset: offline-sample

## auth
- status: granted
- basis: own_system
- evidence_of_auth: User explicitly sets Windows/Linux C++ all registered strict DLL parity as the stopping rule; record it in AGENTS.md, commit locally and refresh offline packages. Other targets do not block this task. No push or publication.
- MUST NOT proceed if status != granted

## in_scope
- assets:
  - F:\pythonx\myocr\win11-oneocr\bin\photos-2026.11090.22001.0\oneocr.dll
  - F:/pythonx/myocr/win11-oneocr/AGENTS.md
  - F:/pythonx/myocr/win11-oneocr/docs
  - F:/pythonx/myocr/win11-oneocr/research
  - F:/pythonx/myocr/win11-oneocr/reports
  - F:/pythonx/myocr/win11-oneocr/artifacts
- surfaces: [local_files, native_results, policy]
- activities: [verify_saved_results, verify_asset_hashes, record_stopping_rule, local_commit, offline_package]

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
  - [x] roles assigned (single local lead)

## ops_refs
- skills/ops/scope-contract.md
- skills/ops/evidence-finding-path.md
- skills/ops/role-map.md
- skills/ops/timeline-workitem.md
- skills/ops/IDENTITY.md
