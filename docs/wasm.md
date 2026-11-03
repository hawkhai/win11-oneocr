# JavaScript + WebAssembly

此后端将同一份 C++20 引擎、OpenCV 4.10.0、ICU 74.2 与 ORT 1.23.2 静态链接进 WebAssembly。JS 只负责文件加载、C ABI 内存管理和 Worker 通信；预处理、识别、词切分、阅读顺序、坐标、置信度、手写及方向判断均在 Wasm 中执行。模型使用仓库现有 `models/manifest.json`，没有换模型。

本轮 Windows Node 24.14.0 和 Chrome 156 均通过全量 **138/138**，两端原始结果/错误 JSON 逐字节一致，见[实测报告](../reports/wasm-2026-11-03/README.md)。相对原 DLL 使用坐标 1 像素、角度 0.1 度、置信度 `1e-3` 的原有阈值；文字、顺序、词切分和手写类型要求一致。其他浏览器及设备须单独验证。

后续 [Review 修复与增强回归](../reports/review-fixes-2026-11-03/README.md)补齐了计数和索引校验，并以全新推理验证 Windows Node 24.14.0、Linux Node 24.14.0、Chrome 156，各 **138/138**。Linux Node 完整集在无网络命名空间运行；除主机输入路径外，三端识别字段逐值一致。Node 20 仍只有历史书页和部分输入记录，不标记为完整集通过。

## 浏览器接入

把 `artifacts/wasm32/` 和独立的 `models/` 部署到同一个静态站点。须通过 HTTP(S) 加载模块，不能直接双击 `file://` HTML；`.mjs` 使用 JavaScript MIME，`.wasm` 使用 `application/wasm`。运行时只访问指定的静态文件，不需要账号、授权服务或原微软 DLL。

可直接打开仓库内的浏览器示例：在仓库根目录执行 `python -m http.server 8000 --bind 127.0.0.1`，访问 `http://127.0.0.1:8000/examples/wasm/`。Python 仅作为示例的静态文件服务器，识别在浏览器 Worker 中执行；也可换用任意静态服务器。

推荐 Worker，避免同步推理阻塞页面：

```js
import { createOneOcrWorker } from '/artifacts/wasm32/worker-client.mjs';

const controller = new AbortController();
const engine = await createOneOcrWorker({
  modelBaseUrl: new URL('/models/', location.href), // 末尾必须有 /
  timeoutMs: 600000, // 单次请求；包含首次模型加载
  signal: controller.signal,
});
try {
  const file = document.querySelector('input[type=file]').files[0];
  const result = await engine.recognize(await file.arrayBuffer(), {
    fileName: file.name,
    maxLines: 1000,
    resizeWidth: 1152,
    resizeHeight: 768,
  });
  console.log(result);
} finally {
  await engine.close();
}
// controller.abort() 或 engine.terminate() 可中止正在执行的任务并销毁 Worker。
// 取消/超时后须重新创建实例。close() 等待已提交任务结束。
```

也可在已有 Worker 内直接调用 `createOneOcr`，`recognize()` 同步返回 JSON 对象：

```js
import { createOneOcr } from '/artifacts/wasm32/oneocr.mjs';
const engine = await createOneOcr({ modelBaseUrl: new URL('/models/', location.href) });
try {
  const response = await fetch('/input.png');
  if (!response.ok) throw new Error(`Input HTTP ${response.status}`);
  console.log(engine.recognize(await response.arrayBuffer(), { fileName: 'input.png' }));
} finally { engine.close(); }
```

输入采用原始编码字节，并由与桌面版相同的解码器处理。不要为回归方便先经过 Canvas 解码：浏览器色彩管理、Alpha 合成或像素转换会改变输入，双通道图片也可能被自动转换，无法验证原接口拒绝行为。原来的三个拒绝样本保留原图及错误契约。

## Node.js 接入

Node 20 或更新版本，ESM：

```js
import { readFile } from 'node:fs/promises';
import { createOneOcr } from './artifacts/wasm32/oneocr.mjs';

const engine = await createOneOcr({ modelBaseUrl: new URL('./models/', import.meta.url) });
try {
  const result = engine.recognize(await readFile('test.png'), { fileName: 'test.png' });
  console.log(result.lines.map(line => line.text).join('\n'));
} finally { engine.close(); }
```

npm 包入口为 `dsh-win11-oneocr/wasm`、`dsh-win11-oneocr/wasm/worker`，模型由接入方单独提供。仓库命令行示例：

```powershell
node web/cli.mjs test.png --model-dir models --output work/wasm-test.json --quiet
```

CLI 保留 `--text/-t`、`--output/-o`、`--max-lines`、`--resize WxH`、`--quiet/-q`；成功退出 0，失败退出 1 并写 stderr。实例可复用；一个 Worker 内按提交顺序串行执行，独立 Worker 的模型及 Wasm 内存互不共享。

DSH 插件配置 `{ backend: 'wasm', timeoutMs: 600000 }` 后使用当前 Node 启动交付目录内的 Wasm CLI，`ocrBin` 可指定其他 Wasm CLI 文件，`modelDir`、取消和输出大小限制与原插件一致。默认仍为 `portable`。也可直接执行 `node artifacts/wasm32/cli.mjs ...`，无需仓库的 `web/` 开发目录。

精确浮点适配的速度明显慢于桌面原生后端，长文档在本次测试中需要数分钟。Worker 默认超时仍为 120 秒，DSH 插件默认仍为 60 秒；接入长文档时应显式设置超时。详细耗时在回归报告的 `measurements.json` 中，多个并行实例会争用 CPU，这些数据不是独占 CPU 的性能基准。

## BGRA 与错误处理

`engine.recognizeBgra(bytes, { width, height, stride, maxLines, resizeWidth, resizeHeight })` 直接调用原生带缓冲区长度的 BGRA 接口，默认 `stride=width*4`，支持行尾填充。通道顺序是 BGRA，Alpha 被忽略；Canvas/Android 常见 RGBA 须显式转换。原始缓冲区结果的 `file` 为空字符串，`image.step` 保留传入步长。

`OneOcrError` 提供 `code` 和 `stage`。原生的 `-12/-13/-15/-18/-19/-30/-31` 与 [错误码文档](error-codes.md) 相同，错误文本保留用于诊断。JS 加载器、HTTP、模块导入及 Worker 错误也可能是普通 `Error`；取消/超时不会伪造原生状态码。关闭的直接实例返回 `OneOcrError(-12)`，关闭的 Worker 拒绝后续请求。所有 C ABI 返回指针在 JS 层释放，调用方无需手动释放 JSON。

`createOneOcr` 支持 `loadAsset(relativePath)` 自定义资源读取、`onProgress({file, completed, total})` 加载进度及可选的 `manifestSha256`。引擎初始化会验证清单记录的模型、字表和元数据摘要；有可信的清单摘要时应传入 `manifestSha256`。清单校验不是模型来源权利声明，模型仍归微软所有。

`runtimeUrl` 可传 URL 对象、绝对 URL 字符串，或相对 `oneocr.mjs` 的路径；`wasmUrl` 相对规范化后的运行模块 URL 解析。Node 的 `workerUrl` 同时接受 `file:` URL 对象及等价字符串，仍支持原来的文件系统路径。自定义 Worker 需采用同一消息协议。

## 从源码构建

### 数值适配

Wasm 保留同一 OCR 流程和模型，对底层数值差异补充源码适配：

- 固定 OpenCV 4.10.0 的 Wasm `v_round` 原来加 `0.5` 后截断，改为 SSE 对应的就近取偶舍入，并保留越界转换哨兵。小数半点、负数、溢出及 SIMD/尾部转换有专门实测。
- 固定 DLL 基准机的浮点卷积采用 AVX-512 NCHWc 运算顺序。Wasm 在 NCHW 存储中重现首层逐通道、普通卷积每 16 通道、点卷积每 128 通道的累加边界，并提供精确 float32 FMA。SIMD 快路径使用 float64 精确乘积；可能出现双重舍入的半点、非正规数或异常范围退回标量 `fmaf`。不依赖 relaxed SIMD 的可变融合行为。
- LSTM 的 sigmoid/tanh 多项式保留上游系数，按原生 FMA3 路径使用融合乘加；避免分步乘加的微小差异在循环状态中累积。

实现位于 `native/wasm/`；构建脚本核对固定上游源码，拒绝覆盖未知修改，并在 `BUILD.json` 记录调整前后的摘要。该数值配置针对当前冻结的 DLL/模型回归基准；不声称所有微软 DLL 版本或任意硬件原生后端具有相同内部算术。原图、模型权重及严格阈值均不改变。

### 构建命令

构建宿主为 Linux/macOS，Windows 使用 WSL。需 Python 3、Git、CMake 3.28+、Ninja、主机 C/C++ 编译器和 make。ORT 会在固定子模块中安装 **Emscripten 4.0.8**，不使用浮动的 `latest`；锁定信息见 `research/dependencies.json`。

```bash
python3 scripts/build-wasm.py --jobs 6
```

脚本获取固定 ORT/OpenCV/ICU 源码，准备 ICU 数据，将 ORT 构建为 `libonnxruntime_webassembly.a`，再编译同一份引擎。已准备源时可指定 `--ort-source`、`--opencv-source`、`--icu-source`；`--build-dir` 可放到 WSL 的 ext4 文件系统提高构建速度。`--dependency-cache` 仅用于复用同一锁定 ORT 构建的 FetchContent 源码缓存；不要传入其他版本。

输出在 `artifacts/wasm32/`：`oneocr-core.mjs`、`oneocr-core.wasm`、JS/Worker 封装、声明及带摘要的 `BUILD.json`。模型不嵌入 Wasm，也不从其他服务下载；`models/` 与桌面平台共用。Emscripten CMake preset `wasm32` 可用于已准备静态 ORT SDK 的独立构建。

当前配置为单线程、SIMD128、无 relaxed SIMD，保留 C++ 异常，禁用隐式浮点收缩；不依赖 SharedArrayBuffer 或 COOP/COEP。浏览器/Node 需要支持 WebAssembly SIMD 和 BigInt。线性内存可增长至 4 GiB；这是地址空间上限，不是每台设备都能分配的内存保证。移动设备及未实测浏览器不得视为已验证。

上游说明：[ORT Wasm 静态库构建](https://onnxruntime.ai/docs/build/web)、[Emscripten 异常](https://emscripten.org/docs/porting/exceptions.html)、[SIMD](https://emscripten.org/docs/porting/simd)。许可与第三方声明在交付目录 `share/oneocr/` 中，模型不适用项目 MIT 许可。

## 运行严格回归

```powershell
node test/wasm-test.mjs
.research-python/python.exe -X utf8 scripts/run-wasm-parity.py --output work/wasm-parity-fresh
```

默认以 4 个独立实例运行所有样本、固定 27 项及书页，不遗漏三个拒绝用例；每项保留原始 JSON/错误、摘要、测量及严格比较报告。内存较少时可用 `--jobs 1`。输出目录须为空，避免残留结果掩盖失败。`--only book` 等选项只做诊断，不标记全量通过。

比较器还检查 `line_count`、`word_count` 与数组长度一致，以及行/词 `index` 的整数类型和连续顺序；缺失字段、错误类型和非有限数值不能通过。主机文件路径与后端专属 `engine` 元数据不要求相等。坐标、角度和置信度阈值保持原值。

数值误差比较前，基准和结果的角度均须位于 `[0,360)`，词及手写/印刷置信度须位于 `[0,1]`；不会以角度取模或误差容限接受越界值。

复核已保存结果必须使用独立的新目录：

```powershell
python scripts/run-wasm-parity.py --compare-only --output reports/review-fixes-2026-11-03/linux-node --verification-output work/linux-record-verification
```

此模式不执行推理，也不修改原来的清单、路径、调度、耗时、构建或结果；只输出新的 `verification.json` 和比较记录。不能传入 `--jobs`、`--browser` 等推理选项来改写历史。复核会检查执行文件摘要、输入和基准绑定，以及当前运行文件和模型是否仍与原执行一致；发生变化时须重新推理。新执行自动保存 `execution.json`，历史报告须保留完整的 `FILES.sha256.json` 索引。该模式不验证尚未编译的源码改动，不能用旧结果声称新引擎已通过。

边界修复的测试与五端历史记录复核见[独立报告](../reports/review-boundaries-2026-11-03/README.md)，与上一轮全新推理分开记录。

`execution.json` 中的构建、用例、基准摘要、调度和浏览器等待预算必须与其封存的原始文件逐项一致，实际复核使用原始文件中的值。有外层报告索引时，还会核验 `execution.json` 本身。单环境与五环境复核共用结果清单校验：成功项旁的 `.error.json`、拒绝项旁的成功结果、缺失或未登记的结果 JSON 均会使复核失败，不发布通过报告。相关修复与负例测试见[执行证据绑定报告](../reports/evidence-binding-2026-11-03/README.md)。

可用 `--schedule-from` 指定同模型、同输入摘要的旧完整报告，仅利用耗时安排并行任务，不复用其识别结果。浏览器回归的单请求测试等待预算默认 30 分钟，可用 `--browser-timeout-ms` 调整，以容纳多进程 CPU 争用；这不会修改 Worker/插件的产品默认超时或任何数值验收阈值。Windows 浏览器回归在关闭前记录当前浏览器进程树的 OS 峰值工作集，各进程峰值之和只作为上界，不代表同时占用峰值。

浏览器全量回归可使用本地 Chromium 和固定的 Playwright Core 1.55.0：

```powershell
npm install --prefix work/wasm-browser --no-package-lock --no-save playwright-core@1.55.0
.research-python/python.exe -X utf8 scripts/run-wasm-parity.py --browser "C:/Program Files/Google/Chrome/Application/chrome.exe" --output work/wasm-browser-fresh
```

该流程启动仅监听 `127.0.0.1` 的临时静态服务器，并阻止测试上下文向其他 origin 请求；图像和模型字节保持原样。在不同浏览器、CPU 或 JS 引擎版本上，需要分别执行回归才能报告运行及一致性通过。
