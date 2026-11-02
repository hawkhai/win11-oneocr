"""Forward build arguments to the pinned project-local ONNX Runtime source.

Usage: python scripts/build-ort.py --config Release --build_shared_lib ...
Works with the isolated Windows research Python as well as regular Python.
"""
import runpy
import subprocess
import sys
from pathlib import Path

root=Path(__file__).resolve().parents[1]
source=root/'work/vendor/onnxruntime'
expected='a83fc4d58cb48eb68890dd689f94f28288cf2278'
actual=subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip()
if actual!=expected:
    raise SystemExit(f'ORT source revision mismatch: {actual}')
entry=source/'tools/ci_build/build.py'
sys.path.insert(0,str(entry.parent))
sys.argv[0]=str(entry)
runpy.run_path(str(entry),run_name='__main__')
