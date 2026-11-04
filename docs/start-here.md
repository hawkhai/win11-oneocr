# OneOCR 接手入口

2026-11-04。Windows x64 / Linux x64 本轮已完成并收手；当前每端 269 项、合计 538 项按原 DLL 严格通过。坐标 ≤1px、角度 ≤0.1°、置信度 ≤1e-3，其余既有字段一致，拒绝非法值。先读 [AGENTS.md](../AGENTS.md)；不自动扩大范围或平台。

| 接手目的 | 入口 |
|---|---|
| 直接运行 | Windows `ocr.cmd ocr-book.jpg --text`；Linux `./ocr.sh ./ocr-book.jpg --text`，现有 Linux 包要求 glibc ≥2.43 |
| 找到当前离线包 | `artifacts/OFFLINE_PACKAGE.json`，其中 files 和 delivery_status.path 均相对 artifacts；directory 指向本次独立发行目录 |
| 自己编译 | [colleague-build.md](colleague-build.md)，依赖自行准备，普通 SDK 模式无需研究 Python/WSL |
| 使用 Wasm 网页 | [接入与构建](wasm.md)、[网页能力对齐报告](../reports/wasm-web-alignment-2026-11-04/README.md)：Chrome 当前 269 项通过，其他浏览器状态单独保留 |
| 复核已有结果 | [delivery-status.md](delivery-status.md)，保存结果复核不执行新推理 |
| 查看实现、历史与排错 | [详细交接](handoff-2026-11-03.md)、[错误语义](error-codes.md) |

当前源码、已验收程序、最新重编程序分别按 [交付状态](delivery-status.md) 核对；包内原有程序不冒称最新重编产物。

## 本地包核验

使用 Python 3.11+；不要使用本机默认指向旧 Python 的命令别名。以下从仓库根执行，不运行推理：

```powershell
$package = Get-Content -LiteralPath artifacts/OFFLINE_PACKAGE.json -Raw | ConvertFrom-Json
if ($package.commit -ne (git rev-parse HEAD).Trim()) { throw 'Package HEAD is stale' }
foreach ($entry in $package.files.PSObject.Properties) {
    $file = Join-Path artifacts $entry.Name
    if ((Get-Item -LiteralPath $file).Length -ne $entry.Value.size -or
        (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Value.sha256) { throw 'Package changed' }
    if ($entry.Name.EndsWith('.bundle')) { git bundle verify $file }
    if ($entry.Name.EndsWith('.zip')) { python scripts/package-source.py --verify $file --commit $package.commit }
}
$status = Join-Path artifacts $package.delivery_status.path
if ((Get-FileHash -LiteralPath $status -Algorithm SHA256).Hash.ToLowerInvariant() -ne $package.delivery_status.sha256) { throw 'Delivery status changed' }
```

版本化发行包位于 `artifacts/releases/<上海时间>-<提交号>/`，旧固定名称包保留为历史。打包失败不前移当前索引；源码 ZIP 从指定提交读取，按允许的源码目录和根文件排除模型／编译产物，保留所有已提交扩展名和报告索引要求的文件。文件内容及执行位可与提交再次核对。

源码、模型、运行库或登记范围变化时仍须按既有规则重新完整验收。仅交付工具和文档更新不冒称新推理；本地提交与刷新包不包含 push 或发布。
