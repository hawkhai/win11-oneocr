# Work Items

| ID | title | role | targets | surface | status | evidence | notes |
|----|-------|------|---------|---------|--------|----------|-------|
| WI-001 | Establish scope and auth | lead | case | process | done | E-001 | Local original samples preserved |
| WI-002 | Export resources, vocabularies and portable graphs | cre | model | binary | done | E-002, E-003 | Static extraction; recorded native conversion probes |
| WI-003 | Build Windows and Linux native runtime | cae | native source | source | done | E-004, E-005, E-006 | Other targets explicitly unverified |
| WI-004 | Measure original/reference/native differences | cre | corpus | binary | done | E-007, E-008, E-009 | Strict failures retained |
| WI-005 | Recover full original inference semantics | cre | pipeline | source | in_progress | E-007, E-008 | Not completed; no substitute models selected |
| WI-006 | Deliver evidence and reproducible build documentation | doc | report | process | done | E-001, E-002, E-003, E-004, E-005, E-006, E-007, E-008, E-009 | Goal remains incomplete |
| WI-007 | Restore strict default-pipeline parity on Windows and Linux | cre, cae | native source and original samples | source | done | E-010, E-011, E-012, E-013 | Each platform 64 strict recognition cases plus 1 original-DLL size error; thresholds unchanged; full target validation remains WI-005 |

## Coverage
- [ ] Recon/analysis complete for in_scope assets
- [x] Critical/High candidates triaged (N/A for pure RE)
- [x] Validated findings have Evidence (E-*)
- [x] Path documented (callflow)
- [x] Timeline continuous across major phases
- [x] Phase report via docs-generator; full objective not complete
- [x] field-journal anonymized

## Refs
- skills/ops/timeline-workitem.md
- skills/ops/evidence-finding-path.md
