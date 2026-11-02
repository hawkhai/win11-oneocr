"""Freeze the checkout-ready runtime and preserve the local research record. No downloads."""
import hashlib
import json
from pathlib import Path
import shutil
import argparse

ROOT=Path(__file__).resolve().parents[1]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-report',type=Path,help='A completed, model-bound regression report with legacy/ and book-legacy.json')
    args=parser.parse_args()
    destination=ROOT/'research/case'
    source=ROOT/'work/oneocr-portable'
    if source.exists():
        destination.mkdir(parents=True,exist_ok=True)
        for pattern in ('*.md','routing.json'):
            for path in source.glob(pattern):shutil.copyfile(path,destination/path.name)
        for directory in ('evidence','notes','report','baseline','corpus','corpus-legacy','classics-error-baseline','handwriting-legacy'):
            for path in (source/directory).rglob('*'):
                if path.is_file():
                    target=destination/path.relative_to(source);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
    files={}
    for platform in ('win32-x64','linux-x64'):
        prefix=ROOT/'artifacts'/platform
        notices=prefix/'share/oneocr';notices.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(ROOT/'THIRD_PARTY_NOTICES.md',notices/'THIRD_PARTY_NOTICES.md')
        shutil.copyfile(ROOT/'docs/portable.md',notices/'portable.md')
        shutil.copyfile(ROOT/'docs/error-codes.md',notices/'error-codes.md')
        for path in (ROOT/'licenses').glob('*.txt'):
            target=notices/'licenses'/path.name;target.parent.mkdir(exist_ok=True);shutil.copyfile(path,target)
        build=json.loads((prefix/'BUILD.json').read_text(encoding='utf8'))
        build['model_directory']='../../models'
        build['model_source_sha256']=json.loads((ROOT/'models/manifest.json').read_text(encoding='utf8'))['source_assets']['oneocr.onemodel']
        build['model_manifest_sha256']=sha(ROOT/'models/manifest.json')
        build['files']={path.relative_to(prefix).as_posix():sha(path) for path in sorted(prefix.rglob('*')) if path.is_file() and path.name!='BUILD.json' and 'models' not in path.relative_to(prefix).parts}
        (prefix/'BUILD.json').write_text(json.dumps(build,indent=2)+'\n',encoding='utf8')
    for directory in ('models','artifacts/win32-x64','artifacts/linux-x64'):
        for path in (ROOT/directory).rglob('*'):
            if not path.is_file():continue
            if path.is_relative_to(ROOT/'artifacts') and 'models' in path.relative_to(ROOT/'artifacts').parts:continue
            files[path.relative_to(ROOT).as_posix()]={'sha256':sha(path),'size':path.stat().st_size}
    manifest={'schema_version':1,'native_source_commit':'58dcdc2','model_ownership':'Microsoft; not MIT; no additional redistribution permission asserted',
              'platforms':{'win32-x64':{'entry':'artifacts/win32-x64/bin/ocr.exe','launcher':'ocr.cmd','system_requirements':'Windows 10/11 x64 with built-in UCRT; release VC 14.44 CRT bundled'},
                           'linux-x64':{'entry':'artifacts/linux-x64/bin/ocr','launcher':'ocr.sh','system_requirements':'glibc >= 2.43; release GCC C++ runtime bundled (tested Ubuntu 26.04 x64)'}},
              'common_models':'models/manifest.json','source_build':'docs/portable.md','files':dict(sorted(files.items()))}
    previous=ROOT/'research/delivery-manifest.json'
    if args.reference_report:
        report=args.reference_report.resolve()
        if not report.is_relative_to(ROOT) or not (report/'book-legacy.json').is_file() or not (report/'legacy').is_dir():
            raise ValueError('Regression report must be inside the project and contain complete frozen DLL outputs')
        summary=json.loads((report/'summary.json').read_text(encoding='utf8'))
        current=json.loads((ROOT/'models/manifest.json').read_text(encoding='utf8'))['source_assets']['oneocr.onemodel']
        if not summary['passed'] or summary['model_source_sha256']!=current:raise ValueError('Report is not a passed regression for the active model')
        base=report.relative_to(ROOT).as_posix()
        manifest['reference']={'model_source_sha256':current,'book_baseline':base+'/book-legacy.json','all_samples_baseline':base+'/legacy','corpus_baseline':base+'/corpus-legacy','legacy_binary_dir':summary['new_reference_assets'].rstrip('/'),'report':base+'/README.md'}
    elif previous.is_file():
        old=json.loads(previous.read_text(encoding='utf8'))
        if 'reference' in old:manifest['reference']=old['reference']
    if 'reference' in manifest:
        current=json.loads((ROOT/'models/manifest.json').read_text(encoding='utf8'))['source_assets']['oneocr.onemodel']
        if manifest['reference']['model_source_sha256']!=current:raise ValueError('Active model changed; provide its passed --reference-report')
    (ROOT/'research/delivery-manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print('Frozen',len(files),'model and runtime files; bytes',sum(x['size']for x in files.values()))

if __name__=='__main__':main()
