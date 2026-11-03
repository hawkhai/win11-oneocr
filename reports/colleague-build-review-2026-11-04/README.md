# 外部依赖重编结果的独立复核

2026-11-04，读取 colleague-build-2026-11-04 封存结果，原有严格比较器再次通过 **538/538**；current_scope=true、fresh_inference=false，不重复计算新推理。源文件、当前全部输入、基准、模型/原资产及报告身份均检查。

隔离副本的书页词框 x 坐标改大 2.1px，并重新封存副本索引后，复核仍拒绝该项：退出 1、537/538，见 negative-proof.json。原始执行、基准和历史索引未改。

复现：`python reports/colleague-build-2026-11-04/evidence/review-build-results.py --report reports/colleague-build-2026-11-04 --output work/fresh-clean-build-review`。新编译程序留在忽略目录，未冒充或覆盖 Git 原有预编译运行包；本轮不提交依赖归档、SDK 或新二进制。
