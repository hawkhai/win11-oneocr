// Opt-in research output, using the existing C++ ONEOCR_TRACE_DIR mechanism.
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';
import { createOneOcr } from './oneocr.mjs';

const [image, output, runtime = 'artifacts/wasm32/oneocr-core.mjs'] = process.argv.slice(2);
if (!image || !output) throw new Error('Usage: node web/trace.mjs image output-directory [runtime-module]');
const factory = (await import(pathToFileURL(resolve(runtime)).href)).default;
let module;
const engine = await createOneOcr({
  modelBaseUrl: new URL('../models/', import.meta.url),
  moduleFactory: async configuration => {
    module = await factory({ ...configuration, preRun: [m => {
      m.ENV.ONEOCR_TRACE_DIR = '/trace';
      m.FS.mkdirTree('/trace');
    }] });
    return module;
  },
});
try {
  const result = engine.recognize(await readFile(image), { fileName: image });
  await mkdir(output, { recursive: true });
  await writeFile(resolve(output, 'result.json'), JSON.stringify(result, null, 2) + '\n');
  async function dump(path, destination) {
    await mkdir(destination, { recursive: true });
    for (const name of module.FS.readdir(path).filter(x => x !== '.' && x !== '..')) {
      const source = path + '/' + name, target = resolve(destination, name);
      if (module.FS.isDir(module.FS.stat(source).mode)) await dump(source, target);
      else { await writeFile(target, module.FS.readFile(source)); module.FS.unlink(source); }
    }
  }
  await dump('/trace', resolve(output));
} finally { engine.close(); }
