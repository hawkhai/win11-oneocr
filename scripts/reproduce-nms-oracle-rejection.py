"""Linux/Ninja: reproduce default NMS test rejection using an isolated oracle copy.

Reuses existing native test objects, compiling only the current test source with
an alternate oracle directory. No tracked oracle, source or build output changes.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'build/linux-x64');p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    if sys.platform!='linux':raise ValueError('This diagnostic requires the Linux Ninja build; normal CTest negatives run on both hosts')
    work=a.output.resolve();build=a.build.resolve()
    if work.exists():raise ValueError('Use a fresh output directory')
    if not work.is_relative_to(ROOT/'work') or work==ROOT/'work':raise ValueError('Diagnostic output must be a new directory below work/')
    work.mkdir(parents=True);oracles=work/'oracles';shutil.copytree(ROOT/'research/options-handwriting/oracles',oracles)
    commands=subprocess.check_output(['ninja','-C',str(build),'-t','commands','oneocr_nms_test'],text=True).splitlines()
    compile_line=next(c for c in commands if '-c ' in c and c.endswith('/native/tests/nms_test.cpp'))
    args=shlex.split(compile_line);obj=work/'nms-test.o';exe=work/'nms-test';old_obj=args[args.index('-o')+1];args[args.index('-o')+1]=str(obj)
    if '-MF' in args:args[args.index('-MF')+1]=str(work/'nms-test.d')
    args=[f'-DONEOCR_OPTIONS_ORACLES="{oracles}"' if x.startswith('-DONEOCR_OPTIONS_ORACLES=') else x for x in args]
    subprocess.run(args,cwd=build,check=True)
    link=next(c for c in reversed(commands) if ' -o bin/oneocr_nms_test ' in c)
    if link.startswith(': && '):link=link[5:]
    if link.endswith(' && :'):link=link[:-5]
    args=shlex.split(link);args=[str(obj) if x==old_obj else x for x in args];args[args.index('-o')+1]=str(exe)
    args=[f'-Wl,--dependency-file={work}/nms-link.d' if x.startswith('-Wl,--dependency-file=') else x for x in args]
    subprocess.run(args,cwd=build,check=True)
    baseline=subprocess.run([str(exe)],capture_output=True,text=True)
    if baseline.returncode:raise ValueError('Unmodified oracle must pass: '+baseline.stdout+baseline.stderr)
    chosen=None
    for file in sorted(oracles.glob('*.json')):
        data=json.loads(file.read_text())
        for call in data.get('nms',[]):
            if call['pairs']:
                pair=call['pairs'][0];before=pair['intersection'];pair['intersection']=before+1e9
                file.write_text(json.dumps(data));chosen={'file':file.name,'expected_before':before,'expected_mutated':pair['intersection']};break
        if chosen:break
    if chosen is None:raise ValueError('No real Photos pair was exercised')
    mutated=subprocess.run([str(exe)],capture_output=True,text=True)
    passed=mutated.returncode==1 and 'Intersection mismatch Photos ' in mutated.stderr
    report={'passed':passed,'baseline_exit_code':baseline.returncode,'baseline_stdout':baseline.stdout,'mutation':chosen,
            'mutated_exit_code':mutated.returncode,'mutated_stdout':mutated.stdout,'mutated_stderr':mutated.stderr,
            'tracked_source_or_assets_modified':False,'reproducer_normalized_sha256':hashlib.sha256(Path(__file__).read_bytes().replace(b'\r\n',b'\n')).hexdigest()}
    (work/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(report))
    return 0 if passed else 1

if __name__=='__main__':sys.exit(main())
