# Timeline (append-only)

## 2026-11-02T10:52:06.8916884+08:00 | lead | init
- action: case-init
- command_or_ref: skills/scripts/case-init.ps1
- result_summary: case directory created; scope ready_for_act=true
- artifacts: [scope.md, workitems.md]
- evidence_ids: []
- decision_delta: [case_initialized]
- carry_forward_refs: [scope.md]
- next: open PRIMARY SKILL.md and ACT within scope
## 2026-11-02T12:41:01.418458+08:00 | cre | sample-and-model-analysis
- action: record measured phase transition (retrospective summary)
- command_or_ref: scripts/research.py; scripts/prepare-models.py
- result_summary: Retrospective phase summary: original hashes preserved; 67 resources, 34 graphs and nine complete vocabularies exported without DLL loading.
- artifacts: [evidence/inventory.json, unpacked/resources.json]
- evidence_ids: [E-001, E-002]
- decision_delta: [container_mode=validated_CFB128, vocab_source=original_resources]
- carry_forward_refs: [scope.md, workitems.md]
- next: continue full algorithm recovery from WI-005; no completion claim

## 2026-11-02T12:41:01.418458+08:00 | cre | private-layer-validation
- action: record measured phase transition (retrospective summary)
- command_or_ref: scripts/convert-auxiliary.py --verify
- result_summary: Retrospective phase summary: 23 private layers translated; native-probed maximum error 1.767277717590332e-05. Layout requires Identity after Conv.
- artifacts: [evidence/auxiliary-conversion.json]
- evidence_ids: [E-003]
- decision_delta: [private_graphs=portable, end_to_end_parity=unproven]
- carry_forward_refs: [scope.md, workitems.md]
- next: continue full algorithm recovery from WI-005; no completion claim

## 2026-11-02T12:41:01.418458+08:00 | cae | source-build-and-offline-run
- action: record measured phase transition (retrospective summary)
- command_or_ref: scripts/build-native.ps1 -SourceRuntime; scripts/verify-offline.py
- result_summary: Retrospective phase summary: Windows source ORT and native build passed; Linux native build and network-isolated staged inference passed. Other targets remain unverified.
- artifacts: [evidence/verification.json, linux-staged-offline.verification.json, evidence/windows-linux-parity.json]
- evidence_ids: [E-004, E-005, E-006]
- decision_delta: [windows_linux_native=running, other_platforms=unverified]
- carry_forward_refs: [scope.md, workitems.md]
- next: continue full algorithm recovery from WI-005; no completion claim

## 2026-11-02T12:41:01.418458+08:00 | cre | strict-differential-validation
- action: record measured phase transition (retrospective summary)
- command_or_ref: scripts/summarize-parity.py; scripts/compare-results.py
- result_summary: Retrospective phase summary: original and native corpus processes 27/27 successful; strict parity only 1/27. Book line counts original/native/reference 105/123/58. Thresholds unchanged.
- artifacts: [evidence/parity-ocr-book.json, evidence/corpus-parity.json, evidence/parity-reference-ocr-book.json]
- evidence_ids: [E-007, E-008, E-009]
- decision_delta: [full_goal=incomplete, WI-005=in_progress]
- carry_forward_refs: [scope.md, workitems.md]
- next: continue full algorithm recovery from WI-005; no completion claim

## 2026-11-02T12:41:01.418458+08:00 | doc | phase-evidence-review
- action: record measured phase transition (retrospective summary)
- command_or_ref: case-review/scripts/review_case.py --verify-hashes --strict
- result_summary: Phase report, editable pipeline diagram, source references and anonymized local field journal recorded. Strict case review follows; it does not certify algorithm equivalence.
- artifacts: [report.md, field-journal.md, notes/references.md]
- evidence_ids: [E-001, E-002, E-003, E-004, E-005, E-006, E-007, E-008, E-009]
- decision_delta: [phase=review, completion_claim=withheld]
- carry_forward_refs: [scope.md, workitems.md]
- next: continue full algorithm recovery from WI-005; no completion claim

## 2026-11-02T12:46:28.047582+08:00 | cae | source-archive-proof
- action: verify standalone source handoff
- command_or_ref: scripts/package-source.py; CMake source-archive-check; scripts/compare-results.py
- result_summary: Source archive contains no compiled binaries or model weights. Extracted source built on Windows; 2/2 CTests passed; its native result matches the staged native program for the test image. This does not establish original DLL parity.
- artifacts: [evidence/source-archive.json, evidence/source-archive-build.log, evidence/source-archive-parity.json]
- evidence_ids: [E-004]
- decision_delta: [source_archive=build_verified]
- carry_forward_refs: [scope.md, workitems.md, report/phase-report.md]
- next: full semantics remain WI-005 in_progress

## 2026-11-02T18:45:30.483034+08:00 | lead | phase2-strict-parity
- action: recover default detector, alignment, ordering, confidence, handwriting and orientation; rebuild deterministic CPU backend
- command_or_ref: scripts/run-corpus.py; scripts/run-parity-suite.py; scripts/build-ort.py; scripts/verify-offline.py; scripts/package-source.py
- result_summary: Windows x64 and WSL Linux x64 each pass 64 strict recognition cases plus one original-DLL image-size error contract; book 105/105 lines; 8/8 unit contracts; independent contexts tested. Linux ORT now source-built with implicit FP contraction disabled. Extracted source archive builds and passes book parity.
- artifacts: [evidence/phase2-verification.json, evidence/phase2-linux-ort-source.json, evidence/phase2-detector-source-linux-parity.json, evidence/phase2-source-archive-verification.json]
- evidence_ids: [E-010, E-011, E-012, E-013]
- decision_delta: [fixed_default_regressions=passed, models=unchanged, tolerances=unchanged, full_goal=incomplete]
- next: WI-005 covers remaining fallback/configuration paths and unverified target environments

## 2026-11-02T19:14:02.086012+08:00 | lead | all-samples-strict-parity
- action: supplement official-capability scenarios; inventory every image including contact sheets and references; recover remaining observed geometry differences
- result_summary: Windows x64 and WSL Linux x64 each pass 110/110 images (107 recognition, 3 explicit errors), plus original 27-case corpus and 105-line book. 8/8 CTests, 2112 polygon oracle pairs, 72 recognition NMS cases. All sample images committed; no model or tolerance changes.
- evidence_ids: [E-014, E-015]
- decision_delta: [sample_inventory=110, all_image_regressions=passed, original_assets=unchanged, full_goal=incomplete]
- carry_forward_refs: [scope.md, workitems.md, evidence/E-014.md, evidence/E-015.md]
- next: retain unimplemented fallback/configuration paths and unverified macOS/Android/ARM64 targets as incomplete

## 2026-11-02T19:59:24.799505+08:00 | lead | photos-model-update
- decision_delta: [source_model=f6cef38b, native_dll_reference=11.0.15, portable_ORT=1.23.2_retained, all_regressions=passed, full_goal=incomplete]
- results: Windows/Linux each 107 strict recognition + 3 original errors, 27 fixed cases and 106-line book; 8 CTests and ABI pass. Old 9.0.103 preparation remains compatible (105-line book).
- evidence_ids: [E-017]
- evidence_correction: E-016 linked an old artifact by mistake; its immutable record is preserved, E-017 hashes the correct new summary.
- carry_forward_refs: [scope.md, reports/photos-model-update-2026-11-02/README.md]
