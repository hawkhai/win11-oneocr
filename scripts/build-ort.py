"""Forward build arguments to the pinned project-local ONNX Runtime source.

Usage: python scripts/build-ort.py --config Release --build_shared_lib ...
Works with the isolated Windows research Python as well as regular Python.
"""
import runpy
import subprocess
import sys
import os
from pathlib import Path

root=Path(__file__).resolve().parents[1]
source=root/'work/vendor/onnxruntime'
expected='a83fc4d58cb48eb68890dd689f94f28288cf2278'
actual=subprocess.check_output(['git','-c','safe.directory='+str(source),'-C',str(source),'rev-parse','HEAD'],text=True).strip()
if actual!=expected:
    raise SystemExit(f'ORT source revision mismatch: {actual}')
# GCC 15 no longer gets uint32_t transitively through <regex>. Apply only
# the missing standard include, checking against the pinned upstream file
# before touching the checkout. Preserve an existing checkout's line endings.
relative='onnxruntime/core/common/semver.h'
upstream=subprocess.check_output(['git','-c','safe.directory='+str(source),'-C',str(source),'show','HEAD:'+relative]).replace(b'\r\n',b'\n')
anchor=b'#include <string_view>'
if anchor not in upstream:raise SystemExit('Unexpected pinned semver header')
patched=upstream.replace(anchor,b'#include <cstdint>\n'+anchor,1)
header=source/relative;payload=header.read_bytes();normalized=payload.replace(b'\r\n',b'\n')
if normalized not in (upstream,patched):raise SystemExit('Preserving unexpected local changes in ORT semver.h; cannot apply the compatibility include')
if normalized==upstream:
    newline=b'\r\n' if b'\r\n' in payload else b'\n'
    header.write_bytes(payload.replace(anchor,b'#include <cstdint>'+newline+anchor,1))
print('ORT source adjustment: semver.h includes <cstdint> explicitly (GCC 15 compatibility)',flush=True)
# Trust a Windows-owned WSL checkout for this child build only.
count=int(os.environ.get('GIT_CONFIG_COUNT','0'));os.environ['GIT_CONFIG_COUNT']=str(count+1)
os.environ['GIT_CONFIG_KEY_'+str(count)]='safe.directory';os.environ['GIT_CONFIG_VALUE_'+str(count)]=str(source)
if os.name!='nt':
    # The original /fp:precise build rounds the QLinearAdd fixed term
    # before its explicit FMA operations. GCC's implicit contraction can
    # otherwise flip an exact half-integer and change detector crops.
    if '--cmake_extra_defines' not in sys.argv:sys.argv.append('--cmake_extra_defines')
    position=sys.argv.index('--cmake_extra_defines')+1
    for key in ('CMAKE_CXX_FLAGS','CMAKE_C_FLAGS'):
        existing=next((i for i in range(position,len(sys.argv))if sys.argv[i].startswith(key+'=')),None)
        if existing is None:sys.argv.insert(position,key+'=-ffp-contract=off')
        elif '-ffp-contract=off' not in sys.argv[existing]:sys.argv[existing]+=' -ffp-contract=off'
entry=source/'tools/ci_build/build.py'
sys.path.insert(0,str(entry.parent))
sys.argv[0]=str(entry)
runpy.run_path(str(entry),run_name='__main__')
