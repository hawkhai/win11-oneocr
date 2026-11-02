"""Fetch pinned ORT source, or a checksum-verified optional CPU SDK."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tarfile
import urllib.request
import zipfile

ROOT=Path(__file__).resolve().parents[1]

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    group=p.add_mutually_exclusive_group(required=True)
    group.add_argument('--source',action='store_true')
    group.add_argument('--sdk',choices=['windows-x64','linux-x64','linux-arm64','macos-x64','macos-arm64'])
    args=p.parse_args();lock=json.loads((ROOT/'research/dependencies.json').read_text())['onnxruntime']
    vendor=ROOT/'work/vendor';vendor.mkdir(parents=True,exist_ok=True)
    if args.source:
        target=vendor/'onnxruntime'
        if not target.exists():
            env=dict(os.environ,GIT_LFS_SKIP_SMUDGE='1')
            subprocess.run(['git','-c','core.autocrlf=false','clone','--depth','1','--branch','v'+lock['version'],lock['repository'],str(target)],check=True,env=env)
        revision=subprocess.check_output(['git','-c','safe.directory='+str(target),'-C',str(target),'rev-parse','HEAD'],text=True).strip()
        if revision!=lock['commit']:raise SystemExit('ORT source revision mismatch; existing source was not modified')
        print(target)
    else:
        record=lock['archives'][args.sdk];archive=vendor/record['file']
        if not archive.exists():urllib.request.urlretrieve(lock['release_base']+record['file'],archive)
        if hashlib.sha256(archive.read_bytes()).hexdigest()!=record['sha256']:raise SystemExit('ORT SDK archive SHA-256 mismatch')
        output=vendor/('sdk-'+args.sdk);output.mkdir(exist_ok=True)
        if archive.suffix=='.zip':
            with zipfile.ZipFile(archive) as z:
                for name in z.namelist():
                    path=Path(name)
                    if path.is_absolute() or '..' in path.parts:raise SystemExit('Unsafe SDK archive path')
                z.extractall(output)
        else:
            with tarfile.open(archive) as t:t.extractall(output,filter='data')
        roots=list(output.glob('onnxruntime-*'))
        if len(roots)!=1:raise SystemExit('Unexpected SDK directory layout')
        print(roots[0])
