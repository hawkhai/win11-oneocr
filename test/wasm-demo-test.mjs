// Real browser UI smoke test; playwright-core@1.55.0 is a research-only tool.
import assert from 'node:assert/strict';
import { createServer } from 'node:http';
import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { resolve, extname, sep } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const [executablePath, outputDirectory, playwrightModule = 'work/wasm-browser/node_modules/playwright-core/index.mjs', wasmDirectory = 'artifacts/wasm32'] = process.argv.slice(2);
if (!executablePath || !outputDirectory) throw new Error('Usage: node test/wasm-demo-test.mjs chromium-executable output-directory [playwright-module]');
const { chromium } = await import(pathToFileURL(resolve(playwrightModule)).href);
const root = resolve(fileURLToPath(new URL('../', import.meta.url)));
const mime = { '.html': 'text/html', '.mjs': 'text/javascript', '.wasm': 'application/wasm', '.json': 'application/json' };
const server = createServer(async (request, response) => {
  try {
    const pathname = decodeURIComponent(new URL(request.url, 'http://localhost').pathname);
    const deliveryRequest = pathname.startsWith('/artifacts/wasm32/');
    const base = deliveryRequest ? resolve(wasmDirectory) : root;
    let path = resolve(base, deliveryRequest ? pathname.slice('/artifacts/wasm32/'.length) : '.' + pathname);
    if (!path.startsWith(base + sep)) throw new Error('Invalid path');
    if (path.endsWith(sep + 'wasm')) path += sep + 'index.html';
    const data = await readFile(path);
    response.setHeader('Content-Type', mime[extname(path)] ?? 'application/octet-stream');
    response.end(data);
  } catch { response.statusCode = 404; response.end('Not found'); }
});
await new Promise(done => server.listen(0, '127.0.0.1', done));
const origin = `http://127.0.0.1:${server.address().port}`;
let browser;
try {
  browser = await chromium.launch({ executablePath, headless: true, args: ['--disable-background-networking'] });
  const context = await browser.newContext({ serviceWorkers: 'block' });
  await context.route('**/*', route => new URL(route.request().url()).origin === origin ? route.continue() : route.abort());
  const page = await context.newPage();
  await page.goto(origin + '/examples/wasm/index.html');
  assert.equal(await page.locator('#run').isDisabled(), true);
  await page.locator('#image').setInputFiles(resolve(root, 'research/case/corpus/latin-36.png'));
  await page.locator('#run').click();
  await page.locator('#cancel').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent === '已取消。');
  await page.locator('#run').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent.startsWith('完成：'), undefined, { timeout: 120000 });
  const result = JSON.parse(await page.locator('#json').textContent());
  assert.equal(result.line_count, 2);
  assert.match(await page.locator('#text').textContent(), /OneOCR native source 2026/);
  assert.equal(await page.locator('#cancel').isDisabled(), true);
  const output = resolve(outputDirectory);
  await mkdir(output, { recursive: true });
  await writeFile(resolve(output, 'result.json'), JSON.stringify(result, null, 2) + '\n');

  // Validate options before loading, then exercise soft batching on real bytes.
  await page.locator('#max-lines').fill('1001');
  await page.locator('#run').click();
  assert.equal(await page.locator('#max-lines').evaluate(input => input.validity.valid), false);
  assert.equal(await page.locator('#cancel').isDisabled(), true);
  await page.locator('#max-lines').fill('1');
  await page.locator('#resize-width').fill('640');
  await page.locator('#run').click();
  assert.equal(await page.locator('#resize-width').evaluate(input => input.validity.valid), false);
  await page.locator('#resize-width').fill('0');
  await page.locator('#image').setInputFiles(resolve(root, 'samples/options-handwriting/twelve-printed-lines.png'));
  await page.locator('#run').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent.startsWith('完成：'), undefined, { timeout: 120000 });
  const softBudget = JSON.parse(await page.locator('#json').textContent());
  assert.equal(softBudget.line_count, 4);
  await writeFile(resolve(output, 'soft-budget.json'), JSON.stringify(softBudget, null, 2) + '\n');
  await page.locator('#resize-width').fill('640');
  await page.locator('#resize-height').fill('384');
  await page.locator('#run').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent.startsWith('完成：'), undefined, { timeout: 120000 });
  const resized = JSON.parse(await page.locator('#json').textContent());
  await writeFile(resolve(output, 'resized.json'), JSON.stringify(resized, null, 2) + '\n');
  const baseline = JSON.parse(await readFile(resolve(root, 'reports/parameter-expansion-2026-11-04/reference/parameters/twelve-printed-lines-m1-r640x384.json'), 'utf8'));
  assert.deepEqual(resized.lines.map(line => line.text), baseline.lines.map(line => line.text));

  // Cancellation must still work when reusing an already initialized worker.
  await page.locator('#run').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent === '正在识别…');
  await page.locator('#cancel').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent === '已取消。');
  await page.locator('#image').setInputFiles({ name: 'broken.png', mimeType: 'image/png', buffer: Buffer.from([1, 2, 3]) });
  await page.locator('#run').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent.startsWith('失败 (-13)'), undefined, { timeout: 120000 });
  assert.equal(await page.locator('#json').textContent(), '');
  await page.locator('#image').setInputFiles(resolve(root, 'research/case/corpus/latin-36.png'));
  await page.locator('#max-lines').fill('0');
  await page.locator('#resize-width').fill('0');
  await page.locator('#resize-height').fill('0');
  await page.locator('#run').click();
  await page.waitForFunction(() => document.querySelector('#status').textContent.startsWith('完成：'), undefined, { timeout: 120000 });
  assert.deepEqual(JSON.parse(await page.locator('#json').textContent()), result);

  const contracts = await page.evaluate(async () => {
    const { createOneOcrWorker } = await import('/artifacts/wasm32/worker-client.mjs');
    async function rejection(create) {
      try { const engine = await create(); await engine.close(); throw new Error('Unexpected success'); }
      catch (error) { return { message: error.message, code: error.code }; }
    }
    const config = { modelBaseUrl: new URL('/models/', location.href), timeoutMs: 120000 };
    const timeout = await rejection(() => createOneOcrWorker({ ...config, timeoutMs: 1 }));
    const checksum = await rejection(() => createOneOcrWorker({ ...config, manifestSha256: '0'.repeat(64) }));
    const missing = await rejection(() => createOneOcrWorker({ ...config, modelBaseUrl: new URL('/missing-models/', location.href) }));
    const worker = await createOneOcrWorker(config);
    const width = 64, height = 64, stride = width * 4 + 16;
    try {
      const data = new Uint8Array(stride * height).fill(255);
      const blanks = await Promise.all([worker.recognizeBgra(data, { width, height, stride }), worker.recognizeBgra(data, { width, height, stride })]);
      const invalid = await rejection(() => worker.recognizeBgra(new Uint8Array(4), { width, height, stride }));
      await worker.close();
      const closed = await rejection(() => worker.recognizeBgra(data, { width, height, stride }));
      return { timeout, checksum, missing, blanks, invalid, closed };
    } finally { await worker.close(); }
  });
  assert.match(contracts.timeout.message, /timed out/);
  assert.equal(contracts.checksum.code, -30);
  assert.equal(contracts.missing.code, -30);
  assert.deepEqual(contracts.blanks[0], contracts.blanks[1]);
  assert.equal(contracts.blanks[0].line_count, 0);
  assert.equal(contracts.blanks[0].image.step, 272);
  assert.equal(contracts.invalid.code, -12);
  assert.match(contracts.closed.message, /closed/);
  await writeFile(resolve(output, 'worker-contracts.json'), JSON.stringify(contracts, null, 2) + '\n');
  await page.screenshot({ path: resolve(output, 'page.png'), fullPage: true });
  await writeFile(resolve(output, 'summary.json'), JSON.stringify({ passed: true, browser: browser.version(), cancellation: 'passed during initialization and reused inference', reloadAfterCancel: 'passed', errorRecovery: 'passed', options: 'soft budget and resize passed; invalid values rejected', workerContracts: 'BGRA padding, queue, close, timeout, checksum and HTTP failure passed', lineCount: result.line_count }, null, 2) + '\n');
  console.log('Browser UI and Worker contracts: PASS');
} finally { await browser?.close(); await new Promise(done => server.close(done)); }
