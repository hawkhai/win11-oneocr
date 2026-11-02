// The entire OCR pipeline runs in one WebAssembly module. No native subprocess.
export class OneOcrError extends Error {
  constructor(code, message, stage) {
    super(message);
    this.name = 'OneOcrError';
    this.code = code;
    this.stage = stage;
  }
}

function bytes(value) {
  if (value instanceof Uint8Array) return value;
  if (value instanceof ArrayBuffer) return new Uint8Array(value);
  throw new OneOcrError(-12, 'Expected Uint8Array or ArrayBuffer', 'input');
}

function relativePath(value) {
  if (typeof value !== 'string' || !value || value.includes('\\') ||
      value.split('/').some(p => !p || p === '.' || p === '..') ||
      /[:?#\x00-\x1f]/u.test(value)) {
    throw new OneOcrError(-12, 'Invalid relative asset path', 'load');
  }
  return value;
}

export function modelAssetPaths(manifest) {
  if (manifest?.schema_version !== 1) throw new OneOcrError(-30, 'Unsupported model manifest version', 'load');
  const paths = new Set();
  const visit = value => {
    if (!value || typeof value !== 'object') return;
    if (typeof value.file === 'string' && typeof value.sha256 === 'string') paths.add(relativePath(value.file));
    for (const item of Object.values(value)) visit(item);
  };
  visit(manifest);
  return [...paths].sort();
}

async function readUrl(url) {
  if (url.protocol === 'file:' && typeof process !== 'undefined' && process.versions?.node) {
    const { readFile } = await import('node:fs/promises');
    return new Uint8Array(await readFile(url));
  }
  const response = await fetch(url);
  if (!response.ok) throw new OneOcrError(-30, `Cannot load ${url}: HTTP ${response.status}`, 'load');
  return new Uint8Array(await response.arrayBuffer());
}

/**
 * Load unchanged manifest assets into the module's private MEMFS. Native code
 * verifies the SHA-256 of every model/metadata record before creating sessions.
 * loadAsset(relativePath) may supply local files, an application cache, or fetch.
 */
export async function createOneOcr({
  modelBaseUrl = new URL('../../models/', import.meta.url),
  loadAsset,
  runtimeUrl = new URL('./oneocr-core.mjs', import.meta.url),
  wasmUrl,
  moduleFactory,
  onProgress,
  manifestSha256,
} = {}) {
  const base = new URL(modelBaseUrl, import.meta.url);
  if (!base.pathname.endsWith('/')) throw new OneOcrError(-12, 'modelBaseUrl must end with /', 'load');
  const loader = loadAsset ?? (path => readUrl(new URL(path.split('/').map(encodeURIComponent).join('/'), base)));
  const manifestBytes = bytes(await loader('manifest.json'));
  if (manifestSha256) {
    if (typeof manifestSha256 !== 'string' || !/^[\da-f]{64}$/iu.test(manifestSha256)) throw new OneOcrError(-12, 'manifestSha256 must contain 64 hexadecimal digits', 'load');
    const digest = await globalThis.crypto.subtle.digest('SHA-256', manifestBytes);
    const actual = [...new Uint8Array(digest)].map(x => x.toString(16).padStart(2, '0')).join('');
    if (actual !== manifestSha256.toLowerCase()) throw new OneOcrError(-30, 'Model manifest checksum mismatch', 'load');
  }
  let manifest;
  try { manifest = JSON.parse(new TextDecoder('utf-8', { fatal: true }).decode(manifestBytes)); }
  catch (error) { throw new OneOcrError(-30, `Invalid model manifest: ${error.message}`, 'load'); }
  const files = modelAssetPaths(manifest);
  const factory = moduleFactory ?? (await import(runtimeUrl.toString())).default;
  const configuration = {};
  if (wasmUrl) configuration.locateFile = name => name.endsWith('.wasm') ? new URL(wasmUrl, runtimeUrl).href : name;
  const module = await factory(configuration);
  module.FS.mkdirTree('/models');
  module.FS.mkdirTree('/inputs');
  module.FS.writeFile('/models/manifest.json', manifestBytes);
  try {
    for (let i = 0; i < files.length; i++) {
      const name = files[i];
      const path = '/models/' + name;
      const data = bytes(await loader(name));
      module.FS.mkdirTree(path.slice(0, path.lastIndexOf('/')));
      // Transfer ownership to MEMFS; loaders must treat returned bytes as immutable.
      module.FS.writeFile(path, data, { canOwn: true });
      onProgress?.({ file: name, completed: i + 1, total: files.length });
    }
    return new OneOcr(module, manifest);
  } catch (error) {
    // No context was created (the constructor cleans up a failed create).
    for (const name of files) {
      try { module.FS.unlink('/models/' + name); } catch { /* not loaded */ }
    }
    module.FS.unlink('/models/manifest.json');
    throw error;
  }
}

class OneOcr {
  #module;
  #context = 0;
  #closed = false;
  #allocations = [];
  #files;

  constructor(module, manifest) {
    this.#module = module;
    this.manifest = manifest;
    this.#files = modelAssetPaths(manifest);
    this.capabilities = JSON.parse(module.UTF8ToString(module._oneocr_capabilities()));
    this.#context = this.#call('create', (output, error) =>
      module._oneocr_create(this.#string('/models'), output, error), true);
  }

  #allocate(size) {
    const pointer = this.#module._malloc(size) >>> 0;
    if (!pointer) throw new OneOcrError(-19, 'Memory allocation failed', 'allocate');
    this.#allocations.push(pointer);
    return pointer;
  }

  #string(value) {
    const module = this.#module;
    const pointer = this.#allocate(module.lengthBytesUTF8(value) + 1);
    module.stringToUTF8(value, pointer, module.lengthBytesUTF8(value) + 1);
    return pointer;
  }

  #options(options) {
    const pointer = this.#allocate(12);
    const values = [options.maxLines ?? 0, options.resizeWidth ?? 0, options.resizeHeight ?? 0];
    for (let i = 0; i < values.length; i++) {
      const value = values[i];
      if (!Number.isInteger(value) || value < -2147483648 || value > 2147483647) {
        throw new OneOcrError(-12, 'OCR options must be int32 values', 'options');
      }
      this.#module.HEAP32[(pointer >>> 2) + i] = value;
    }
    return pointer;
  }

  #call(stage, invoke, handle = false) {
    if (this.#closed) throw new OneOcrError(-12, 'OneOCR instance is closed', stage);
    const module = this.#module;
    let output = 0, error = 0, result = 0, message = 0;
    try {
      output = this.#allocate(4); error = this.#allocate(4);
      module.HEAPU32[output >>> 2] = 0; module.HEAPU32[error >>> 2] = 0;
      const code = invoke(output, error);
      result = module.HEAPU32[output >>> 2]; message = module.HEAPU32[error >>> 2];
      if (code !== 0) throw new OneOcrError(code, message ? module.UTF8ToString(message) : `OneOCR failed: ${code}`, stage);
      if (!result) throw new OneOcrError(-31, 'OneOCR returned an empty output pointer', stage);
      return handle ? result : JSON.parse(module.UTF8ToString(result));
    } finally {
      if (message) module._oneocr_free(message);
      if (result && !handle) module._oneocr_free(result);
      for (const pointer of this.#allocations) module._free(pointer);
      this.#allocations.length = 0;
    }
  }

  /** Encoded image bytes, decoded by the same stb_image implementation as native. */
  recognize(input, { fileName = 'image', ...options } = {}) {
    if (this.#closed) throw new OneOcrError(-12, 'OneOCR instance is closed', 'recognize_file');
    const data = bytes(input);
    if (typeof fileName !== 'string' || fileName.includes('\0')) throw new OneOcrError(-12, 'Invalid fileName', 'input');
    const path = '/inputs/current-image';
    this.#module.FS.writeFile(path, data);
    try {
      const result = this.#call('recognize_file', (output, error) =>
        this.#module._oneocr_recognize_file(this.#context, this.#string(path), this.#options(options), output, error));
      result.file = fileName;
      return result;
    } finally { this.#module.FS.unlink(path); }
  }

  /** BGRA bytes including row padding; alpha is ignored, as in the native ABI. */
  recognizeBgra(input, { width, height, stride = width * 4, ...options } = {}) {
    const data = bytes(input);
    for (const value of [width, height]) {
      if (!Number.isInteger(value) || value < -2147483648 || value > 2147483647) throw new OneOcrError(-12, 'width/height must be int32', 'input');
    }
    if (!Number.isSafeInteger(stride)) throw new OneOcrError(-12, 'stride must be a safe integer', 'input');
    return this.#call('recognize_bgra', (output, error) => {
      const pointer = this.#allocate(Math.max(1, data.byteLength));
      this.#module.HEAPU8.set(data, pointer);
      return this.#module._oneocr_recognize_bgra(this.#context, pointer, data.byteLength, width, height,
        BigInt(stride), this.#options(options), output, error);
    });
  }

  close() {
    if (this.#closed) return;
    this.#module._oneocr_destroy(this.#context);
    this.#context = 0;
    this.#closed = true;
    for (const name of this.#files) this.#module.FS.unlink('/models/' + name);
    this.#module.FS.unlink('/models/manifest.json');
    // Dropping the module allows its WebAssembly.Memory to be collected.
    this.#module = null;
  }
}
