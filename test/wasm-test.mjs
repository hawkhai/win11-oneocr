// Integration tests require the real compiled wasm and repository models.
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { createOneOcr, OneOcrError, modelAssetPaths } from '../web/oneocr.mjs';
import { createOneOcrWorker } from '../web/worker-client.mjs';
import { fileURLToPath } from 'node:url';
import { apply } from '../src/index.js';

const root = new URL('../', import.meta.url);
const configuration = { modelBaseUrl: new URL('models/', root), runtimeUrl: new URL('artifacts/wasm32/oneocr-core.mjs', root) };
const image = new URL('research/case/corpus/latin-36.png', root);
const input = new Uint8Array(await readFile(image));
const unchanged = createHash('sha256').update(input).digest('hex');
assert.throws(() => modelAssetPaths({ schema_version: 1, models: [{ file: '../escape', sha256: 'x' }] }), e => e.code === -12);
await assert.rejects(createOneOcr({ ...configuration, manifestSha256: '0'.repeat(64) }), e => e.code === -30);
const manifest = JSON.parse(await readFile(new URL('models/manifest.json', root), 'utf8'));
await assert.rejects(createOneOcr({ ...configuration, loadAsset: async path => {
  const data = new Uint8Array(await readFile(new URL('models/' + path, root)));
  if (path === manifest.models[0].file) data[0] ^= 1;
  return data;
} }), e => e instanceof OneOcrError && e.code === -30 && /checksum mismatch/iu.test(e.message));
const engine = await createOneOcr(configuration);
let first;
try {
  first = engine.recognize(input, { fileName: '中文路径-😀.png' });
  assert.equal(first.file, '中文路径-😀.png');
  assert.ok(first.line_count > 0);
  assert.deepEqual(engine.recognize(input, { fileName: first.file }), first);
  assert.equal(createHash('sha256').update(input).digest('hex'), unchanged);
  assert.throws(() => engine.recognize(input, { maxLines: 1001 }), e => e instanceof OneOcrError && e.code === -12);
  assert.throws(() => engine.recognize(new Uint8Array([1, 2, 3])), e => e.code === -13);
  assert.throws(() => engine.recognizeBgra(new Uint8Array(4), { width: 50, height: 50, stride: 200 }), e => e.code === -12);
  assert.throws(() => engine.recognizeBgra(new Uint8Array(50 * 50 * 4), { width: 50, height: 50, stride: -200 }), e => e.code === -12);
  assert.throws(() => engine.recognizeBgra(new Uint8Array(50 * 49 * 4), { width: 50, height: 49 }), e => e.code === -18);
  const width = 64, height = 64, stride = width * 4 + 16;
  const padded = new Uint8Array(stride * height).fill(255);
  const blank = engine.recognizeBgra(padded, { width, height, stride });
  assert.equal(blank.line_count, 0);
  assert.deepEqual(blank.image, { width, height, step: stride });
} finally { engine.close(); engine.close(); }
assert.throws(() => engine.recognize(input), e => e.code === -12);
const relative = await createOneOcr({ ...configuration, runtimeUrl: '../artifacts/wasm32/oneocr-core.mjs', wasmUrl: './oneocr-core.wasm' });
try { assert.deepEqual(relative.recognize(input, { fileName: first.file }), first); }
finally { relative.close(); }
const worker = await createOneOcrWorker({ ...configuration, workerUrl: new URL('../web/worker.mjs', import.meta.url).href, timeoutMs: 120000 });
try {
  const results = await Promise.all([worker.recognize(input, { fileName: first.file }), worker.recognize(input, { fileName: first.file })]);
  assert.deepEqual(results, [first, first]);
} finally { await worker.close(); await worker.close(); }
const controller = new AbortController();
const aborting = createOneOcrWorker({ ...configuration, signal: controller.signal });
controller.abort();
await assert.rejects(aborting, /aborted/);
await assert.rejects(createOneOcrWorker({ ...configuration, timeoutMs: 1 }), /timed out/);
let plugin;
apply({ tools: { register(value) { plugin = value; } } }, { backend: 'wasm', timeoutMs: 120000 });
const pluginResult = await plugin.execute({ image_path: fileURLToPath(image) }, {});
const actualPlugin = JSON.parse(pluginResult.result);
assert.deepEqual(actualPlugin.lines, first.lines);
assert.equal(pluginResult.text, first.lines.map(line => line.text).filter(Boolean).join('\n'));
console.log('Real Wasm API, relative runtime/Wasm URLs, file URL string worker, ownership, BGRA stride, worker ordering, cancellation and timeout: PASS');
