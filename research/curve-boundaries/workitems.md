# Work Items

| ID | 工作项 | 状态 | 证据 |
| --- | --- | --- | --- |
| WI-001 | 固定路由、离线范围、版本与导入导出 | done | scope.md, route-scope.md, assets.json |
| WI-002 | 零主元备用 SVD 与保留 LU 控制 | done | contract.json, oracle-crosscheck.json, static/ |
| WI-003 | 截断目标尾部、三角形像素与词框逆映射 | done | 七组 RGB, 56 个词框, boundary_test.cpp |
| WI-004 | 极长图缩放 AREA 分支 | done | long-detector-input.f32, 报告 diagnostics |
| WI-005 | 114 图片、27 语料和书页，五端严格新推理 | done | 本轮报告 710/710 |
| WI-006 | 测试、输入/证据拒绝负例、离线与克隆验证 | done | 47 Python, 各 9 CTest, Wasm 断言, offline 和 checkout 日志 |

Scope 授权、单代理角色、离线目标均已核对；Ghidra 静态与动态观察有可复现命令，时间线、field-journal、Mermaid 和 docs-generator 普通逆向报告已落盘。整体跨平台目标继续未完成；未验证平台与剩余输入覆盖详见报告。最后执行本地报告 commit，再由已提交脚本生成绑定该 HEAD 的离线包，不 push。
