// Real browser UI smoke test; playwright-core@1.55.0 is a research-only tool.
import assert from 'node:assert/strict';
import { createServer } from 'node:http';
import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { resolve, extname, sep } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const [executablePath, outputDirectory, playwrightModule = 'work/wasm-browser/node_modules/playwright-core/index.mjs'] = process.argv.slice(2);
if (!executablePath || !outputDirectory) throw new Error('Usage: node test/wasm-demo-test.mjs chromium-executable output-directory [playwright-module]');
const { chromium } = await import(pathToFileURL(resolve(playwrightModule)).href);
const root = fileURLToPath(new URL('../', import.meta.url));
const mime = { '.html': 'text/html', '.mjs': 'text/javascript', '.wasm': 'application/wasm', '.json': 'application/json' };
const server = createServer(async (request, response) => {
  try {
    let path = resolve(root, '.' + decodeURIComponent(new URL(request.url, 'http://localhost').pathname));
    if (!path.startsWith(root)) throw new Error('Invalid path');
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
  await writeFile(resolve(output, 'summary.json'), JSON.stringify({ passed: true, browser: browser.version(), cancellation: 'passed', reloadAfterCancel: 'passed', lineCount: result.line_count }, null, 2) + '\n');
  console.log('Browser UI: select original bytes, cancel, reload and recognize: PASS');
} finally { await browser?.close(); await new Promise(done => server.close(done)); }
