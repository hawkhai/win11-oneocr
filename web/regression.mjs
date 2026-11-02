// Batch runner for scripts/run-wasm-parity.py. Real model inference only.
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';
import { performance } from 'node:perf_hooks';
import { createOneOcr } from './oneocr.mjs';

const [inventoryFile, outputDirectory, modelDirectory] = process.argv.slice(2);
if (!inventoryFile || !outputDirectory || !modelDirectory) throw new Error('Expected cases.json output models');
const cases = JSON.parse(await readFile(inventoryFile, 'utf8'));
const started = performance.now();
const engine = await createOneOcr({
  modelBaseUrl: pathToFileURL(resolve(modelDirectory) + '/'),
  runtimeUrl: new URL('../artifacts/wasm32/oneocr-core.mjs', import.meta.url),
});
const initializationMs = performance.now() - started;
await mkdir(outputDirectory, { recursive: true });
const measurements = [];
try {
  for (const entry of cases) {
    const data = await readFile(entry.path);
    if (createHash('sha256').update(data).digest('hex') !== entry.sha256) throw new Error(`Input hash changed: ${entry.path}`);
    const began = performance.now();
    let status = 'success', code = 0;
    try {
      const result = engine.recognize(data, { fileName: entry.path });
      await writeFile(resolve(outputDirectory, entry.name + '.json'), JSON.stringify(result, null, 2) + '\n');
    } catch (error) {
      status = 'error'; code = error.code ?? null;
      await writeFile(resolve(outputDirectory, entry.name + '.error.json'), JSON.stringify({ code, message: error.message, stage: error.stage }, null, 2) + '\n');
    }
    measurements.push({ name: entry.name, status, code, milliseconds: performance.now() - began, rssBytes: process.memoryUsage().rss });
    await writeFile(resolve(outputDirectory, 'measurements.json'), JSON.stringify({ node: process.version, v8: process.versions.v8,
      initializationMs, peakRssBytes: process.resourceUsage().maxRSS * 1024, cases: measurements }, null, 2) + '\n');
    console.log(entry.name, status, code, Math.round(measurements.at(-1).milliseconds));
  }
} finally { engine.close(); }
