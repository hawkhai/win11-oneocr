# 严格比较器换机字节身份补充校验

[Photos 交集 P2 主报告](../photos-intersection-p2-2026-11-03/README.md)及其封存索引保持不变：两套原生全新回归各 221/221、CTest 各 26/26、Python 完整套件 65/65 已通过。

首次从交付 bundle 的干净 Windows 检出运行新的原生保存结果复核时，Git 的自动 CRLF 转换改变了 `scripts/compare-results.py` 的原始字节摘要；身份保护按预期拒绝执行，并报 `Comparator changed`。该检出的原生交付校验仍通过 252 个资产及 106 行书页，属于源码传输身份问题，不是 OCR 结果或阈值变化。

修复在 `.gitattributes` 将严格比较器明确设为 `-text`，与已封存且绑定原始摘要的研究工具采用同类保护。没有修改比较器源码、更新或绕过既有摘要，也没有放宽任何数值阈值。

[git-filters.json](git-filters.json)记录 Git blob 经 `core.autocrlf=true` 和 `false` 两种实际工作区过滤后的摘要，均保持 `06d137cc93680f766031d783da9526064808fea1135124348e180bb8607f4869`，与原生执行绑定摘要一致。可复现：

```sh
git check-attr text -- scripts/compare-results.py
git -c core.autocrlf=true cat-file --filters HEAD:scripts/compare-results.py
git -c core.autocrlf=false cat-file --filters HEAD:scripts/compare-results.py
```

后两条命令输出比较器字节，可用标准 SHA-256 工具核对；第一条应显示 `text: unset`。最终提交后重新生成 bundle 和源码 ZIP，再从新 bundle 的独立目录做 Windows/Linux 交付与原生保存结果复核。精确最终 HEAD、包摘要和提交后检出证明记录在忽略的 `artifacts/OFFLINE_PACKAGE.json`，避免自引用提交。

本次仅补充字节身份保护，不改变主报告、原生程序、模型或 Wasm 资产；Wasm 仍为诊断，未验证原生平台仍保持未验证，整体 incomplete。
