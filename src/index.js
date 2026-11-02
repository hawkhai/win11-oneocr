import { spawn } from 'node:child_process'
import { promises as fs } from 'node:fs'
import { tmpdir } from 'node:os'
import { dirname, isAbsolute, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'
import { defineTool } from '@deepseek-ai/dsh-tools'

export const name = 'win11-oneocr'
export const inject = ['tools']

const DEFAULT_OCR_BIN = fileURLToPath(new URL('../bin/ocr.exe', import.meta.url))
const PORTABLE_OCR_BIN = fileURLToPath(new URL(`../artifacts/${process.platform}-${process.arch}/bin/ocr${process.platform === 'win32' ? '.exe' : ''}`, import.meta.url))
const DEFAULT_MODEL_DIR = fileURLToPath(new URL('../models', import.meta.url))

function positiveInteger(value, fallback, label) {
  const resolved = value === undefined ? fallback : value
  if (!Number.isInteger(resolved) || resolved < 1) throw new Error(`${label} must be a positive integer`)
  return resolved
}

function sessionPath(path, exec) {
  if (isAbsolute(path)) return path
  const cwd = exec.agent?.session?.header?.cwd ?? process.cwd()
  return resolve(cwd, path)
}

function runProcess(command, args, { cwd, timeoutMs, maxOutputBytes, signal }) {
  return new Promise((resolveRun, reject) => {
    if (signal?.aborted) return reject(new Error('OneOCR call aborted'))
    const child = spawn(command, args, { cwd, windowsHide: true, shell: false, stdio: ['ignore', 'pipe', 'pipe'] })
    let stdout = Buffer.alloc(0)
    let stderr = Buffer.alloc(0)
    let settled = false

    const stop = () => {
      if (child.exitCode !== null || child.signalCode !== null) return
      if (process.platform === 'win32' && child.pid) {
        const killer = spawn('taskkill', ['/pid', String(child.pid), '/t', '/f'], { windowsHide: true, stdio: 'ignore' })
        killer.unref()
      } else child.kill('SIGKILL')
    }
    const finish = (error, value) => {
      if (settled) return
      settled = true
      clearTimeout(timer)
      signal?.removeEventListener('abort', onAbort)
      error ? reject(error) : resolveRun(value)
    }
    const append = (current, chunk, stream) => {
      const next = Buffer.concat([current, chunk])
      if (next.length > maxOutputBytes) {
        stop()
        finish(new Error(`${stream} exceeded ${maxOutputBytes} bytes`))
      }
      return next
    }
    const onAbort = () => {
      stop()
      finish(new Error('OneOCR call aborted'))
    }
    const timer = setTimeout(() => {
      stop()
      finish(new Error(`OneOCR timed out after ${timeoutMs} ms`))
    }, timeoutMs)
    signal?.addEventListener('abort', onAbort, { once: true })
    child.stdout.on('data', (chunk) => { stdout = append(stdout, chunk, 'stdout') })
    child.stderr.on('data', (chunk) => { stderr = append(stderr, chunk, 'stderr') })
    child.on('error', (error) => finish(error))
    child.on('close', (code) => {
      if (settled) return
      if (code !== 0) return finish(new Error(`OneOCR exited with code ${code}: ${stderr.toString('utf8').trim().slice(0, 2000)}`))
      finish(undefined)
    })
  })
}

function plainText(result) {
  if (!Array.isArray(result?.lines)) return ''
  return result.lines.map((line) => typeof line?.text === 'string' ? line.text : '').filter(Boolean).join('\n')
}

export function apply(ctx, config = {}) {
  const backend = config.backend ?? 'portable'
  if (!['portable', 'legacy'].includes(backend)) throw new Error('backend must be portable or legacy')
  const ocrBin = typeof config.ocrBin === 'string' && config.ocrBin ? resolve(config.ocrBin) : backend === 'legacy' ? DEFAULT_OCR_BIN : PORTABLE_OCR_BIN
  const modelDir = typeof config.modelDir === 'string' && config.modelDir ? resolve(config.modelDir) : DEFAULT_MODEL_DIR
  const timeoutMs = positiveInteger(config.timeoutMs, 60_000, 'timeoutMs')
  const maxOutputBytes = positiveInteger(config.maxOutputBytes, 4 * 1024 * 1024, 'maxOutputBytes')

  ctx.tools.register(defineTool({
    name: 'oneocr_recognize',
    description: 'Recognize text in a local image with offline OneOCR. The portable native backend runs on supported desktop platforms; its JSON includes capability and parity status. Returns plain text and structured line/word results.',
    parameters: {
      image_path: { type: 'string', required: true, description: 'Absolute path, or a path relative to the current session workspace.' },
      max_lines: { type: 'integer', description: 'Maximum recognition lines (1-1000). Default: 1000.' },
      resize_width: { type: 'integer', description: 'Optional internal resize width; use together with resize_height.' },
      resize_height: { type: 'integer', description: 'Optional internal resize height; use together with resize_width.' }
    },
    output: {
      schema: {
        type: 'object',
        additionalProperties: false,
        properties: {
          text: { type: 'string', required: true },
          result: { type: 'string', required: true }
        }
      },
      render: (_args, value) => [{ type: 'text', text: `<ocr_text>\n${value.text}\n</ocr_text>\n<oneocr_json>\n${value.result}\n</oneocr_json>` }]
    },
    isConcurrencySafe: () => true,
    async execute(args, exec) {
      if (exec.signal?.aborted) throw new Error('OneOCR call aborted')
      if (backend === 'legacy' && process.platform !== 'win32') throw new Error('The legacy OneOCR backend requires Windows')
      if (typeof args.image_path !== 'string' || !args.image_path.trim()) throw new Error('image_path must be a non-empty string')
      const maxLines = positiveInteger(args.max_lines, 1000, 'max_lines')
      if (maxLines > 1000) throw new Error('max_lines must not exceed 1000')
      const hasWidth = args.resize_width !== undefined
      const hasHeight = args.resize_height !== undefined
      if (hasWidth !== hasHeight) throw new Error('resize_width and resize_height must be supplied together')
      const tempDir = await fs.mkdtemp(join(tmpdir(), 'dsh-oneocr-'))
      const outputPath = join(tempDir, 'result.json')
      const argv = [sessionPath(args.image_path, exec), '--output', outputPath, '--max-lines', String(maxLines), '--quiet']
      if (backend === 'portable') argv.push('--model-dir', modelDir)
      if (hasWidth) {
        argv.push('--resize', `${positiveInteger(args.resize_width, 0, 'resize_width')}x${positiveInteger(args.resize_height, 0, 'resize_height')}`)
      }
      try {
        await runProcess(ocrBin, argv, { cwd: dirname(ocrBin), timeoutMs, maxOutputBytes, signal: exec.signal })
        const info = await fs.stat(outputPath)
        if (info.size > maxOutputBytes) throw new Error(`OneOCR result exceeded ${maxOutputBytes} bytes`)
        const result = JSON.parse(await fs.readFile(outputPath, 'utf8'))
        return { text: plainText(result), result: JSON.stringify(result, null, 2) }
      } finally {
        await fs.rm(tempDir, { recursive: true, force: true })
      }
    },
    presentCall: (args) => ({ card: 'generic', title: `OneOCR ${args.image_path}`, kind: 'read', locations: [{ path: args.image_path }] })
  }))
}
