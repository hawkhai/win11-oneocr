import { OneOcrError } from './oneocr.mjs';

/** One isolated Wasm instance per worker. Timeout/cancel terminates the worker. */
export async function createOneOcrWorker({ timeoutMs = 120000, signal, workerUrl = new URL('./worker.mjs', import.meta.url), ...configuration } = {}) {
  if (!Number.isSafeInteger(timeoutMs) || timeoutMs < 1) throw new TypeError('timeoutMs must be a positive integer');
  if (signal?.aborted) throw new Error('OneOCR call aborted');
  const node = typeof process !== 'undefined' && process.versions?.node;
  const WorkerClass = node ? (await import('node:worker_threads')).Worker : globalThis.Worker;
  // Node accepts file/data URL objects, but rejects the equivalent URL strings.
  // Keep native filesystem paths (including Windows drive letters) unchanged.
  const target = typeof workerUrl === 'string' && /^(file|data):/iu.test(workerUrl) ? new URL(workerUrl) : workerUrl;
  const worker = new WorkerClass(target, { type: 'module' });
  const pending = new Map();
  let sequence = 0, closed = false, terminalError;
  const stop = error => {
    if (closed) return;
    closed = true;
    terminalError = error;
    signal?.removeEventListener('abort', aborted);
    worker.terminate();
    for (const entry of pending.values()) { clearTimeout(entry.timer); entry.reject(error); }
    pending.clear();
  };
  const aborted = () => stop(new Error('OneOCR call aborted'));
  signal?.addEventListener('abort', aborted, { once: true });
  if (signal?.aborted) aborted();
  const receive = message => {
    const entry = pending.get(message.id);
    if (!entry) return;
    pending.delete(message.id); clearTimeout(entry.timer);
    if (message.error) {
      const e = message.error;
      entry.reject(typeof e.code === 'number' ? new OneOcrError(e.code, e.message, e.stage) :
        Object.assign(new Error(e.message), { name: e.name, code: e.code, stage: e.stage }));
    } else entry.resolve(message.result);
  };
  if (node) {
    worker.on('message', receive);
    worker.on('error', error => stop(error));
    worker.on('exit', code => { if (!closed) stop(new Error(`OneOCR worker exited: ${code}`)); });
  } else {
    worker.onmessage = event => receive(event.data);
    worker.onerror = event => stop(new Error(event.message || 'OneOCR worker failed'));
    worker.onmessageerror = () => stop(new Error('OneOCR worker message could not be decoded'));
  }
  const call = (method, payload) => new Promise((resolve, reject) => {
    if (closed) { reject(terminalError ?? new Error('OneOCR worker is closed')); return; }
    const id = ++sequence;
    const timer = setTimeout(() => stop(new Error(`OneOCR timed out after ${timeoutMs} ms`)), timeoutMs);
    pending.set(id, { resolve, reject, timer });
    try { worker.postMessage({ id, method, payload }); }
    catch (error) { pending.delete(id); clearTimeout(timer); reject(error); }
  });
  for (const key of ['modelBaseUrl', 'runtimeUrl', 'wasmUrl']) {
    if (configuration[key] instanceof URL) configuration[key] = configuration[key].href;
  }
  try {
    const metadata = await call('create', configuration);
    return {
      ...metadata,
      recognize: (input, options) => call('recognize', { input, options }),
      recognizeBgra: (input, options) => call('recognizeBgra', { input, options }),
      async close() {
        if (closed) return;
        try { await call('close'); } finally { stop(new Error('OneOCR worker is closed')); }
      },
      terminate() { stop(new Error('OneOCR call aborted')); },
    };
  } catch (error) { stop(error); throw error; }
}
