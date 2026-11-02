"""Build ICU 74.2 dictionary/Unicode data from pinned source, then emit C.

Run on a Linux/macOS build host (WSL is supported). No library binary or OCR
model is downloaded. CMake compiles the generated data and ICU common sources
for the target, including Android and Windows; the host library is not linked.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,default=ROOT/'work/vendor/icu-source')
    parser.add_argument('--data',type=Path,help='Reuse data built from the pinned source (checksum verified)')
    parser.add_argument('--jobs',type=int,default=6)
    parser.add_argument('--fetch-only',action='store_true',help='Fetch and verify sources without generating host data')
    args=parser.parse_args();lock=json.loads((ROOT/'research/dependencies.json').read_text())['icu']
    source=args.source.resolve()
    if not source.exists():
        source.parent.mkdir(parents=True,exist_ok=True)
        subprocess.run(['git','-c','core.autocrlf=false','clone','--depth','1','--branch',lock['tag'],lock['repository'],str(source)],check=True)
    git=['git','-c','safe.directory='+str(source),'-C',str(source)]
    revision=subprocess.check_output(git+['rev-parse','HEAD'],text=True).strip()
    if revision!=lock['commit']:raise ValueError('ICU revision mismatch; existing source was not modified')
    if args.fetch_only:return
    if args.data:data=args.data.resolve()
    else:
        host={'Linux':'Linux/gcc','Darwin':'MacOSX'}.get(platform.system())
        if not host:raise SystemExit('Prepare ICU data on Linux/macOS or WSL, then pass ONEOCR_ICU_DATA_C to CMake')
        configure=source/'icu4c/source/runConfigureICU'
        if b'\r\n' in configure.read_bytes():raise ValueError('Use an LF source checkout; git -c core.autocrlf=false clone')
        build=ROOT/'work/icu-data-host';build.mkdir(parents=True,exist_ok=True)
        environment=dict(os.environ,LC_ALL='C',TZ='UTC')
        subprocess.run([str(configure),host,'--enable-static','--disable-shared','--disable-tests','--disable-samples','--with-data-packaging=static'],cwd=build,env=environment,check=True)
        subprocess.run(['make','-j'+str(args.jobs)],cwd=build,env=environment,check=True)
        data=build/'data/out/tmp/icudt74l.dat'
    payload=data.read_bytes();digest=hashlib.sha256(payload).hexdigest()
    if digest!=lock['data_sha256']:raise ValueError('ICU source data checksum mismatch: '+digest)
    # Same aligned C structure as ICU genccode; buffered output avoids NTFS's
    # expensive per-byte writes when code generation runs under WSL.
    output=ROOT/'work/icu-generated/icudt74l_dat.c';output.parent.mkdir(parents=True,exist_ok=True)
    with output.open('w',encoding='ascii',newline='\n') as stream:
        stream.write('/* Generated from ICU 74.2 source data; Unicode-3.0. */\n#include "unicode/umachine.h"\nU_CDECL_BEGIN\nconst struct { double alignment; uint8_t bytes['+str(len(payload))+']; } icudt74_dat={0.0,{\n')
        for offset in range(0,len(payload),4096):
            block=payload[offset:offset+4096]
            stream.write(',\n'.join(','.join(str(x) for x in block[i:i+32]) for i in range(0,len(block),32))+',\n')
        stream.write('}};\nU_CDECL_END\n')
    (output.parent/'provenance.json').write_text(json.dumps({'icu_commit':revision,'data_sha256':digest,'generated_c_sha256':hashlib.sha256(output.read_bytes()).hexdigest()},indent=2))
    print(output)

if __name__=='__main__':main()
