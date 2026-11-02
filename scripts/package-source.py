"""Create a local source archive, excluding models and compiled binaries."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

ROOT=Path(__file__).resolve().parents[1]
DIRECTORIES=('native','cmake','docs','licenses','research','scripts','src','test','samples','reports/oneocr-all-samples-2026-11-02')
EXTENSIONS={'.cpp','.h','.hpp','.cmake','.json','.md','.txt','.py','.ps1','.java','.js','.mjs','.yml','.yaml','.png','.jpg','.jpeg','.bmp','.gif','.tif','.tiff','.webp'}
ROOT_FILES=('.gitignore','.npmignore','CMakeLists.txt','CMakePresets.json','README.md',
            'THIRD_PARTY_NOTICES.md','package.json','package-lock.json','cordis.patch.yml',
            'ocr.cpp','oneocr_wrapper.cpp','oneocr_test.cpp','oneocr.h','json.hpp','stb_image.h',
            'ocr-book.jpg','test.png')

def package(destination):
    destination=destination.resolve()
    if not destination.is_relative_to((ROOT/'artifacts').resolve()):
        raise ValueError('Source archive must be written under project artifacts/')
    files={ROOT/name for name in ROOT_FILES}
    for directory in DIRECTORIES:
        files.update(p for p in (ROOT/directory).rglob('*')
                     if p.is_file() and not p.is_symlink() and p.suffix in EXTENSIONS
                     and '__pycache__' not in p.parts)
    for path in files:
        if not path.resolve().is_relative_to(ROOT) or not path.is_file() or path.is_symlink():
            raise ValueError('Missing or unsafe source file: '+str(path))
    hashes={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(files)}
    manifest={'parity':'incomplete','contains_models':False,'contains_runtime_binaries':False,'files':hashes}
    destination.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(destination,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
        for name in sorted(hashes):
            info=zipfile.ZipInfo(name,date_time=(2026,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
            info.external_attr=0o100644<<16
            archive.writestr(info,(ROOT/name).read_bytes())
        info=zipfile.ZipInfo('SOURCE_BUILD.json',date_time=(2026,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
        info.external_attr=0o100644<<16
        archive.writestr(info,json.dumps(manifest,ensure_ascii=False,indent=2).encode('utf8'))
    print(json.dumps({'archive':str(destination),'source_files':len(hashes),'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'parity':'incomplete'}))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'artifacts/oneocr-source.zip')
    package(parser.parse_args().output)
