#!/usr/bin/env node
import { readFile, writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';
import { createOneOcr } from './oneocr.mjs';

try {
  const images = [], options = {};
  let modelDirectory = new URL('../../models/', import.meta.url), output, quiet = false, text = false;
  const args = process.argv.slice(2);
  const positive = value => {
    if (!/^[1-9]\d*$/u.test(value ?? '') || !Number.isSafeInteger(Number(value))) throw new Error(`Expected a positive integer: ${value}`);
    return Number(value);
  };
  for (let i = 0; i < args.length; i++) {
    const arg = args[i];
    const value = () => { if (++i >= args.length) throw new Error(`Missing value for ${arg}`); return args[i]; };
    if (arg === '--help' || arg === '-h') {
      console.log('OneOCR WebAssembly\nUsage: node web/cli.mjs <image> [images ...] [--model-dir DIR] [--output FILE] [--text] [--max-lines N] [--resize WxH] [--quiet]');
      process.exit(0);
    }
    if (arg === '--capabilities') {
      const factory = (await import('./oneocr-core.mjs')).default;
      const module = await factory();
      console.log(module.UTF8ToString(module._oneocr_capabilities()));
      process.exit(0);
    }
    if (arg === '--quiet' || arg === '-q') quiet = true;
    else if (arg === '--text' || arg === '-t') text = true;
    else if (arg === '--model-dir') modelDirectory = pathToFileURL(resolve(value()) + '/');
    else if (arg === '--output' || arg === '-o') output = value();
    else if (arg === '--max-lines') options.maxLines = positive(value());
    else if (arg === '--resize') {
      const size = value().split('x');
      if (size.length !== 2) throw new Error('Resize must be WxH');
      [options.resizeWidth, options.resizeHeight] = size.map(positive);
    } else if (arg.startsWith('-')) throw new Error(`Unknown option: ${arg}`);
    else images.push(arg);
  }
  if (!images.length) throw new Error('No input images; use --help');
  if (output && images.length !== 1) throw new Error('--output requires exactly one input image');
  const engine = await createOneOcr({ modelBaseUrl: modelDirectory, runtimeUrl: new URL('./oneocr-core.mjs', import.meta.url) });
  try {
    for (const file of images) {
      if (!quiet) console.error(`OCR: ${file}`);
      const result = engine.recognize(await readFile(file), { fileName: file, ...options });
      if (text) for (const line of result.lines) console.log(line.text);
      else await writeFile(output ?? file + '.json', JSON.stringify(result, null, 2) + '\n');
    }
  } finally { engine.close(); }
} catch (error) {
  console.error(`ERROR: ${error.message}`);
  process.exitCode = 1;
}
