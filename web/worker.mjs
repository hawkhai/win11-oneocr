import { createOneOcr } from './oneocr.mjs';

let send, listen;
if (typeof process !== 'undefined' && process.versions?.node) {
  const { parentPort } = await import('node:worker_threads');
  send = value => parentPort.postMessage(value);
  listen = callback => parentPort.on('message', callback);
} else {
  send = value => self.postMessage(value);
  listen = callback => { self.onmessage = event => callback(event.data); };
}
let engine, queue = Promise.resolve();
async function dispatch({ id, method, payload }) {
  try {
    let result;
    if (method === 'create') {
      if (engine) throw new Error('Worker already initialized');
      engine = await createOneOcr(payload);
      result = { capabilities: engine.capabilities, manifest: engine.manifest };
    } else {
      if (!engine) throw new Error('Worker is not initialized');
      if (method === 'recognize') result = engine.recognize(payload.input, payload.options);
      else if (method === 'recognizeBgra') result = engine.recognizeBgra(payload.input, payload.options);
      else if (method === 'close') { engine.close(); engine = undefined; result = null; }
      else throw new Error('Unknown worker method');
    }
    send({ id, result });
  } catch (error) {
    send({ id, error: { name: error.name, message: error.message, code: error.code, stage: error.stage } });
  }
}
listen(message => { queue = queue.then(() => dispatch(message)); });
