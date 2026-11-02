export class OneOcrError extends Error {
  readonly code: number;
  readonly stage: string;
  constructor(code: number, message: string, stage: string);
}
export type ImageBytes = Uint8Array | ArrayBuffer;
export interface RecognitionOptions {
  fileName?: string;
  maxLines?: number;
  resizeWidth?: number;
  resizeHeight?: number;
}
export interface BgraOptions extends Omit<RecognitionOptions, 'fileName'> {
  width: number;
  height: number;
  stride?: number;
}
export type Quadrilateral = [number, number, number, number, number, number, number, number];
export interface RecognizedWord {
  index: number;
  text: string;
  bounding_box: Quadrilateral;
  confidence: number;
}
export interface RecognizedLine {
  index: number;
  text: string;
  bounding_box: Quadrilateral;
  style: { type: 'printed' | 'handwritten'; confidence: number };
  word_count: number;
  words: RecognizedWord[];
}
export interface RecognitionResult {
  file: string;
  image: { width: number; height: number; step: number };
  image_angle: number;
  line_count: number;
  lines: RecognizedLine[];
  engine: Record<string, unknown>;
}
export interface CreateOptions {
  modelBaseUrl?: string | URL;
  runtimeUrl?: string | URL;
  wasmUrl?: string | URL;
  manifestSha256?: string;
  loadAsset?: (relativePath: string) => ImageBytes | Promise<ImageBytes>;
  onProgress?: (progress: { file: string; completed: number; total: number }) => void;
}
export interface OneOcr {
  readonly capabilities: Record<string, unknown>;
  readonly manifest: Record<string, unknown>;
  recognize(input: ImageBytes, options?: RecognitionOptions): RecognitionResult;
  recognizeBgra(input: ImageBytes, options: BgraOptions): RecognitionResult;
  close(): void;
}
export function modelAssetPaths(manifest: Record<string, unknown>): string[];
export function createOneOcr(options?: CreateOptions): Promise<OneOcr>;
