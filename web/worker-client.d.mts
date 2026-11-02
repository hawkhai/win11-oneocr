import type { BgraOptions, CreateOptions, ImageBytes, RecognitionOptions, RecognitionResult } from './oneocr.mjs';
export interface WorkerOptions extends Omit<CreateOptions, 'loadAsset' | 'onProgress'> {
  workerUrl?: string | URL;
  timeoutMs?: number;
  signal?: AbortSignal;
}
export interface OneOcrWorker {
  readonly capabilities: Record<string, unknown>;
  readonly manifest: Record<string, unknown>;
  recognize(input: ImageBytes, options?: RecognitionOptions): Promise<RecognitionResult>;
  recognizeBgra(input: ImageBytes, options: BgraOptions): Promise<RecognitionResult>;
  close(): Promise<void>;
  terminate(): void;
}
export function createOneOcrWorker(options?: WorkerOptions): Promise<OneOcrWorker>;
