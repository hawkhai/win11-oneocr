"""Run the native CLI inside a Linux network namespace with no network route.

Invoke using: sudo unshare -n python3 scripts/verify-offline.py --binary ...
This script deliberately refuses to claim network isolation on Windows.
"""
import argparse
import json
from pathlib import Path
import socket
import subprocess
import sys
import time

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--binary',type=Path,required=True)
    p.add_argument('--models',type=Path,required=True)
    p.add_argument('--image',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    args=p.parse_args()
    if sys.platform!='linux':raise SystemExit('Linux network namespace required')
    import resource
    if Path('/proc/self/ns/net').readlink()==Path('/proc/1/ns/net').readlink():raise SystemExit('No distinct network namespace; use unshare -n')
    with socket.socket() as connection:
        connection.settimeout(.5)
        try:connection.connect(('1.1.1.1',443))
        except OSError as error:network_error=str(error)
        else:raise SystemExit('Unexpected outbound network access')
    args.output.parent.mkdir(parents=True,exist_ok=True)
    command=[str(args.binary.resolve()),str(args.image.resolve()),'--model-dir',str(args.models.resolve()),'--output',str(args.output.resolve()),'--quiet']
    started=time.perf_counter()
    process=subprocess.Popen(command,cwd=args.binary.resolve().parent,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    mappings=set()
    while process.poll() is None:
        try:
            mappings.update(line.split()[-1] for line in Path(f'/proc/{process.pid}/maps').read_text().splitlines() if '/' in line)
        except (FileNotFoundError,ProcessLookupError):pass
        if time.perf_counter()-started>120:
            process.kill();raise RuntimeError('Native OCR timed out')
        time.sleep(.01)
    stdout,stderr=process.communicate()
    forbidden=[name for name in mappings if any(token in Path(name).name.lower() for token in ('oneocr.dll','onnxruntime.dll','wine'))]
    report={'passed':process.returncode==0 and not forbidden,'exit_code':process.returncode,'network_error':network_error,'mapped_files':sorted(mappings),'forbidden_modules':forbidden,'seconds':time.perf_counter()-started,'peak_working_set_bytes':resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss*1024,'stderr':stderr.decode('utf8',errors='replace'),'note':'Only source-built native OCR plus Linux ORT are executed; Python is the test harness, not the OCR backend.'}
    if args.output.exists():
        result=json.loads(args.output.read_text(encoding='utf8'));report['line_count']=result['line_count'];report['parity']=result.get('engine',{}).get('parity')
    args.output.with_suffix('.verification.json').write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps({k:v for k,v in report.items() if k!='mapped_files'},ensure_ascii=False))
    raise SystemExit(0 if report['passed'] else 1)
