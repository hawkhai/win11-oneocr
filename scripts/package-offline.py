"""Refresh local Git/source transfer packages from a clean committed checkout.

Packages contain no new network downloads and are not uploaded. The Git bundle
includes this branch's committed models/runtime assets with their notices.
The source ZIP excludes model weights and compiled binaries. Output packages
are ignored build products: committing a bundle containing its own commit is
not possible. OFFLINE_PACKAGE.json binds both packages to the exact Git head.
"""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]


def git(*args):
    return subprocess.check_output(['git',*args],cwd=ROOT,text=True,encoding='utf8').strip()


def sha(path):
    digest=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):digest.update(block)
    return digest.hexdigest()


def main():
    if git('status','--porcelain','--untracked-files=all'):
        raise ValueError('Commit all source, sample, runtime and report changes before packaging')
    ref=git('symbolic-ref','HEAD');commit=git('rev-parse','HEAD')
    artifacts=ROOT/'artifacts';artifacts.mkdir(exist_ok=True)
    bundle=artifacts/'win11-oneocr-offline.bundle'
    subprocess.run(['git','bundle','create',str(bundle),ref],cwd=ROOT,check=True)
    verified=subprocess.run(['git','bundle','verify',str(bundle)],cwd=ROOT,check=True,capture_output=True,text=True,encoding='utf8')
    heads=git('bundle','list-heads',str(bundle))
    if heads!=commit+' '+ref:raise ValueError('Unexpected bundle head: '+heads)
    source=artifacts/'oneocr-source.zip'
    subprocess.run([sys.executable,ROOT/'scripts/package-source.py','--output',source],cwd=ROOT,check=True)
    if git('rev-parse','HEAD')!=commit or git('status','--porcelain','--untracked-files=all'):
        raise ValueError('Checkout changed while packaging; regenerate packages')
    files={}
    for path in [bundle,source]:
        digest=sha(path);files[path.name]={'sha256':digest,'size':path.stat().st_size}
        path.with_suffix(path.suffix+'.sha256').write_text(digest+'  '+path.name+'\n',encoding='utf8')
    record={'commit':commit,'ref':ref,'files':files,'bundle_verify':verified.stdout+verified.stderr,
            'parity':'incomplete','bundle_contains_committed_models':True,
            'source_zip_contains_models':False,'model_ownership':'Microsoft; existing notices retained; no additional redistribution permission asserted',
            'upload_performed':False}
    (artifacts/'OFFLINE_PACKAGE.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'commit':commit,'files':files},ensure_ascii=False))


if __name__=='__main__':main()
