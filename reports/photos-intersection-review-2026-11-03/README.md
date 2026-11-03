# C++ 原生封存结果独立复核

按当前原生验收铁律重新比较 Windows/Linux 的保存输出，**442/442 通过**；这是 `saved-native-results-verification`，`fresh_inference=false`，不冒充又一次全新推理。原始新推理与测试见[主报告](../photos-intersection-p2-2026-11-03/README.md)。

[verification.json](verification.json)绑定原始报告索引 SHA-256 `a051437f6a0411efaffcb3f4a0d19ad9ee521c3922a4b9f2e623aa0973719f59`。验证内容包括封存文件、当前输入/参数/黄金输出身份、原始四资产、155 个模型资源、原生实现提交对应的 bin/lib，以及每项实际结果的严格字段比较。索引共 906 项，来源与阈值均保留原值。

```powershell
python scripts/verify-native-results.py --report reports/photos-intersection-p2-2026-11-03 --output work/fresh-native-review
```

该入口只将两套原生结果纳入识别通过判定，`wasm_is_acceptance_gate=false`。Wasm 数值差异不会阻塞原生；已归档文件的完整性仍会核验。新增 CLI 负例已证明原生坐标超差和成功/错误文件冲突会被拒绝，而 Wasm 诊断差异不会改变 C++ 通过判定。

原生全新执行各 221 项、CTest 各 26/26，Python 完整套件 65/65。Photos 标量断言包含每端 13 个负例及三个边界正例；默认程序在 Review 的 1e9 错误场景中也已正确退出 1。

Linux ARM64、macOS x64/arm64、Android arm64-v8a 原生构建/运行仍未验证。Wasm 本轮仅对既有资产在 Windows Node 24 抽查两项，不是新构建或完整集验收。整体目标继续 incomplete。

最终本地提交后的离线 bundle、源码 ZIP 与实际 HEAD/摘要见生成的 `artifacts/OFFLINE_PACKAGE.json`。这些是忽略的提交后产物，避免包包含自身提交的循环；没有 push、发布或上传。
