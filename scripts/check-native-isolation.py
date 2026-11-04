"""Private-runtime C ABI isolation and success/error/success recovery witness.

Uses an existing registered image and its original-DLL baseline. This is an
additional contract check, not a replacement for the 269-case platform suite.
Python is the inspection harness, not a dependency of native recognition.
"""
import argparse
import ctypes as C
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def modules():
    if os.name != 'nt':
        return sorted({row.split(None, 5)[5].strip() for row in Path('/proc/self/maps').read_text().splitlines()
                       if len(row.split(None, 5)) == 6 and row.split(None, 5)[5].startswith('/')})
    from ctypes import wintypes as W
    kernel, psapi = C.WinDLL('kernel32'), C.WinDLL('psapi')
    kernel.GetCurrentProcess.restype = W.HANDLE
    psapi.EnumProcessModulesEx.argtypes = [W.HANDLE, C.POINTER(W.HMODULE), W.DWORD, C.POINTER(W.DWORD), W.DWORD]
    psapi.GetModuleFileNameExW.argtypes = [W.HANDLE, W.HMODULE, W.LPWSTR, W.DWORD]
    process = kernel.GetCurrentProcess()
    values, needed = (W.HMODULE * 4096)(), W.DWORD()
    if not psapi.EnumProcessModulesEx(process, values, C.sizeof(values), C.byref(needed), 3):
        raise RuntimeError('Module enumeration failed')
    if needed.value > C.sizeof(values):
        raise RuntimeError('Module enumeration truncated')
    result = []
    for module in values[:needed.value // C.sizeof(W.HMODULE)]:
        name = C.create_unicode_buffer(32768)
        if not psapi.GetModuleFileNameExW(process, module, name, len(name)):
            raise RuntimeError('Module filename unavailable')
        result.append(name.value)
    return sorted(set(result))


def worker(args):
    directory = os.add_dll_directory(str(args.library.parent)) if os.name == 'nt' else None
    dll = C.CDLL(str(args.library))
    pointer = C.c_void_p
    dll.oneocr_create.argtypes = [C.c_char_p, C.POINTER(pointer), C.POINTER(pointer)]
    dll.oneocr_create.restype = C.c_int
    dll.oneocr_recognize_file.argtypes = [pointer, C.c_char_p, pointer, C.POINTER(pointer), C.POINTER(pointer)]
    dll.oneocr_recognize_file.restype = C.c_int
    dll.oneocr_destroy.argtypes = [pointer]
    dll.oneocr_destroy.restype = None
    dll.oneocr_free.argtypes = [pointer]
    dll.oneocr_free.restype = None
    context, error = pointer(), pointer()

    def release_text(value):
        try:
            return C.string_at(value).decode('utf-8') if value else None
        finally:
            dll.oneocr_free(value)

    code = dll.oneocr_create(str(args.models).encode('utf-8'), C.byref(context), C.byref(error))
    message = release_text(error)
    if code:
        raise RuntimeError(message)
    try:
        def recognize(path):
            # Non-null sentinels prove the ABI clears stale output pointers.
            result, error = pointer(1), pointer(1)
            code = dll.oneocr_recognize_file(context, str(path).encode('utf-8'), None, C.byref(result), C.byref(error))
            return code, release_text(result), release_text(error)

        code, value, message = recognize(args.image)
        if code or message or not value:
            raise RuntimeError('Initial recognition failed: ' + str(message))
        before = json.loads(value)
        code, value, message = recognize(args.output.parent / '__missing_image__.png')
        if code != -13 or value is not None or not message:
            raise AssertionError('Failure must clear output and report image error')
        code, value, message = recognize(args.image)
        if code or message or json.loads(value) != before:
            raise AssertionError('Successful result/error state changed after rejection')
        spec = importlib.util.spec_from_file_location('strict', ROOT / 'scripts/compare-results.py')
        comparator = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(comparator)
        comparison = comparator.compare(json.loads(args.baseline.read_text(encoding='utf-8')), before)
        if not comparison['passed']:
            raise AssertionError('Original DLL comparison failed: ' + json.dumps(comparison))
        loaded = modules()
        runtime = [Path(name) for name in loaded if 'onnxruntime' in Path(name).name.lower()
                   or 'oneocr' in Path(name).name.lower() and Path(name).suffix.lower() in ('.dll', '.so')]
        if not any('onnxruntime' in path.name.lower() for path in runtime):
            raise AssertionError('ORT module was not observed')
        if not any('oneocr_portable' in path.name.lower() for path in runtime):
            raise AssertionError('Native engine module was not observed')
        if any(path.resolve().parent != args.library.parent.resolve() for path in runtime):
            raise AssertionError('Native engine/ORT loaded outside private runtime directory')
        if any(path.name.lower() in ('oneocr.dll', 'oneocr_wrapper.dll') for path in runtime):
            raise AssertionError('Reference/compatibility DLL loaded')
        return dict(passed=True, checks=['success-error-success exact result recovery', 'stale outputs cleared',
                    'original DLL strict comparison', 'private native engine/ORT modules'],
                    loaded_modules=loaded, native_modules={str(p): sha(p) for p in runtime},
                    input_sha256=sha(args.image), baseline_sha256=sha(args.baseline),
                    fresh_inference=True, full_regression=False, registered_scope_unchanged=True,
                    python_is_test_harness_only=True)
    finally:
        dll.oneocr_destroy(context)
        if directory:
            directory.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, help='Existing installed runtime directory, containing bin/ and optional lib/')
    parser.add_argument('--models', type=Path, default=ROOT / 'models')
    parser.add_argument('--image', type=Path, default=ROOT / 'ocr-book.jpg')
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--library', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Use a new output file')
    sys.path.insert(0, str(ROOT / 'scripts'))
    from regression_reference import active_reference
    args.baseline = (args.baseline or ROOT / active_reference()['book_baseline']).resolve()
    args.models, args.image, args.output = args.models.resolve(), args.image.resolve(), args.output.resolve()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.library:
        record = worker(args)
        args.output.write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
        return
    if not args.runtime:
        parser.error('--runtime is required')
    library_name = 'oneocr_portable.dll' if os.name == 'nt' else 'liboneocr_portable.so'
    with tempfile.TemporaryDirectory(prefix='oneocr-isolated-') as temporary:
        private = Path(temporary)
        for folder in ('bin', 'lib'):
            for file in (args.runtime / folder).glob('*'):
                if file.is_file() and (file.suffix.lower() in ('.dll', '.so') or '.so.' in file.name):
                    # The old compatibility wrapper is unnecessary for this native C ABI witness.
                    if file.name.lower() != 'oneocr_wrapper.dll':
                        shutil.copyfile(file, private / file.name)
        environment = {key: value for key, value in os.environ.items()
                       if key not in ('LD_LIBRARY_PATH', 'LD_PRELOAD', 'ORT_DYLIB_PATH', 'PYTHONPATH')}
        environment['PATH'] = str(Path(os.environ['SystemRoot']) / 'System32') if os.name == 'nt' else '/usr/bin:/bin'
        subprocess.run([sys.executable, str(Path(__file__).resolve()), '--library', str(private / library_name),
                        '--models', str(args.models), '--image', str(args.image), '--baseline', str(args.baseline),
                        '--output', str(args.output)], cwd=private, env=environment, check=True, timeout=180)
    print(json.dumps({'passed': True, 'output': str(args.output), 'full_regression': False}))


if __name__ == '__main__':
    main()
