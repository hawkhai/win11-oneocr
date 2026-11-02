"""Stage a local native installation and independent model bundle.

Does not publish, upload, or copy the proprietary Windows OCR binaries.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT=Path(__file__).resolve().parents[1]

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build',type=Path,required=True)
    p.add_argument('--prefix',type=Path,required=True)
    p.add_argument('--models',type=Path,default=ROOT/'models')
    p.add_argument('--config',default='Release')
    p.add_argument('--cmake',default='cmake')
    args=p.parse_args();prefix=args.prefix.resolve();models=args.models.resolve()
    if not prefix.is_relative_to(ROOT.resolve()):raise SystemExit('Staging prefix must be inside the project')
    subprocess.run([args.cmake,'--install',str(args.build),'--config',args.config,'--prefix',str(prefix)],check=True)
    notices=prefix/'share/oneocr';notices.mkdir(parents=True,exist_ok=True)
    shutil.copy2(ROOT/'THIRD_PARTY_NOTICES.md',notices/'THIRD_PARTY_NOTICES.md')
    shutil.copy2(ROOT/'docs/error-codes.md',notices/'error-codes.md')
    for source in (ROOT/'licenses').glob('*.txt'):
        destination=notices/'licenses'/source.name;destination.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,destination)
    # npm omits symbolic links; materialize installed library aliases locally.
    # This also lets a Windows host inspect a bundle staged from WSL.
    for alias in prefix.rglob('*'):
        if not alias.is_symlink():continue
        target=alias.resolve(strict=True)
        if not target.is_relative_to(prefix) or not target.is_file():
            raise SystemExit('Installed symlink escapes staging prefix or is not a file')
        alias.unlink();shutil.copy2(target,alias)
    manifest=json.loads((models/'manifest.json').read_text(encoding='utf8'))
    assets=manifest['models']+manifest['vocabularies']+[manifest['portable_detector'],manifest['handwritten_calibration']]+manifest.get('portable_auxiliary',[])
    selected={'manifest.json'}|{entry['file'] for entry in assets}
    selected.update(str(path.relative_to(models)).replace('\\','/') for path in (models/'vocab').glob('metadata_*.json'))
    for relative in selected:
        source=(models/relative).resolve();destination=prefix/'models'/relative
        if not source.is_relative_to(models):raise SystemExit('Model path escapes its root')
        destination.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,destination)
    hashes={str(file.relative_to(prefix)).replace('\\','/'):hashlib.sha256(file.read_bytes()).hexdigest() for file in sorted(prefix.rglob('*')) if file.is_file() and file != prefix/'BUILD.json'}
    if any(Path(name).name in ('oneocr.dll','oneocr.onemodel') for name in hashes):raise SystemExit('Proprietary Windows OCR asset in staging prefix')
    source_files=[ROOT/'CMakeLists.txt',ROOT/'json.hpp',ROOT/'stb_image.h']+[file for file in (ROOT/'native').rglob('*') if file.is_file()]
    build={'parity':'incomplete','onnxruntime_commit':'a83fc4d58cb48eb68890dd689f94f28288cf2278','model_source_sha256':manifest['source_assets']['oneocr.onemodel'],'files':hashes,'source_files':{str(file.relative_to(ROOT)).replace('\\','/'):hashlib.sha256(file.read_bytes()).hexdigest() for file in source_files}}
    (prefix/'BUILD.json').write_text(json.dumps(build,indent=2),encoding='utf8')
    print('Staged local runtime:',prefix)
