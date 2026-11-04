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
import tempfile

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
    with tempfile.TemporaryDirectory(prefix='saved-delivery-review-',dir=artifacts) as temporary:
        review=Path(temporary)/'review'
        subprocess.run([sys.executable,ROOT/'reports/colleague-build-2026-11-04/evidence/review-build-results.py',
                        '--report',ROOT/'reports/colleague-build-2026-11-04','--output',review],cwd=ROOT,check=True)
        reviewed=json.loads((review/'verification.json').read_text(encoding='utf8'))
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
    bindings=json.loads((ROOT/'reports/colleague-build-2026-11-04/evidence/bindings.json').read_text(encoding='utf8'))
    policy=json.loads((ROOT/'research/acceptance-policy.json').read_text(encoding='utf8'))
    status={'format':'native-delivery-status','version':1,'project':'win11-oneocr','source_commit':commit,
            'scope':policy['completion_scope'],
            'acceptance':{'status':'complete','fresh_inference_for_refresh':False,
                          'saved_cases_verified':reviewed['passed_cases'],
                          'thresholds':policy['native_cpp']['maximum_errors']},
            'evidence':{'fresh_inference_report':'reports/colleague-build-2026-11-04',
                        'index_sha256':sha(ROOT/'reports/colleague-build-2026-11-04/FILES.sha256.json')},
            'runtime_provenance':{'latest_rebuilt_outputs':bindings['rebuilt_outputs'],
                                  'packaged_runtime_is_latest_rebuild':False,
                                  'packaged_runtime_manifest_sha256':sha(ROOT/'research/delivery-manifest.json'),
                                  'packaged_runtime_builds':{platform:json.loads((ROOT/'artifacts'/platform/'BUILD.json').read_text(encoding='utf8'))
                                                            for platform in ('win32-x64','linux-x64')}},
            'legacy_parity':'incomplete','browser_is_current_scope':False}
    status_file=artifacts/'DELIVERY_STATUS.json'
    status_file.write_text(json.dumps(status,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    record['delivery_status']={'path':status_file.name,'sha256':sha(status_file)}
    (artifacts/'OFFLINE_PACKAGE.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'commit':commit,'files':files},ensure_ascii=False))


if __name__=='__main__':main()
