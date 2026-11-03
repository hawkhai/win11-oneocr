"""Compare actual WebAssembly inference with the active, hash-bound Microsoft DLL baseline.

All samples, the fixed 27-case corpus, and the book run by default. Thresholds
come directly from compare-results.py; rejected originals remain error cases.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'scripts'))
from regression_reference import active_reference


def read(path):
    return json.loads(path.read_text(encoding='utf8'))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2)+'\n', encoding='utf8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node', default='node')
    parser.add_argument('--jobs', type=int, default=4, help='Independent inference processes (default: 4)')
    parser.add_argument('--schedule-from', type=Path, help='Use previous same-input timings only to balance process workloads')
    parser.add_argument('--output', type=Path, default=ROOT/'work/wasm-parity')
    parser.add_argument('--only', help='Diagnostic subset: exact case name; never marked as a full regression')
    parser.add_argument('--compare-only', action='store_true', help='Compare already generated outputs')
    parser.add_argument('--browser', type=Path, help='Use headless Chromium at this executable path instead of Node Wasm')
    parser.add_argument('--browser-timeout-ms', type=int, default=1800000, help='Test-only request budget; does not alter product defaults or comparison tolerances')
    parser.add_argument('--playwright-path', type=Path, help='Optional installed playwright-core index.mjs for browser tests')
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    if args.browser_timeout_ms < 1:
        parser.error('--browser-timeout-ms must be positive')
    build = read(ROOT/'artifacts/wasm32/BUILD.json')
    for name, expected in build['files'].items():
        if sha(ROOT/'artifacts/wasm32'/name) != expected:
            raise ValueError('Wasm delivery asset changed: '+name)
    for name, expected in build['source_files'].items():
        if hashlib.sha256((ROOT/name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() != expected:
            raise ValueError('Wasm build is stale for source: '+name)
    reference = active_reference()
    model = read(ROOT/'models/manifest.json')
    if reference['model_source_sha256'] != model['source_assets']['oneocr.onemodel']:
        raise ValueError('Model and frozen reference differ')
    cases = []
    inventory = read(ROOT/'samples/manifest.json')
    images = sorted(p.relative_to(ROOT/'samples').as_posix() for p in (ROOT/'samples').rglob('*')
                    if p.suffix.lower() in {'.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tif', '.tiff', '.webp'})
    if images != [item['file'] for item in inventory['cases']]:
        raise ValueError('Sample inventory changed; no omissions allowed')
    for item in inventory['cases']:
        contract = read(ROOT/reference['all_samples_baseline']/(item['name']+'.contract.json'))
        golden = ROOT/reference['all_samples_baseline']/(item['name']+'.json')
        path = ROOT/'samples'/item['file']
        if sha(path) != item['sha256'] or contract['image_sha256'] != item['sha256']:
            raise ValueError('Sample fingerprint changed: '+str(path))
        if contract['result_sha256'] and sha(golden) != contract['result_sha256']:
            raise ValueError('Baseline fingerprint changed: '+str(golden))
        error = None
        if contract['measurement']['exit_code']:
            message = contract['measurement']['stderr']
            if 'Original wrapper OCR failed: -18' in message and not (50 <= item['width'] <= 10000 and 50 <= item['height'] <= 10000):
                error = {'code': -18, 'message': 'Image dimensions must be 50..10000 pixels'}
            elif 'Original wrapper OCR failed: -15' in message and item['pixel_mode'] == 'LA':
                error = {'code': -15, 'message': 'Unsupported image channel count'}
            else:
                raise ValueError('Unclassified original failure; cannot waive it: '+item['file'])
        cases.append({'name': 'sample-'+item['name'], 'path': str(path.resolve()), 'sha256': item['sha256'],
                      'baseline': str(golden.resolve()) if not error else None, 'expected_error': error})
    corpus = ROOT/'research/case/corpus'
    for item in read(corpus/'manifest.json')['cases']:
        path = corpus/item['file']
        cases.append({'name': 'corpus-'+item['name'], 'path': str(path.resolve()), 'sha256': sha(path),
                      'baseline': str((ROOT/reference['corpus_baseline']/(item['name']+'.json')).resolve())})
    cases.append({'name': 'book', 'path': str(ROOT/'ocr-book.jpg'), 'sha256': sha(ROOT/'ocr-book.jpg'),
                  'baseline': str(ROOT/reference['book_baseline'])})
    total = len(cases)
    if args.only:
        cases = [entry for entry in cases if entry['name'] == args.only]
        if not cases:
            raise ValueError('Unknown exact case name: '+args.only)
    jobs=min(args.jobs,len(cases))
    groups=[cases[index::jobs] for index in range(jobs)]
    scheduling={'method':'round-robin','jobs':jobs}
    if args.schedule_from:
        previous=read(args.schedule_from)
        if previous['model_manifest_sha256']!=sha(ROOT/'models/manifest.json'):
            raise ValueError('Scheduling reference uses another model')
        inputs={entry['name']:entry['input_sha256'] for entry in previous['results']}
        timings={entry['name']:entry['milliseconds'] for entry in previous['execution']['cases']}
        if any(inputs.get(entry['name'])!=entry['sha256'] or entry['name'] not in timings for entry in cases):
            raise ValueError('Scheduling reference must cover the exact input hashes')
        groups=[[] for _ in range(jobs)];loads=[0. for _ in range(jobs)]
        for entry in sorted(cases,key=lambda c:(-timings[c['name']],c['name'])):
            index=min(range(jobs),key=lambda i:loads[i]);groups[index].append(entry);loads[index]+=timings[entry['name']]
        scheduling={'method':'longest previous duration first; scheduling only','jobs':jobs,
                    'reference_sha256':sha(args.schedule_from),'estimated_load_ms':loads}
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    write(output/'cases.json', cases)
    if not args.compare_only:
        # Fresh directory required: stale results must never mask a failed call.
        actual = output/'actual'
        if actual.exists() and any(actual.iterdir()):
            raise ValueError('Use a fresh --output directory, or --compare-only')
        if (output/'shards').exists() and any((output/'shards').iterdir()):
            raise ValueError('Use a fresh --output directory; previous shards exist')
        def execute(index):
            inventory = output/'shards'/str(index)/'cases.json'
            destination = output/'shards'/str(index)/'actual'
            write(inventory, groups[index])
            command = [args.node, ROOT/('web/browser-regression.mjs' if args.browser else 'web/regression.mjs'), inventory, destination, ROOT/'models']
            if args.browser:
                command.append(args.browser)
                command.append(args.playwright_path or ROOT/'work/wasm-browser/node_modules/playwright-core/index.mjs')
                command.append(str(args.browser_timeout_ms))
            subprocess.run(command, check=True, cwd=ROOT)
            return destination
        with ThreadPoolExecutor(max_workers=jobs) as executor:
            destinations = list(executor.map(execute, range(jobs)))
        actual.mkdir(parents=True, exist_ok=True)
        workers = []
        for destination in destinations:
            worker=read(destination/'measurements.json')
            if (destination/'browser-memory.json').is_file():worker['browser_memory']=read(destination/'browser-memory.json')
            workers.append(worker)
            for path in destination.glob('*.json'):
                if path.name not in ('measurements.json','browser-memory.json'):
                    target = actual/path.name
                    if target.exists():raise ValueError('Duplicate result: '+target.name)
                    target.write_bytes(path.read_bytes())
        measured = {entry['name']: entry for worker in workers for entry in worker['cases']}
        write(actual/'measurements.json', {'jobs': jobs, 'workers': [{k:v for k,v in w.items() if k!='cases'} for w in workers],
                                         'cases': [measured[c['name']] for c in cases]})
    actual = output/'actual'
    spec = importlib.util.spec_from_file_location('strict_compare', ROOT/'scripts/compare-results.py')
    comparator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(comparator)
    results = []
    for item in cases:
        result_path, error_path = actual/(item['name']+'.json'), actual/(item['name']+'.error.json')
        if item.get('expected_error'):
            error = read(error_path) if error_path.is_file() else None
            expected = item['expected_error']
            report = {'passed': bool(error and all(error.get(k) == v for k, v in expected.items()) and not result_path.exists()),
                      'expected_error': expected, 'actual_error': error}
        elif result_path.is_file() and not error_path.exists():
            report = comparator.compare(read(Path(item['baseline'])), read(result_path))
        else:
            report = {'passed': False, 'error': read(error_path) if error_path.is_file() else 'No result'}
        write(output/'comparisons'/(item['name']+'.json'), report)
        results.append({'name': item['name'], 'passed': report['passed'], 'failure_count': report.get('failure_count'),
                        'expected_error': item.get('expected_error'), 'input_sha256': item['sha256'],
                        'baseline_sha256': sha(Path(item['baseline'])) if item.get('baseline') else None})
        print(item['name'], 'PASS' if report['passed'] else 'FAIL', flush=True)
    summary = {'passed': all(x['passed'] for x in results), 'full_regression': not args.only,
               'cases': len(cases), 'full_case_count': total, 'passed_cases': sum(x['passed'] for x in results),
               'model_source_sha256': reference['model_source_sha256'], 'reference': reference,
               'model_manifest_sha256': sha(ROOT/'models/manifest.json'), 'wasm_build': build,
               'comparator_sha256': sha(ROOT/'scripts/compare-results.py'), 'scheduling':scheduling,
               'browser_request_timeout_ms':args.browser_timeout_ms if args.browser else None,
               'execution': read(actual/'measurements.json'), 'results': results}
    write(output/'summary.json', summary)
    print('Passed', summary['passed_cases'], '/', len(cases), '; full regression:', summary['full_regression'])
    return 0 if summary['passed'] else 1


if __name__ == '__main__':
    sys.exit(main())
