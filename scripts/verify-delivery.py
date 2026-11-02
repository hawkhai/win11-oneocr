"""Verify committed runtime/model hashes and optionally replay all samples. Standard library only."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--all-samples',action='store_true')
    parser.add_argument('--output',type=Path)
    args=parser.parse_args()
    manifest=json.loads((ROOT/'research/delivery-manifest.json').read_text(encoding='utf8'))
    reference=manifest.get('reference',{})
    model_manifest=json.loads((ROOT/'models/manifest.json').read_text(encoding='utf8'))
    if reference and reference['model_source_sha256']!=model_manifest['source_assets']['oneocr.onemodel']:
        raise RuntimeError('Active model and regression reference are bound to different source versions')
    for name,record in manifest['files'].items():
        path=ROOT/name
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=record['sha256']:
            raise RuntimeError('Missing or changed delivery asset: '+name)
    machine=platform.machine().lower()
    if machine not in ('amd64','x86_64'):raise RuntimeError('No verified prebuilt runtime for architecture: '+machine)
    key='win32-x64' if sys.platform=='win32' else 'linux-x64' if sys.platform=='linux' else None
    if not key:raise RuntimeError('No verified prebuilt runtime for OS: '+sys.platform)
    if key=='linux-x64':
        libc,version=platform.libc_ver()
        if libc!='glibc' or tuple(map(int,version.split('.'))) < (2,43):
            raise RuntimeError('This Linux bundle requires glibc >= 2.43. Build from source on older systems; see docs/portable.md')
    binary=ROOT/manifest['platforms'][key]['entry']
    if not os.access(binary,os.X_OK):raise RuntimeError('Executable bit missing: use git clone, or chmod +x '+str(binary))
    with tempfile.TemporaryDirectory(prefix='oneocr-delivery-') as temporary:
        destination=Path(temporary)/'book.json'
        # Execute from an unrelated directory so no working-tree path is implicit.
        subprocess.run([str(binary),'--model-dir',str(ROOT/'models'),str(ROOT/'ocr-book.jpg'),'--quiet','--output',str(destination)],cwd=temporary,check=True,timeout=120)
        result=json.loads(destination.read_text(encoding='utf8'))
        # The whole-book baseline is committed alongside the original corpus.
        baseline=json.loads((ROOT/reference.get('book_baseline','research/case/baseline/ocr-book.json')).read_text(encoding='utf8'))
        import importlib.util
        spec=importlib.util.spec_from_file_location('delivery_compare',ROOT/'scripts/compare-results.py')
        comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
        comparison=comparator.compare(baseline,result)
        if not comparison['passed']:raise RuntimeError('Book parity failed: '+json.dumps(comparison,ensure_ascii=False))
    if args.all_samples:
        output=(args.output or ROOT/'work/delivery-all-samples').resolve()
        subprocess.run([sys.executable,'-X','utf8',str(ROOT/'scripts/run-all-samples.py'),'--inventory',str(ROOT/'samples/manifest.json'),'--baseline',str(ROOT/reference.get('all_samples_baseline','reports/oneocr-all-samples-2026-11-02/legacy')),'--binary',str(binary),'--output',str(output)],cwd=ROOT,check=True)
    print(json.dumps({'passed':True,'platform':key,'verified_assets':len(manifest['files']),'book_lines':len(result['lines']),'all_samples':args.all_samples},ensure_ascii=False))
    return 0

if __name__=='__main__':sys.exit(main())
