"""Fetch the version-locked source used for the native image normalizer.

No models or runtime binaries are downloaded. Existing checkouts are verified
and are never reset or replaced.
"""
import json
import os
from pathlib import Path
import subprocess

root=Path(__file__).resolve().parents[1]
lock=json.loads((root/'research/dependencies.json').read_text())['opencv']
target=root/'work/vendor/opencv'
if not target.exists():
    target.parent.mkdir(parents=True,exist_ok=True)
    subprocess.run(['git','clone','--depth','1','--branch',lock['version'],
                    lock['repository'],str(target)],check=True,
                   env=dict(os.environ,GIT_LFS_SKIP_SMUDGE='1'))
revision=subprocess.check_output(['git','-C',str(target),'rev-parse','HEAD'],text=True).strip()
if revision!=lock['commit']:raise SystemExit('OpenCV source revision mismatch; existing source was not modified')
print(target)
