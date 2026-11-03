"""Create a local source archive, excluding models and compiled binaries."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import zipfile

ROOT=Path(__file__).resolve().parents[1]
DIRECTORIES=('native','cmake','docs','licenses','research','scripts','src','web','test','examples','samples','reports')
EXTENSIONS={'.c','.cpp','.h','.hpp','.asm','.cmake','.json','.md','.txt','.log','.tsv','.py','.ps1','.java','.js','.mjs','.cjs','.mts','.html','.yml','.yaml','.png','.jpg','.jpeg','.bmp','.gif','.tif','.tiff','.webp','.rgb','.f32'}
ROOT_FILES=('.gitattributes','.gitignore','.npmignore','AGENTS.md','CMakeLists.txt','CMakePresets.json','README.md','ocr.cmd','ocr.sh',
            'THIRD_PARTY_NOTICES.md','package.json','package-lock.json','cordis.patch.yml',
            'ocr.cpp','oneocr_wrapper.cpp','oneocr_test.cpp','oneocr.h','json.hpp','stb_image.h',
            'ocr-book.jpg','test.png')
FORBIDDEN={'.dll','.exe','.so','.dylib','.wasm','.onnx','.onemodel','.a','.lib','.o','.obj'}


def report_entries(index_name, content):
    """Resolve sealed report entries without allowing paths outside their report."""
    index=json.loads(content)
    if not isinstance(index,dict):raise ValueError('Invalid report index: '+index_name)
    for name,record in index.items():
        relative=PurePosixPath(name) if isinstance(name,str) else None
        if (not name or relative is None or relative.is_absolute() or '\\' in name or
                '..' in relative.parts or str(relative)!=name or name=='.'):
            raise ValueError('Invalid indexed report path: '+str(name))
        if not isinstance(record,dict):raise ValueError('Invalid indexed report record: '+name)
        # Existing sealed reports use two explicit schemas. Preserve both byte
        # counts, and reject ambiguous records rather than rewriting history.
        lengths=[record[k] for k in ('size','bytes') if k in record]
        if (not isinstance(record.get('sha256'),str) or not re.fullmatch('[0-9a-f]{64}',record['sha256']) or
                not lengths or any(type(n) is not int or n<0 for n in lengths) or len(set(lengths))!=1):
            raise ValueError('Invalid indexed report digest/size: '+name)
        if relative.suffix.lower() in FORBIDDEN:
            raise ValueError('Compiled/model asset in source report: '+name)
        yield (PurePosixPath(index_name).parent/relative).as_posix(),{'sha256':record['sha256'],'size':lengths[0]}


def indexed_report_files():
    files=set()
    for index in (ROOT/'reports').rglob('FILES.sha256.json'):
        for name,record in report_entries(index.relative_to(ROOT).as_posix(),index.read_text(encoding='utf8')):
            path=ROOT/name
            if not path.resolve().is_relative_to(index.parent.resolve()) or path.is_symlink() or not path.is_file():
                raise ValueError('Missing or unsafe indexed report file: '+name)
            if path.stat().st_size!=record['size'] or hashlib.sha256(path.read_bytes()).hexdigest()!=record['sha256']:
                raise ValueError('Indexed report content changed: '+name)
            files.add(path)
    return files


def verify_source_archive(path):
    with zipfile.ZipFile(path) as archive:
        names=archive.namelist();manifest=json.loads(archive.read('SOURCE_BUILD.json'))
        if len(set(names))!=len(names) or set(names)!=set(manifest['files'])|{'SOURCE_BUILD.json'}:
            raise ValueError('Source archive inventory differs from its manifest')
        for name,expected in manifest['files'].items():
            if name.startswith(('models/','artifacts/')) or PurePosixPath(name).suffix.lower() in FORBIDDEN:
                raise ValueError('Compiled/model asset in source archive: '+name)
            if hashlib.sha256(archive.read(name)).hexdigest()!=expected:
                raise ValueError('Source archive digest differs: '+name)
        indexes=[n for n in names if n.startswith('reports/') and PurePosixPath(n).name=='FILES.sha256.json']
        entries=0
        for index in indexes:
            for name,record in report_entries(index,archive.read(index).decode('utf8')):
                if name not in manifest['files']:
                    raise ValueError('Missing indexed report file in source archive: '+name)
                data=archive.read(name)
                if len(data)!=record['size'] or hashlib.sha256(data).hexdigest()!=record['sha256']:
                    raise ValueError('Indexed report content differs in source archive: '+name)
                entries+=1
    return {'passed':True,'source_files':len(manifest['files']),'report_indexes':len(indexes),'indexed_report_files':entries}

def package(destination):
    destination=destination.resolve()
    if not destination.is_relative_to((ROOT/'artifacts').resolve()):
        raise ValueError('Source archive must be written under project artifacts/')
    files={ROOT/name for name in ROOT_FILES}
    for directory in DIRECTORIES:
        files.update(p for p in (ROOT/directory).rglob('*')
                     if p.is_file() and not p.is_symlink() and p.suffix in EXTENSIONS
                     and '__pycache__' not in p.parts)
    # A sealed report is indivisible. Include every indexed evidence file,
    # including compressed logs and future data extensions, before writing.
    files.update(indexed_report_files())
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
    verification=verify_source_archive(destination)
    print(json.dumps({'archive':str(destination),'source_files':len(hashes),'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'parity':'incomplete','verification':verification}))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    options=parser.add_mutually_exclusive_group()
    options.add_argument('--output',type=Path)
    options.add_argument('--verify',type=Path,help='Read-only validation, including every sealed report index')
    args=parser.parse_args()
    if args.verify:print(json.dumps(verify_source_archive(args.verify)))
    else:package(args.output or ROOT/'artifacts/oneocr-source.zip')
