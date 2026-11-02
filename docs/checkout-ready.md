# 会话资产与换机运行

本地 Git 版本同时保留源码、运行资产与验证资料。正式识别仅执行原生程序和本地模型，不运行 Python，不调用原微软 OCR DLL，也不需要账号、证书、激活服务或网络。源码构建和研究工具仍可以按固定版本重新获取。

## 直接运行

Windows 10/11 x64（系统内置 UCRT）：

```powershell
.\ocr.cmd .\ocr-book.jpg --text
.\ocr.cmd '.\samples\oneocr-capabilities\script-cjk.png' --output result.json --quiet
```

Linux x64，glibc 2.43 或更新（实测 Ubuntu 26.04）：

```sh
./ocr.sh ./ocr-book.jpg --text
```

路径中可以包含空格和 Unicode；加引号即可。入口始终使用仓库中的统一 `models/`，不依赖当前工作目录。Windows 的 VC 14.44 release CRT 与 Linux 的 GCC 运行库已随程序保存，原始文件保持不变。Linux 的系统 glibc 不随包替换，旧系统应按 [源码构建说明](portable.md) 在目标系统构建。

现有预编译文件只覆盖 Windows x64 和 Linux x64。macOS、Android、ARM64 的源码配置和示例已提交，但不能把它们视作拉取后已可直接执行的平台。

干净 Git 克隆的最终结果见 [换机验收](../reports/checkout-ready-2026-11-02/README.md)：两端各通过 110 张样本、原 27 项及 105 行书页，未使用原机器构建目录或研究环境。

本次没有 push。如通过离线 Git 包转移，在新电脑执行：

```sh
git clone -b research win11-oneocr-offline.bundle oneocr
cd oneocr
```

然后使用上述 Windows 或 Linux 入口。Git 包是包含这些提交和完整历史的可再生成交付物，不嵌套提交到仓库本身。

## 提交了哪些资产

| 路径 | 内容 |
| --- | --- |
| `native/`、`cmake/`、`scripts/`、`research/*contracts.json` | 引擎源码、ABI/JNI、依赖获取和构建脚本、固定原生行为合约 |
| `models/` | 当前版本全部准备资源：图、原始序列化资源、字表与映射、校准及模型清单；保留 SHA-256 与微软来源 |
| `artifacts/win32-x64/`、`artifacts/linux-x64/` | 已验证的原生程序、动态库、系统编译器运行库、开发头文件/导入库、第三方声明与构建指纹 |
| `samples/` | 全部 110 张输入、缩略图、参考图、来源与许可 |
| `reports/` | 各轮报告；全目录逐图冻结 DLL 基准、严格比较结果、耗时和内存 |
| `research/case/` | 会话 scope、workitems、timeline、field journal、证据目录、原 27 项回归图片与基准、书页基准及早期结论 |
| `research/delivery-manifest.json` | 所有模型/运行文件的逐文件长度与 SHA-256，平台条件和统一模型入口 |

两个平台安装目录里的模型副本不重复加入 Git，使用根目录模型。原仓库中已经跟踪的 `bin/` 仍保留，用于旧接口与离线对照。

不纳入运行交付的目录是构建缓存、Python 环境、可按固定提交下载的第三方工具/源码、重新提取的重复模型与实验中间张量。它们仍在本机 `work/` 等忽略目录中。运行、复跑固定回归及继续阅读研究证据所需文件均已在上述路径保存；需要重新观察内部张量时，用已提交的脚本和固定原 DLL 再生成。研究记录中原来的 `work/oneocr-portable/` 相对引用对应现在的 `research/case/` 快照，历史机器绝对路径仅为测量记录。

## 校验和复跑

可选 Python 3.9+，仅使用标准库，不需要 pip：

```powershell
python scripts/verify-delivery.py
python scripts/verify-delivery.py --all-samples --output work/replay-checkout
```

Linux 使用 `python3`。校验全部模型和运行文件 SHA-256，并从无关工作目录运行 105 行书页，与冻结 DLL 输出按原阈值比较；`--all-samples` 再检查全部 110 张图。验收仍是文字、顺序、词切分及手写类别完全一致，坐标 1 px、角度 0.1 度、置信度 `1e-3`。

这是原生 CLI 的拉取即用流程。使用 DSH JavaScript 插件时，宿主仍需 Node.js 20+，按已有 `package-lock.json` 安装插件依赖；`node_modules` 不加入 Git。

三个原接口拒绝用例单独统计，不冒充识别成功；详情见 [全量报告](../reports/oneocr-all-samples-2026-11-02/README.md)。模型损坏或文件缺失会明确报错，不会自动下载或换模型。

## 资产权利与交付边界

模型不是 MIT。此次把准备资源加入用户授权的本地 Git 交付，不增加微软模型的再分发权，也没有 push、上传或公开发布模型。项目代码、来源图片及各运行库各自保留许可，见 [第三方声明](../THIRD_PARTY_NOTICES.md)。正式执行依赖的 VC release CRT 来源见 [微软官方列表](https://learn.microsoft.com/en-us/visualstudio/releases/2022/redistribution)；其权利不由项目 MIT 许可授予。
