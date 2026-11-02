# Local Photos model update scope

- auth.status: granted
- basis: user explicitly requested regression of the local Photos 2026.11090.22001.0 OneModel and the ongoing native-source task
- source: C:/Program Files/WindowsApps/Microsoft.Windows.Photos_2026.11090.22001.0_x64__8wekyb3d8bbwe/
- in_scope: read/hash/copy local oneocr.onemodel, oneocr.dll, onnxruntime.dll; offline isolated wrapper inference; static model parsing; native CPU regression; evidence; authorized local commits
- out_of_scope: modifying WindowsApps originals; uploading models; push; online OCR/authentication services
- network_profile: offline sample analysis/inference
- prior_case: work/oneocr-portable/scope.md
- frozen_old_assets: existing Git history and reports; must not overwrite old golden outputs
- thresholds: text/order/words/style exact; coordinates 1 px; angle 0.1 degree; confidence 1e-3
- triage: evidence/triage.json includes hashes, imports, exports, version strings
