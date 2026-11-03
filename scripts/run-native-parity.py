"""Run the complete current native scope against the active DLL baseline."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from regression_reference import active_reference


def read(path):return json.loads(path.read_text(encoding='utf8'))
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def write(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf8')


def main():
    sys.stdout.reconfigure(encoding='utf8')
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--binary',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    args=p.parse_args();output=args.output.resolve();binary=args.binary.resolve()
    if output.exists() and any(output.iterdir()):raise ValueError('Use a fresh output directory')
    output.mkdir(parents=True,exist_ok=True)
    reference=active_reference();model=read(ROOT/'models/manifest.json');model_hash=sha(ROOT/'models/manifest.json')
    if reference['model_source_sha256']!=model['source_assets']['oneocr.onemodel']:
        raise ValueError('Model and active DLL reference differ')
    subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/run-all-samples.py','--inventory',ROOT/'samples/manifest.json',
                    '--baseline',ROOT/reference['all_samples_baseline'],'--binary',binary,'--output',output/'all-samples'],check=True,cwd=ROOT)
    subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/run-parity-suite.py','--manifest',ROOT/'research/case/corpus/manifest.json',
                    '--baseline',ROOT/reference['corpus_baseline'],'--binary',binary,'--output',output/'corpus'],check=True,cwd=ROOT)
    spec=importlib.util.spec_from_file_location('runner',ROOT/'scripts/run-corpus.py');runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
    book=output/'book.json'
    measurement=runner.run([str(binary),str(ROOT/'ocr-book.jpg'),'--model-dir',str(ROOT/'models'),'--quiet','--output',str(book)],binary.parent)
    write(output/'book-measurement.json',measurement)
    if measurement['exit_code']:raise RuntimeError('Book failed: '+measurement['stderr'])
    subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/compare-results.py',ROOT/reference['book_baseline'],book,'--output',output/'book.parity.json'],check=True,cwd=ROOT)
    samples=read(output/'all-samples/suite.json');corpus=read(output/'corpus/suite.json');book_report=read(output/'book.parity.json')
    options={'passed':True,'total_cases':0,'passed_cases':0}
    if 'parameter_manifest' in reference:
        subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/run-parameter-parity.py','--binary',binary,'--output',output/'options'],check=True,cwd=ROOT)
        options=read(output/'options/suite.json')
    if model_hash!=sha(ROOT/'models/manifest.json'):raise ValueError('Model manifest changed during regression')
    sample_count=len(read(ROOT/'samples/manifest.json')['cases']);corpus_count=len(read(ROOT/'research/case/corpus/manifest.json')['cases'])
    passed=samples['passed'] and corpus['passed'] and book_report['passed'] and options['passed'] and samples['completed_cases']==sample_count and corpus['total_cases']==corpus_count
    summary={'passed':passed,'full_regression':True,'cases':sample_count+corpus_count+1+options['total_cases'],'passed_cases':samples['passed_cases']+corpus['passed_cases']+int(book_report['passed'])+options['passed_cases'],
             'recognition_output_cases':samples['recognition_passes']+corpus['strict_recognition_passes']+int(book_report['passed'])+options['passed_cases'],
             'expected_rejections':samples['passed_cases']-samples['recognition_passes'],
             'comparator_sha256':sha(ROOT/'scripts/compare-results.py'),'model_manifest_sha256':model_hash,
             'model_source_sha256':reference['model_source_sha256'],'binary_sha256':sha(binary),'reference':reference}
    write(output/'summary.json',summary)
    print(json.dumps(summary,ensure_ascii=False));return 0 if passed else 1


if __name__=='__main__':sys.exit(main())
