"""Inventory every sample image and compare against isolated, hash-bound DLL runs."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXTENSIONS = {'.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tif', '.tiff', '.webp'}

def module(name, filename):
    spec = importlib.util.spec_from_file_location(name, ROOT/'scripts'/filename)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2)+'\n', encoding='utf8')

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--inventory', type=Path, required=True)
    p.add_argument('--refresh-inventory', action='store_true')
    p.add_argument('--freeze-dll', action='store_true')
    p.add_argument('--baseline', type=Path, required=True)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    files = sorted(x for x in (ROOT/'samples').rglob('*') if x.suffix.lower() in EXTENSIONS)
    cases = [{'name': hashlib.sha256(x.relative_to(ROOT/'samples').as_posix().encode()).hexdigest()[:16],
              'file': x.relative_to(ROOT/'samples').as_posix(), 'sha256': digest(x)} for x in files]
    if args.refresh_inventory:
        from PIL import Image
        for case, path in zip(cases, files):
            with Image.open(path) as im:
                case['width'], case['height'] = im.size
                case['pixel_mode'] = im.mode
        write(args.inventory, {'schema_version': 1, 'scope': 'Every image recursively below samples, including references and thumbnails', 'cases': cases})
    inventory = json.loads(args.inventory.read_text(encoding='utf8'))
    if [{k:c[k] for k in ('name','file','sha256')} for c in inventory['cases']] != [{k:c[k] for k in ('name','file','sha256')} for c in cases]:
        raise ValueError('Sample inventory changed: refresh explicitly; no image may be omitted')
    runner = module('sample_runner', 'run-corpus.py')
    comparator = module('sample_comparator', 'compare-results.py')
    args.output.mkdir(parents=True, exist_ok=True)
    binary = args.binary.resolve()
    def runtime_bindings():
        paths={binary}
        for pattern in ('*oneocr_portable*','*onnxruntime*'):
            paths.update(x for x in binary.parent.glob(pattern) if x.is_file() and x.suffix.lower() not in ('.lib','.exp','.pdb'))
        return {x.name:digest(x) for x in sorted(paths)}
    native_bindings=runtime_bindings()
    bindings = {x: digest(ROOT/'bin'/x) for x in ('oneocr.dll','oneocr.onemodel','onnxruntime.dll','oneocr_wrapper.dll')} if args.freeze_dll else None
    results = []
    for case in inventory['cases']:
        name, source = case['name'], ROOT/'samples'/case['file']
        golden, contract_path = args.baseline/(name+'.json'), args.baseline/(name+'.contract.json')
        if args.freeze_dll and not contract_path.exists():
            measurement = runner.run([sys.executable, str(ROOT/'scripts/run-legacy.py'), str(source), str(golden.resolve())], ROOT)
            write(contract_path, {'file':case['file'], 'image_sha256':case['sha256'], 'dll_bindings':bindings, 'measurement':measurement,
                                 'result_sha256': digest(golden) if golden.exists() else None})
        contract = json.loads(contract_path.read_text(encoding='utf8'))
        if bindings and contract['dll_bindings'] != bindings:
            raise ValueError('Original DLL/model/wrapper binding changed: '+case['file'])
        if contract['image_sha256'] != case['sha256'] or (contract['result_sha256'] and digest(golden) != contract['result_sha256']):
            raise ValueError('Frozen baseline fingerprint mismatch: '+case['file'])
        destination = (args.output/'native'/(name+'.json')).resolve()
        destination.parent.mkdir(exist_ok=True)
        if destination.exists(): destination.unlink()
        measurement = runner.run([str(binary),str(source),'--model-dir',str(ROOT/'models'),'--quiet','--output',str(destination)],binary.parent)
        entry = {'file':case['file'], 'name':name, 'passed':False, 'measurement':measurement}
        if contract['measurement']['exit_code'] == 0 and measurement['exit_code'] == 0:
            report = comparator.compare(json.loads(golden.read_text(encoding='utf8')),json.loads(destination.read_text(encoding='utf8')))
            write(args.output/(name+'.parity.json'), report)
            entry.update({k:v for k,v in report.items() if k!='failures'})
            entry['status'] = 'passed' if report['passed'] else 'mismatch'
        elif (contract['measurement']['exit_code'] == 1 and 'Original wrapper OCR failed: -18' in contract['measurement']['stderr']
              and (case['width'] < 50 or case['height'] < 50 or case['width'] > 10000 or case['height'] > 10000)):
            entry['passed'] = measurement['exit_code'] == 1 and measurement['stderr'].replace('\r\n','\n') == 'ERROR: Image dimensions must be 50..10000 pixels\n' and not destination.exists()
            entry['status'] = 'expected_size_error' if entry['passed'] else 'error_contract_mismatch'
        elif (contract['measurement']['exit_code'] == 1 and 'Original wrapper OCR failed: -15' in contract['measurement']['stderr']
              and case['pixel_mode'] == 'LA'):
            entry['passed'] = measurement['exit_code'] == 1 and measurement['stderr'].replace('\r\n','\n') == 'ERROR: Unsupported image channel count\n' and not destination.exists()
            entry['status'] = 'expected_channel_error' if entry['passed'] else 'error_contract_mismatch'
        else:
            entry['status'] = 'process_failed'
            entry['original_measurement'] = contract['measurement']
        results.append(entry)
        print(case['file'], entry['status'], flush=True)
        write(args.output/'suite.json', {'passed':all(x['passed'] for x in results), 'completed_cases':len(results), 'total_cases':len(cases),
              'passed_cases':sum(x['passed'] for x in results), 'recognition_passes':sum(x['status']=='passed' for x in results),
              'error_contract_passes':sum(x['status'] in ('expected_size_error','expected_channel_error') for x in results),
              'binary_sha256':native_bindings[binary.name], 'runtime_bindings':native_bindings, 'inventory_sha256':digest(args.inventory), 'cases':results})
    if runtime_bindings()!=native_bindings:raise ValueError('Native runtime changed during suite execution')
    return 0 if all(x['passed'] for x in results) else 1

if __name__ == '__main__':
    sys.exit(main())
