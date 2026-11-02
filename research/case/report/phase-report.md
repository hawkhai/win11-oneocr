# OneOCR source recovery phase report

> Historical phase 1 report below. Phase 2: E-010..E-013 validate 65/65 default-pipeline regressions on each of Windows x64 and WSL Linux x64, source-built ORT on both, network-isolated staged Linux recognition, and a fresh build from the source-only archive. Current repository report: ../../../docs/2026-11-02_oneocr-parity-phase2.md. Full Android/macOS/ARM64 and uncovered algorithm paths remain incomplete.

Status: native prototype delivered for review; full equivalence goal remains incomplete.
Scope: [scope.md](../scope.md). Timeline: [timeline.md](../timeline.md).
Repository report: ../../../docs/2026-11-02_oneocr-source-report.md.

### F-001
- title: Standalone model preparation is possible without loading original DLLs
- severity: n/a_re
- category: reverse_algo
- status: validated
- evidence_ids: [E-001, E-002, E-003]
- location: scripts/unpack-onemodel.py; scripts/prepare-models.py
- confidence: high
- impact: 67 resources, 34 graphs and nine complete original vocabularies exported locally.
- residual_risk: Model ownership remains unchanged; future model versions require new format/graph validation.

### F-002
- title: Native source code builds and runs on Windows and Linux
- severity: n/a_re
- category: reverse_algo
- status: validated
- evidence_ids: [E-004, E-005, E-006]
- location: native/; research/verification.json
- confidence: high
- impact: Windows ORT was source-built; Linux native inference also ran in a network namespace without outbound connectivity.
- residual_risk: Linux used the official pinned ORT SDK; ARM64, Android and macOS were not validated.

### F-003
- title: Full DLL parity has not been achieved
- severity: n/a_re
- category: reverse_algo
- status: validated
- evidence_ids: [E-007, E-008, E-009]
- location: native/src/engine.cpp; scripts/compare-results.py
- confidence: high
- impact: Native execution succeeds but strict corpus parity passes only 1/27 cases; 105 original book lines versus 123 native lines.
- residual_risk: Curved geometry, rejection/confidence features, segmentation, ordering and full style/orientation decisions remain to be recovered.

### P-001
- title: Local container to measured native inference
- path_type: callflow
- start: Original local OneModel container
- goal: Offline native source runtime with measured fidelity
- steps:
  1. Fix sample hashes and PE anchors — evidence: E-001 — finding: F-001
  2. Unpack and validate independent models — evidence: E-002, E-003 — finding: F-001
  3. Source build and test native interfaces — evidence: E-004 — finding: F-002
  4. Run Linux offline and compare native platforms — evidence: E-005, E-006 — finding: F-002
  5. Compare original and reference outputs without suppressing failures — evidence: E-007, E-008, E-009 — finding: F-003
- residual_risks: Full equivalence remains incomplete and is explicitly exposed in runtime capabilities.
