"""Immutable inference records and read-only verification of saved Wasm results."""
import hashlib
import json
from pathlib import Path


def read(path):return json.loads(Path(path).read_text(encoding='utf8'))
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def same_json(a,b):
    # JSON booleans, integers and nonintegral numbers must not alias via Python ==.
    return json.dumps(a,sort_keys=True,allow_nan=False,separators=(',',':'))==json.dumps(b,sort_keys=True,allow_nan=False,separators=(',',':'))


def contained(root,name):
    path=(Path(root)/name).resolve()
    if not path.is_relative_to(Path(root).resolve()):raise ValueError('Recorded path leaves execution directory: '+name)
    return path


def signatures(cases):
    result={}
    for case in cases:
        name=case['name']
        if not isinstance(name,str) or not name or name in ('.','..') or '/' in name or '\\' in name or name in result:
            raise ValueError('Invalid or duplicate case name')
        result[name]=(case['sha256'],case.get('expected_error'))
    return result


def destination_for_verification(execution,destination):
    execution,destination=Path(execution).resolve(),Path(destination).resolve()
    if destination.is_relative_to(execution) or execution.is_relative_to(destination):
        raise ValueError('Verification output must be separate from the execution directory')
    if destination.exists() and (not destination.is_dir() or any(destination.iterdir())):
        raise ValueError('Use a fresh verification output directory')
    return destination


def runtime_assets(build):
    assets={n:h for n,h in build['files'].items() if n.endswith(('.mjs','.wasm','.d.mts'))}
    if not any(n.endswith('.wasm') for n in assets):raise ValueError('Execution has no recorded Wasm runtime')
    return assets


def check_hashes(directory,files):
    for name,expected in files.items():
        if sha(contained(directory,name))!=expected:raise ValueError('Execution file changed: '+name)


def check_result_files(directory,cases,native=False):
    """Require one unambiguous outcome per case, with no extra result JSON files."""
    directory=Path(directory).resolve();signatures(cases);expected=set()
    for case in cases:
        name=case['name']
        if native:
            result=directory/'book.json' if name=='book' else directory/'corpus/native'/(name.removeprefix('corpus-')+'.json') if name.startswith('corpus-') else directory/'all-samples/native'/(name.removeprefix('sample-')+'.json')
        else:result=directory/'actual'/(name+'.json')
        error=result.with_name(result.stem+'.error.json')
        if case.get('expected_error'):
            if result.exists() or (native and error.exists()):raise ValueError('Conflicting result for rejected case: '+name)
            if not native:expected.add(error.resolve())
        else:
            if error.exists():raise ValueError('Conflicting error file for successful case: '+name)
            expected.add(result.resolve())
    if native:
        found={p.resolve() for folder in ['all-samples/native','corpus/native'] for p in (directory/folder).glob('*.json')}
        found.update(p.resolve() for p in [directory/'book.json',directory/'book.error.json'] if p.exists())
    else:found={p.resolve() for p in (directory/'actual').glob('*.json') if p.name!='measurements.json'}
    if found!=expected:raise ValueError('Missing or unexpected result JSON files')
    return found


def check_report_index(directory,names,anchor,required):
    directory=Path(directory).resolve()
    for ancestor in [directory,*directory.parents]:
        index=ancestor/'FILES.sha256.json'
        if not index.is_file():continue
        records=read(index);prefix=directory.relative_to(ancestor)
        if (prefix/anchor).as_posix() not in records:continue
        keys={name:(prefix/name).as_posix() for name in names}
        if not all(key in records for key in keys.values()):raise ValueError('Incomplete committed report index')
        check_hashes(directory,{name:records[key]['sha256'] for name,key in keys.items()})
        return 'committed report index: '+str(index)
    if required:raise ValueError('No sealed execution record; retain the complete legacy report index or run fresh inference')
    return None


def checkpoint(directory,build,cases,scheduling,timeout):
    directory=Path(directory)
    names=['cases.json','execution-cases.json','execution-build.json','summary.json','actual/measurements.json']
    names.extend(p.relative_to(directory).as_posix() for p in sorted((directory/'actual').glob('*.json')) if p.name!='measurements.json')
    return {'schema_version':1,'mode':'fresh-inference','build':build,'cases':cases,
            'scheduling':scheduling,'browser_request_timeout_ms':timeout,
            'baselines':{c['name']:sha(c['baseline']) if c.get('baseline') else None for c in cases},
            'files':{name:sha(directory/name) for name in names}}


def load_execution(directory):
    directory=Path(directory).resolve();summary=read(directory/'summary.json')
    if (directory/'execution.json').is_file():
        state=read(directory/'execution.json')
        if type(state.get('schema_version')) is not int or state['schema_version']!=1:raise ValueError('Unsupported execution record')
        required={'summary.json','cases.json','execution-cases.json','execution-build.json','actual/measurements.json'}
        required.update(p.relative_to(directory).as_posix() for p in (directory/'actual').glob('*.json'))
        if not required<=set(state['files']):raise ValueError('Incomplete execution file seal')
        check_hashes(directory,state['files'])
        published=check_report_index(directory,set(state['files'])|{'execution.json'},'execution.json',False)
        captured_build=read(directory/'execution-build.json');captured_cases=read(directory/'execution-cases.json')
        baselines={c['name']:c.get('baseline_sha256') for c in summary['results']}
        checks=[state.get('mode')=='fresh-inference',summary.get('mode')=='fresh-inference',summary.get('fresh_inference') is True,
                same_json(state['build'],captured_build) and same_json(captured_build,summary['wasm_build']),
                same_json(state['cases'],captured_cases) and same_json(captured_cases,read(directory/'cases.json')),
                same_json(state['scheduling'],summary['scheduling']),same_json(state.get('baselines'),baselines),
                same_json(state.get('browser_request_timeout_ms'),summary.get('browser_request_timeout_ms'))]
        if not all(checks):raise ValueError('Duplicated execution metadata disagree with sealed files')
        # Use the hash-checked files as authority, rather than the mutable copies.
        state={**state,'build':captured_build,'cases':captured_cases,'scheduling':summary['scheduling'],'baselines':baselines}
        integrity=published or 'execution.json file hashes; duplicate metadata checked'
    else:
        # Legacy reports have an independently committed outer report index.
        case_file='execution-cases.json' if (directory/'execution-cases.json').is_file() else 'cases.json'
        build_file=directory/'execution-build.json'
        state={'build':read(build_file) if build_file.is_file() else summary['wasm_build'],
               'cases':read(directory/case_file),'scheduling':summary.get('scheduling'),
               'browser_request_timeout_ms':summary.get('browser_request_timeout_ms')}
        needed={'summary.json','cases.json',case_file,'actual/measurements.json'}
        if build_file.is_file():needed.add('execution-build.json')
        needed.update(p.relative_to(directory).as_posix() for p in (directory/'actual').glob('*.json'))
        integrity='legacy '+check_report_index(directory,needed,'summary.json',True)
    check_result_files(directory,state['cases'])
    measured=read(directory/'actual/measurements.json')
    if state['scheduling'] and state['scheduling']['jobs']!=measured['jobs']:
        raise ValueError('Recorded scheduling contradicts execution measurements')
    if summary['model_manifest_sha256']!=state['build']['model_manifest_sha256']:
        raise ValueError('Execution model metadata disagree')
    return {'summary':summary,'state':state,'measurements':measured,'integrity':integrity}


def verify_execution(record,cases,build,models,subset=False):
    original=record['state'];captured=signatures(original['cases']);selected=signatures(cases)
    if (not subset and set(captured)!=set(selected)) or any(captured.get(n)!=v for n,v in selected.items()):
        raise ValueError('Verification input scope differs from the recorded execution')
    for key in ('model_manifest_sha256','model_source_sha256'):
        if original['build'][key]!=build[key]:raise ValueError('Current model differs from execution: '+key)
    if runtime_assets(original['build'])!=runtime_assets(build):
        raise ValueError('Current runtime differs from execution; fresh inference is required')
    if sha(Path(models)/'manifest.json')!=build['model_manifest_sha256']:
        raise ValueError('Model manifest changed')
    def assets(value):
        if isinstance(value,dict):
            if isinstance(value.get('file'),str) and isinstance(value.get('sha256'),str):
                yield value['file'],value['sha256']
            for item in value.values():yield from assets(item)
        elif isinstance(value,list):
            for item in value:yield from assets(item)
    for name,expected in assets(read(Path(models)/'manifest.json')):
        if sha(contained(models,name))!=expected:raise ValueError('Model asset changed: '+name)
    baselines=original.get('baselines') or {c['name']:c.get('baseline_sha256') for c in record['summary']['results']}
    for case in cases:
        if case.get('baseline') and baselines.get(case['name'])!=sha(case['baseline']):
            raise ValueError('Frozen baseline changed: '+case['name'])
