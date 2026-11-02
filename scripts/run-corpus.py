"""Run each backend separately; record wall time and Windows peak working set."""
import argparse
import ctypes as C
import json
import os
from pathlib import Path
import subprocess
import sys
import time

ROOT=Path(__file__).resolve().parents[1]
CASE=ROOT/'work/oneocr-portable'

def run(command,cwd):
    started=time.perf_counter();peak=None
    process=subprocess.Popen(command,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if os.name=='nt':
        class Memory(C.Structure):
            _fields_=[('cb',C.c_ulong),('faults',C.c_ulong)]+[(x,C.c_size_t) for x in ['peak_working','working','peak_paged','paged','peak_nonpaged','nonpaged','pagefile','peak_pagefile']]
        dll=C.WinDLL('psapi');dll.GetProcessMemoryInfo.argtypes=[C.c_void_p,C.POINTER(Memory),C.c_ulong]
        while process.poll() is None:
            if time.perf_counter()-started>120:
                process.kill()
                process.communicate()
                return {'exit_code':-1,'seconds':time.perf_counter()-started,'peak_working_set_bytes':peak,'stderr':'OCR timed out after 120 seconds'}
            memory=Memory();memory.cb=C.sizeof(memory)
            if dll.GetProcessMemoryInfo(int(process._handle),C.byref(memory),C.sizeof(memory)):
                peak=max(peak or 0,memory.peak_working)
            time.sleep(.02)
    elif sys.platform=='linux':
        while process.poll() is None:
            if time.perf_counter()-started>120:
                process.kill();process.communicate()
                return {'exit_code':-1,'seconds':time.perf_counter()-started,'peak_working_set_bytes':peak,'stderr':'OCR timed out after 120 seconds'}
            try:
                status=Path(f'/proc/{process.pid}/status').read_text()
                for line in status.splitlines():
                    if line.startswith('VmHWM:'):peak=max(peak or 0,int(line.split()[1])*1024)
            except (FileNotFoundError,ProcessLookupError):pass
            time.sleep(.02)
    stdout,stderr=process.communicate(timeout=120)
    return {'exit_code':process.returncode,'seconds':time.perf_counter()-started,'peak_working_set_bytes':peak,'stderr':stderr.decode('utf8',errors='replace')[-2000:]}

if __name__=='__main__':
    sys.stdout.reconfigure(encoding='utf8')
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--backend',choices=['legacy','native'],required=True)
    p.add_argument('--binary',type=Path)
    p.add_argument('--limit',type=int)
    p.add_argument('--output-dir',type=Path,help='Keep a new phase separate from existing evidence')
    p.add_argument('--models',type=Path,default=ROOT/'models')
    p.add_argument('--legacy-binary-dir',type=Path,default=ROOT/'bin')
    args=p.parse_args()
    corpus=CASE/'corpus'
    if not (corpus/'manifest.json').is_file():corpus=ROOT/'research/case/corpus'
    fixtures=json.loads((corpus/'manifest.json').read_text(encoding='utf8'))['cases']
    if args.limit:fixtures=fixtures[:args.limit]
    binary=(args.binary or (ROOT/'bin/ocr.exe' if args.backend=='legacy' else ROOT/'build/portable/native/Release/ocr.exe')).resolve()
    output=(args.output_dir or CASE/('corpus-'+args.backend)).resolve();output.mkdir(parents=True,exist_ok=True)
    results=[]
    for entry in fixtures:
        if args.backend=='legacy':
            command=[sys.executable,'-X','utf8',str(ROOT/'scripts/run-legacy.py'),str(corpus/entry['file']),str(output/(entry['name']+'.json')),'--binary-dir',str(args.legacy_binary_dir.resolve())]
        else:
            command=[str(binary),str(corpus/entry['file']),'--output',str(output/(entry['name']+'.json')),'--quiet','--model-dir',str(args.models.resolve())]
        result=run(command,binary.parent);result['name']=entry['name'];result['runner']='original-wrapper-python-process' if args.backend=='legacy' else 'native-cli';results.append(result)
        print(entry['name'],result['exit_code'],round(result['seconds'],3),flush=True)
    (output/'measurements.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf8')
    raise SystemExit(0 if all(x['exit_code']==0 for x in results) else 1)
