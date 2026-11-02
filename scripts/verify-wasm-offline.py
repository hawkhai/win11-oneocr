"""Run only committed Wasm/model assets in a Linux network namespace without DLLs."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node', default='node', help='Linux Node executable')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != 'linux':
        parser.error('Requires Linux unshare network namespace (Windows: run inside WSL)')
    node = shutil.which(args.node)
    if not node:
        parser.error('Linux Node executable not found')
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error('Output directory must be empty')
    output.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((ROOT/'research/delivery-manifest.json').read_text())
    with tempfile.TemporaryDirectory(prefix='oneocr-wasm-offline-') as name:
        isolated = Path(name)
        shutil.copytree(ROOT/'artifacts/wasm32', isolated/'artifacts/wasm32')
        shutil.copytree(ROOT/'models', isolated/'models')
        shutil.copyfile(ROOT/'ocr-book.jpg', isolated/'ocr-book.jpg')
        files = sorted(p for p in isolated.rglob('*') if p.is_file())
        if any(p.suffix.lower() in {'.dll', '.so', '.exe', '.dylib'} for p in files):
            raise RuntimeError('Native executable/library found in isolation directory')
        fingerprints = {p.relative_to(isolated).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
        for name, digest in fingerprints.items():
            if name in manifest['files'] and digest != manifest['files'][name]['sha256']:
                raise RuntimeError('Changed isolated asset: '+name)
        destination = isolated/'result.json'
        began = time.perf_counter()
        # unshare failure is fatal; there is no online or native-backend fallback.
        command = ['unshare', '--net', '--', node, str(isolated/'artifacts/wasm32/cli.mjs'),
                   str(isolated/'ocr-book.jpg'), '--model-dir', str(isolated/'models'),
                   '--quiet', '--output', str(destination)]
        completed = subprocess.run(command, cwd=isolated, capture_output=True, text=True, timeout=900)
        milliseconds = (time.perf_counter()-began)*1000
        (output/'stdout.txt').write_text(completed.stdout)
        (output/'stderr.txt').write_text(completed.stderr)
        completed.check_returncode()
        actual = json.loads(destination.read_text())
        (output/'book.json').write_bytes(destination.read_bytes())
    baseline_path = ROOT/manifest['reference']['book_baseline']
    spec = importlib.util.spec_from_file_location('strict_compare', ROOT/'scripts/compare-results.py')
    comparator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(comparator)
    comparison = comparator.compare(json.loads(baseline_path.read_text(encoding='utf8')), actual)
    report = {'passed': comparison['passed'], 'network': 'Linux unshare --net; no host interfaces/routes',
              'node': subprocess.check_output([node, '--version'], text=True).strip(),
              'milliseconds': milliseconds, 'isolated_asset_sha256': fingerprints,
              'native_libraries_in_isolation': [], 'comparison': comparison}
    (output/'summary.json').write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf8')
    print(json.dumps({k:report[k] for k in ('passed','node','milliseconds')}))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    sys.exit(main())
