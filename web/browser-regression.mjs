// Local HTTP + headless Chromium. Models/images never leave the local machine.
import { createServer } from 'node:http';
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { dirname, extname, relative, resolve, sep } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const [inventoryFile, outputDirectory, modelDirectory, executablePath,
  playwrightPath = fileURLToPath(new URL('../work/wasm-browser/node_modules/playwright-core/index.mjs', import.meta.url)), timeoutValue = '1800000'] = process.argv.slice(2);
if (!inventoryFile || !outputDirectory || !modelDirectory || !executablePath) throw new Error('Expected cases.json output models chromium-executable [playwright-module]');
const requestTimeoutMs = Number(timeoutValue);
if (!Number.isSafeInteger(requestTimeoutMs) || requestTimeoutMs < 1) throw new Error('Invalid test request budget');
const { chromium } = await import(pathToFileURL(resolve(playwrightPath)).href);
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const mime = { '.mjs': 'text/javascript', '.js': 'text/javascript', '.wasm': 'application/wasm', '.json': 'application/json' };
const server = createServer(async (request, response) => {
  try {
    const pathname = decodeURIComponent(new URL(request.url, 'http://localhost').pathname);
    if (pathname === '/') { response.setHeader('Content-Type', 'text/html'); response.end('<!doctype html><title>Local OneOCR regression</title>'); return; }
    const path = resolve(root, '.' + pathname);
    if (!path.startsWith(root + sep)) throw new Error('Invalid path');
    const data = await readFile(path);
    response.setHeader('Content-Type', mime[extname(path)] ?? 'application/octet-stream');
    response.end(data);
  } catch { response.statusCode = 404; response.end('Not found'); }
});
await new Promise(resolveListen => server.listen(0, '127.0.0.1', resolveListen));
const origin = `http://127.0.0.1:${server.address().port}`;
let browser;
const measurements = [];
try {
  browser = await chromium.launch({ executablePath, headless: true, args: ['--disable-background-networking', '--disable-component-update'] });
  const context = await browser.newContext({ serviceWorkers: 'block' });
  await context.route('**/*', route => new URL(route.request().url()).origin === origin ? route.continue() : route.abort());
  const page = await context.newPage();
  page.on('console', message => console.error('browser:', message.text()));
  page.on('pageerror', error => console.error('browser error:', error.message));
  await page.goto(origin);
  const relativeModels = relative(root, resolve(modelDirectory));
  if (relativeModels.startsWith('..') || resolve(root, relativeModels) !== resolve(modelDirectory)) throw new Error('Browser test models must be inside repository');
  const initialization = await page.evaluate(async ({ origin, relativeModels, requestTimeoutMs }) => {
    const { createOneOcrWorker } = await import('/artifacts/wasm32/worker-client.mjs');
    const started = performance.now();
    globalThis.ocr = await createOneOcrWorker({ modelBaseUrl: `${origin}/${relativeModels.split('\\').join('/')}/`, timeoutMs: requestTimeoutMs });
    return { initializationMs: performance.now() - started, requestTimeoutMs, userAgent: navigator.userAgent, capabilities: ocr.capabilities };
  }, { origin, relativeModels, requestTimeoutMs });
  const cases = JSON.parse(await readFile(inventoryFile, 'utf8'));
  await mkdir(outputDirectory, { recursive: true });
  for (const entry of cases) {
    const path = resolve(entry.path), data = await readFile(path);
    if (createHash('sha256').update(data).digest('hex') !== entry.sha256) throw new Error(`Input hash changed: ${path}`);
    const imagePath = relative(root, path);
    if (imagePath.startsWith('..')) throw new Error('Browser test image must be inside repository');
    const url = origin + '/' + imagePath.split(sep).map(encodeURIComponent).join('/');
    const actual = await page.evaluate(async ({ url, entry }) => {
      const response = await fetch(url);
      if (!response.ok) throw new Error('Cannot fetch test image');
      const input = new Uint8Array(await response.arrayBuffer());
      const digest = [...new Uint8Array(await crypto.subtle.digest('SHA-256', input))].map(x => x.toString(16).padStart(2, '0')).join('');
      if (digest !== entry.sha256) throw new Error('Browser received changed image bytes');
      const began = performance.now();
      try { return { result: await ocr.recognize(input, { fileName: entry.path, maxLines: entry.options?.max_lines ?? 0, resizeWidth: entry.options?.resize_width ?? 0, resizeHeight: entry.options?.resize_height ?? 0 }), milliseconds: performance.now() - began }; }
      catch (error) { return { error: { code: error.code ?? null, message: error.message, stage: error.stage }, milliseconds: performance.now() - began }; }
    }, { url, entry });
    const suffix = actual.error ? '.error.json' : '.json';
    await writeFile(resolve(outputDirectory, entry.name + suffix), JSON.stringify(actual.error ?? actual.result, null, 2) + '\n');
    measurements.push({ name: entry.name, milliseconds: actual.milliseconds, status: actual.error ? 'error' : 'success', code: actual.error?.code ?? 0 });
    await writeFile(resolve(outputDirectory, 'measurements.json'), JSON.stringify({ browser: browser.version(), ...initialization, cases: measurements }, null, 2) + '\n');
    console.log(entry.name, actual.error ? 'error' : 'success', Math.round(actual.milliseconds));
    if (actual.error && typeof actual.error.code !== 'number') throw new Error(`Regression transport failed: ${actual.error.message}`);
  }
  if (process.platform === 'win32') {
    const { stdout } = await promisify(execFile)('powershell.exe', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
      resolve(root, 'scripts/browser-memory.ps1'), '-ControllerProcessId', String(process.pid)], { windowsHide: true });
    const memory = JSON.parse(stdout.replace(/^\uFEFF/u, ''));
    await writeFile(resolve(outputDirectory, 'browser-memory.json'), JSON.stringify(memory, null, 2) + '\n');
  }
  await page.evaluate(() => ocr.close());
} finally {
  await browser?.close();
  await new Promise(resolveClose => server.close(resolveClose));
}
