"""Run the frozen 30-image corpus with current native OneOCR and the DLL control.

Use the repository's .research-python/python.exe (Python 3.12 on this host).
Each image/backend runs in a new child process; times include process startup.
The input pixels are not rotated, dewarped or otherwise preprocessed externally.
"""
import argparse
import ctypes as C
from datetime import datetime, timezone, timedelta
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / 'samples/ocr-classics'
DEFAULT_OUTPUT = ROOT / 'reports/oneocr-classics-2026-11-02'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def legacy_image(image, output, binary_dir):
    # Use the ORIGINAL wrapper, never the native runtime's compatibility wrapper.
    directory = os.add_dll_directory(str(binary_dir))
    kernel = C.WinDLL('kernel32', use_last_error=True)
    kernel.SetDllDirectoryW.argtypes = [C.c_wchar_p]
    kernel.SetDllDirectoryW.restype = C.c_int
    if not kernel.SetDllDirectoryW(str(binary_dir)):
        raise C.WinError(C.get_last_error())
    dll = C.CDLL(str(binary_dir / 'oneocr_wrapper.dll'))
    dll.initModel.argtypes = [C.c_wchar_p]
    dll.initModel.restype = C.c_int
    dll.releaseModel.argtypes = []
    dll.releaseModel.restype = C.c_int
    allocator_type = C.CFUNCTYPE(C.c_void_p, C.c_size_t)
    dll.ocrImageEx.argtypes = [C.c_wchar_p, C.POINTER(C.c_void_p), allocator_type,
                               C.c_int, C.c_int, C.c_int]
    dll.ocrImageEx.restype = C.c_int
    crt = C.CDLL('msvcrt')
    crt.malloc.argtypes = [C.c_size_t]
    crt.malloc.restype = C.c_void_p
    crt.free.argtypes = [C.c_void_p]
    allocator = allocator_type(lambda size: crt.malloc(size))
    code = dll.initModel(str(binary_dir))
    if code:
        raise RuntimeError(f'Original DLL initialization failed: {code}')
    pointer = C.c_void_p()
    try:
        code = dll.ocrImageEx(str(image), C.byref(pointer), allocator, 1000, 1152, 768)
        if code:
            raise RuntimeError(f'Original wrapper OCR failed: {code}')
        result = json.loads(C.string_at(pointer).decode('utf-8'))
        write_json(output, result)
    finally:
        if pointer.value:
            crt.free(pointer)
        dll.releaseModel()
        directory.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--backend', choices=['both', 'native', 'legacy'], default='both')
    parser.add_argument('--native-bin', type=Path, default=ROOT / 'artifacts/win32-x64/bin/ocr.exe')
    parser.add_argument('--model-dir', type=Path, default=ROOT / 'models')
    parser.add_argument('--legacy-dir', type=Path, default=ROOT / 'bin')
    parser.add_argument('--timeout', type=int, default=120)
    parser.add_argument('--legacy-image', type=Path, help=argparse.SUPPRESS)
    parser.add_argument('--legacy-output', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.legacy_image:
        legacy_image(args.legacy_image.resolve(), args.legacy_output.resolve(), args.legacy_dir.resolve())
        return 0

    manifest_path = CORPUS / 'manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    output = args.output.resolve()
    cases = manifest['cases']
    assert len(cases) == 30, 'Expected the frozen 30-case collection'
    for case in cases:
        assert sha(CORPUS / case['file']) == case['sha256'], case['name']
    native = args.native_bin.resolve()
    models = args.model_dir.resolve()
    legacy = args.legacy_dir.resolve()
    backends = ['native', 'legacy'] if args.backend == 'both' else [args.backend]
    tracked = {'runner': Path(__file__), 'corpus_manifest': manifest_path}
    if 'native' in backends:
        tracked.update({'native_cli': native, 'native_core': native.parent / 'oneocr_portable.dll',
                        'native_ort': native.parent / 'onnxruntime.dll', 'model_manifest': models / 'manifest.json'})
    if 'legacy' in backends:
        tracked.update({f'legacy_{name}': legacy / name for name in
                        ['oneocr_wrapper.dll', 'oneocr.dll', 'oneocr.onemodel', 'onnxruntime.dll']})
    fingerprints = {key: {'file': str(path), 'sha256': sha(path)} for key, path in tracked.items()}
    # The exported ONNX models must originate from the same container as the DLL.
    models_manifest = json.loads((models / 'manifest.json').read_text(encoding='utf-8'))
    if 'native' in backends and 'legacy' in backends:
        assert models_manifest['source_assets']['oneocr.onemodel'] == sha(legacy / 'oneocr.onemodel')
    capability = None
    if 'native' in backends:
        capability = json.loads(subprocess.check_output([str(native), '--capabilities'], timeout=20))
    run = {'schema_version': 1, 'date': '2026-11-02',
           'started_at': datetime.now(timezone(timedelta(hours=8))).isoformat(),
           'host': {'platform': platform.platform(), 'python': sys.version,
                    'logical_processors': os.cpu_count()},
           'parameters': {'max_lines': 1000, 'resize_width': 1152, 'resize_height': 768,
                          'preprocessing': 'none; original file passed unchanged to each backend',
                          'execution': 'sequential, separate process per image/backend',
                          'timing': 'end-to-end child wall time including startup/model load; not a latency benchmark'},
           'native_capabilities': capability, 'fingerprints': fingerprints, 'measurements': []}
    write_json(output / 'run.json', run)
    for backend in backends:
        folder = output / backend
        folder.mkdir(parents=True, exist_ok=True)
        for case in cases:
            image = (CORPUS / case['file']).resolve()
            destination = folder / (case['name'] + '.json')
            if destination.exists():
                raise FileExistsError(f'Refusing to overwrite a run: {destination}; choose a new --output')
            if backend == 'native':
                command = [str(native), str(image), '--model-dir', str(models), '--max-lines', '1000',
                           '--resize', '1152x768', '--output', str(destination), '--quiet']
                cwd = native.parent
            else:
                command = [sys.executable, str(Path(__file__).resolve()), '--legacy-image', str(image),
                           '--legacy-output', str(destination), '--legacy-dir', str(legacy)]
                cwd = legacy
            started = time.perf_counter()
            record = {'name': case['name'], 'number': case['number'], 'backend': backend,
                      'input_sha256': case['sha256'], 'command': command}
            try:
                result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                        timeout=args.timeout, creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
                record.update(exit_code=result.returncode,
                              stdout=result.stdout.decode('utf-8', errors='replace')[-4000:],
                              stderr=result.stderr.decode('utf-8', errors='replace')[-4000:])
                if result.returncode != 0:
                    raise RuntimeError(f'Process exited with {result.returncode}')
                data = json.loads(destination.read_text(encoding='utf-8'))
                assert data['image']['width'] == case['width'] and data['image']['height'] == case['height']
                lines = data['lines']
                assert len(lines) == data['line_count']
                text = '\n'.join(x.get('text', '') for x in lines)
                (folder / (case['name'] + '.txt')).write_text(text + '\n', encoding='utf-8')
                record.update(status='success', lines=len(lines), characters=len(text), image_angle=data.get('image_angle'),
                              result_sha256=sha(destination))
            except Exception as exc:
                record.update(status='failed', error=str(exc))
            record['seconds'] = round(time.perf_counter() - started, 6)
            run['measurements'].append(record)
            write_json(output / 'run.json', run)
            print(f"{backend} {case['number']:02d} {case['name']}: {record['status']}, "
                  f"{record.get('lines', '?')} lines, {record['seconds']:.3f}s", flush=True)
    run['fingerprints_unchanged'] = all(sha(Path(x['file'])) == x['sha256'] for x in fingerprints.values())
    run['finished_at'] = datetime.now(timezone(timedelta(hours=8))).isoformat()
    run['process_successes'] = sum(x['status'] == 'success' for x in run['measurements'])
    run['process_total'] = len(run['measurements'])
    write_json(output / 'run.json', run)
    return 0 if run['process_successes'] == run['process_total'] and run['fingerprints_unchanged'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
