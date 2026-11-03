"""Build OneOCR C++ and pinned ORT with source-controlled Wasm numerical adaptations.

Run with Python 3 on Linux/macOS or WSL. Emsdk is installed by ORT at its
locked submodule revision; no model is downloaded. See docs/wasm.md.
"""
import argparse
import filecmp
import hashlib
import json
import os
import re
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def run(command, **kwargs):
    print('+', ' '.join(map(str, command)), flush=True)
    subprocess.run(list(map(str, command)), check=True, **kwargs)


def patch_opencv(source, lock):
    git = ['git', '-c', 'safe.directory='+str(source), '-C', source]
    if subprocess.check_output(git+['rev-parse', 'HEAD'], text=True).strip() != lock['opencv']['commit']:
        raise ValueError('OpenCV revision mismatch; preserving checkout')
    name = 'modules/core/include/opencv2/core/hal/intrin_wasm.hpp'
    original = subprocess.check_output(git+['show', 'HEAD:'+name]).replace(b'\r\n', b'\n')
    before = b'''inline v_int32x4 v_round(const v_float32x4& a)
{
    v128_t h = wasm_f32x4_splat(0.5);
    return v_int32x4(wasm_i32x4_trunc_saturate_f32x4(wasm_f32x4_add(a.val, h)));
}'''
    if original.count(before) != 1:
        raise ValueError('Unexpected pinned OpenCV v_round implementation')
    replacement = (ROOT/'native/wasm/opencv-rounding.hpp').read_bytes().replace(b'\r\n', b'\n').rstrip()
    patched = original.replace(before, replacement, 1)
    path = source/name
    current = path.read_bytes()
    normalized = current.replace(b'\r\n', b'\n')
    if normalized not in (original, patched):
        raise ValueError('Preserving unexpected local OpenCV changes: '+str(path))
    if normalized == original:
        newline = b'\r\n' if b'\r\n' in current else b'\n'
        path.write_bytes(current.replace(before.replace(b'\n', newline), replacement.replace(b'\n', newline), 1))
    return {'file': name, 'upstream_sha256': hashlib.sha256(original).hexdigest(),
            'patched_sha256': hashlib.sha256(patched).hexdigest(), 'change': 'Wasm v_round matches SSE nearest-even and invalid conversion sentinel'}


def patch_ort(source, git):
    state_path=source/'.oneocr-wasm-source.json'
    previous=json.loads(state_path.read_text()) if state_path.is_file() else {}
    records=previous.get('files',{})
    if previous.get('file'):records[previous['file']]=previous
    def apply(name,original,patched,description):
        path=source/name;payload=path.read_bytes();normalized=payload.replace(b'\r\n',b'\n')
        upstream_sha=hashlib.sha256(original).hexdigest();current_sha=hashlib.sha256(normalized).hexdigest()
        old=records.get(name,{})
        known_previous=(old.get('upstream_sha256')==upstream_sha and old.get('patched_sha256')==current_sha)
        if normalized not in (original,patched) and not known_previous:raise ValueError('Preserving unexpected local MLAS changes: '+name)
        if normalized!=patched:
            newline=b'\r\n' if b'\r\n' in payload else b'\n';path.write_bytes(patched.replace(b'\n',newline))
        record={'file':name,'upstream_sha256':upstream_sha,'patched_sha256':hashlib.sha256(patched).hexdigest(),'change':description}
        records[name]=record
        state_path.write_text(json.dumps({'files':records},indent=2)+'\n',encoding='utf8')
        return record
    name='onnxruntime/core/mlas/lib/convolve.cpp'
    original=subprocess.check_output(git+['show','HEAD:'+name]).replace(b'\r\n',b'\n')
    fragment=(ROOT/'native/wasm/convolution.hpp').read_bytes().replace(b'\r\n',b'\n')
    include=b'#include "mlasi.h"\n'
    anchor=b'    // Override\n'
    # MlasConv's override precedes MlasConvPrepare's second override in this pin.
    if original.count(include)!=1 or original.count(anchor)!=2:raise ValueError('Unexpected pinned MLAS convolution source')
    patched=original.replace(include,include+b'\n'+fragment+b'\n',1).replace(anchor,
        b'#if defined(__EMSCRIPTEN__)\n    if(OneOcrWasmConvolution(Parameters,Input,Filter,Bias,Output))return;\n#endif\n'+anchor,1)
    adjustments=[apply(name,original,patched,'AVX512 NCHWc channel blocks 16; pointwise batches 128; correctly rounded f32 FMA')]
    activation=(ROOT/'native/wasm/activation.hpp').read_bytes().replace(b'\r\n',b'\n')
    for filename in ('logistic.cpp','tanh.cpp'):
        name='onnxruntime/core/mlas/lib/'+filename
        original=subprocess.check_output(git+['show','HEAD:'+name]).replace(b'\r\n',b'\n')
        anchor=b'#include "softmax.h"\n' if filename=='tanh.cpp' else include
        if original.count(anchor)!=1:raise ValueError('Unexpected activation source')
        patched=original.replace(anchor,anchor+b'\n'+activation+b'\n',1)
        pattern=rb'(?m)^( *)([pq]) = ([A-Za-z0-9_.]+) \* ([A-Za-z0-9_.]+) \+ ([A-Za-z0-9_.]+);$'
        patched,count=re.subn(pattern,rb'\1\2 = OneOcrScalarFma(\3, \4, \5);',patched)
        if count!=9:raise ValueError('Unexpected scalar activation polynomial: '+filename+' '+str(count))
        adjustments.append(apply(name,original,patched,'Use the native FMA3 polynomial operations for SIMD and scalar tails; coefficients unchanged'))
    return {'profile':'oneocr-wasm-fma-nchwc16-v2','files':adjustments}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ort-source', type=Path, default=ROOT/'work/vendor/onnxruntime')
    parser.add_argument('--opencv-source', type=Path, default=ROOT/'work/vendor/opencv')
    parser.add_argument('--icu-source', type=Path, default=ROOT/'work/vendor/icu-source')
    parser.add_argument('--build-dir', type=Path, default=ROOT/'build/wasm32')
    parser.add_argument('--output', type=Path, default=ROOT/'artifacts/wasm32')
    parser.add_argument('--dependency-cache', type=Path, help='Reuse ORT FetchContent source directories from an existing build')
    parser.add_argument('--jobs', type=int, default=6)
    parser.add_argument('--skip-ort-build','--reuse-ort-config', action='store_true', help='Reuse the configured ORT build; rebuild changed sources so the archive stays in sync')
    args = parser.parse_args()
    if os.name == 'nt':
        raise SystemExit('Use Linux/macOS or WSL for the reproducible wasm build')
    if args.jobs < 1:
        raise ValueError('--jobs must be positive')
    lock = json.loads((ROOT/'research/dependencies.json').read_text())
    ort = args.ort_source.resolve()
    if not ort.exists():
        run(['git', 'clone', '--no-checkout', lock['onnxruntime']['repository'], ort])
        run(['git', '-C', ort, 'checkout', lock['onnxruntime']['commit']])
    git = ['git', '-c', 'safe.directory='+str(ort), '-C', ort]
    if subprocess.check_output(git+['rev-parse', 'HEAD'], text=True).strip() != lock['onnxruntime']['commit']:
        raise ValueError('ORT revision mismatch; preserving existing checkout')
    ort_adjustment=patch_ort(ort,git)
    run(git+['submodule', 'update', '--init', 'cmake/external/emsdk'])
    emsdk = ort/'cmake/external/emsdk'
    expected = lock['webassembly']['emsdk_commit']
    actual = subprocess.check_output(['git', '-C', emsdk, 'rev-parse', 'HEAD'], text=True).strip()
    if actual != expected:
        raise ValueError('Emsdk revision mismatch')
    run([emsdk/'emsdk', 'install', lock['webassembly']['emscripten_version']])
    run([emsdk/'emsdk', 'activate', lock['webassembly']['emscripten_version']])
    environment = dict(os.environ)
    # Activate before configuring ORT too: WSL must not find a Windows node.exe.
    payload = subprocess.check_output(['bash', '-c', 'source "$1/emsdk_env.sh" >/dev/null 2>&1 && env -0', 'oneocr-env', str(emsdk)])
    for item in payload.split(b'\0'):
        if b'=' in item:
            key, value = item.split(b'=', 1)
            environment[os.fsdecode(key)] = os.fsdecode(value)
    build = args.build_dir.resolve()
    build.mkdir(parents=True, exist_ok=True)
    ort_build = build/'ort'
    if not args.skip_ort_build:
        definitions = ['CMAKE_CXX_FLAGS=-ffp-contract=off', 'CMAKE_C_FLAGS=-ffp-contract=off', 'CMAKE_POLICY_VERSION_MINIMUM=3.5', 'onnxruntime_BUILD_UNIT_TESTS=OFF']
        if args.dependency_cache:
            for directory in sorted(args.dependency_cache.resolve().glob('*-src')):
                if directory.is_dir():
                    name = directory.name[:-4].upper()
                    definitions.append('FETCHCONTENT_SOURCE_DIR_'+name+'='+str(directory))
        command = [sys.executable, ort/'tools/ci_build/build.py', '--build_dir', ort_build,
                   '--config', 'Release', '--build_wasm_static_lib', '--enable_wasm_simd',
                   '--parallel', args.jobs, '--skip_tests', '--skip_submodule_sync',
                   '--allow_running_as_root', '--cmake_generator', 'Ninja',
                   '--emsdk_version', lock['webassembly']['emscripten_version'],
                   '--compile_no_warning_as_error', '--cmake_extra_defines', *definitions]
        run(command, cwd=ort, env=environment)
    else:
        run(['cmake','--build',ort_build/'Release','--target','bundling_target','--parallel',args.jobs],env=environment)
    archive = ort_build/'Release/libonnxruntime_webassembly.a'
    # Upstream's bundle custom command only depends on target ordering, so a
    # changed MLAS .a can otherwise leave an old complete archive untouched.
    # Re-run its generated MRI recipe with the same compiler ar, without a shell.
    recipe=ort_build/'Release/onnxruntime_webassembly.ar'
    if not recipe.is_file():raise FileNotFoundError(recipe)
    with recipe.open('rb') as commands:
        run([emsdk/'upstream/emscripten/emar','-M'],stdin=commands,cwd=ort_build/'Release',env=environment)
    if not archive.is_file():
        raise FileNotFoundError(archive)
    sdk = build/'ort-sdk'
    (sdk/'lib').mkdir(parents=True, exist_ok=True)
    installed_archive = sdk/'lib/libonnxruntime_webassembly.a'
    if not installed_archive.exists() or not filecmp.cmp(archive, installed_archive, shallow=False):
        shutil.copyfile(archive, installed_archive)
    shutil.copytree(ort/'include/onnxruntime/core/session', sdk/'include', dirs_exist_ok=True)
    emcmake = emsdk/'upstream/emscripten/emcmake'
    if not args.opencv_source.exists():
        if args.opencv_source.resolve() != (ROOT/'work/vendor/opencv').resolve():
            raise FileNotFoundError(args.opencv_source)
        run([sys.executable, ROOT/'scripts/fetch-opencv.py'])
    opencv_adjustment = patch_opencv(args.opencv_source.resolve(), lock)
    run([sys.executable, ROOT/'scripts/prepare-icu.py', '--source', args.icu_source, '--fetch-only'])
    if not (ROOT/'work/icu-generated/icudt74l_dat.c').is_file():
        run([sys.executable, ROOT/'scripts/prepare-icu.py', '--source', args.icu_source, '--jobs', args.jobs])
    data_record = json.loads((ROOT/'work/icu-generated/provenance.json').read_text())
    if (data_record['icu_commit'] != lock['icu']['commit'] or data_record['data_sha256'] != lock['icu']['data_sha256'] or
        data_record['generated_c_sha256'] != hashlib.sha256((ROOT/'work/icu-generated/icudt74l_dat.c').read_bytes()).hexdigest()):
        raise ValueError('ICU data provenance mismatch; regenerate from the pinned source')
    native = build/'engine'
    run([emcmake, 'cmake', '-S', ROOT, '-B', native, '-G', 'Ninja',
         '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF', '-DONNXRUNTIME_ROOT='+str(sdk),
         '-DONEOCR_OPENCV_SOURCE='+str(args.opencv_source.resolve()),
         '-DONEOCR_ICU_SOURCE='+str(args.icu_source.resolve())], env=environment)
    run(['cmake', '--build', native, '--target', 'oneocr_wasm', 'oneocr_wasm_rounding_test', 'oneocr_wasm_fma_test', 'oneocr_wasm_boundary_test', 'oneocr_wasm_word_box_assertions_test', '--parallel', args.jobs], env=environment)
    node = environment['EMSDK_NODE']
    run([node, native/'bin/oneocr_wasm_rounding_test.js'], env=environment)
    run([node, native/'bin/oneocr_wasm_fma_test.js'], env=environment)
    boundary=subprocess.run([node,str(native/'bin/oneocr_wasm_boundary_test.js')],cwd=native/'bin',env=environment,capture_output=True,text=True)
    print(boundary.stdout,end='',flush=True);print(boundary.stderr,end='',file=sys.stderr,flush=True)
    boundary.check_returncode()
    if 'Curve fallback and clipped polygon DLL oracles: PASS' not in boundary.stdout:
        raise RuntimeError('Wasm boundary test did not complete its C++ assertions')
    run([node, native/'bin/oneocr_wasm_word_box_assertions_test.js'], env=environment)
    output = args.output.resolve()
    run(['cmake', '--install', native, '--prefix', output], env=environment)
    emscripten = emsdk/'upstream/emscripten'
    license_sources = {'emscripten-LICENSE.txt': 'LICENSE', 'musl-COPYRIGHT.txt': 'system/lib/libc/musl/COPYRIGHT',
                       'libcxx-LICENSE.txt': 'system/lib/libcxx/LICENSE.TXT', 'libcxxabi-LICENSE.txt': 'system/lib/libcxxabi/LICENSE.TXT',
                       'compiler-rt-LICENSE.txt': 'system/lib/compiler-rt/LICENSE.TXT'}
    for name, relative in license_sources.items():
        source = emscripten/relative
        destination = ROOT/'licenses'/name
        if destination.exists() and destination.read_bytes() != source.read_bytes():
            raise ValueError('Preserving changed checked-in runtime notice: '+str(destination))
        shutil.copyfile(source, destination)
        shutil.copyfile(source, output/'share/oneocr/licenses'/name)
    provenance = {'schema_version': 1, 'target': 'wasm32', 'onnxruntime_commit': lock['onnxruntime']['commit'],
                  'emsdk_commit': expected, 'emscripten_version': lock['webassembly']['emscripten_version'],
                  'simd': 'wasm SIMD128; no relaxed SIMD', 'threads': 1, 'exceptions': 'Emscripten JS-based',
                  'implicit_fp_contraction': 'off', 'model_directory': '../../models',
                  'opencv_source_adjustment': opencv_adjustment,
                  'ort_source_adjustment': ort_adjustment,
                  'model_source_sha256': json.loads((ROOT/'models/manifest.json').read_text())['source_assets']['oneocr.onemodel'],
                  'model_manifest_sha256': hashlib.sha256((ROOT/'models/manifest.json').read_bytes()).hexdigest(),
                  'source_hash_format': 'sha256 after CRLF to LF normalization; models and artifacts use exact bytes',
                  'source_files': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes().replace(b'\r\n', b'\n')).hexdigest()
                                   for p in sorted([*(ROOT/'native').rglob('*'), *(ROOT/'cmake').rglob('*'), *(ROOT/'web').rglob('*'),
                                                    ROOT/'CMakeLists.txt', ROOT/'json.hpp', ROOT/'stb_image.h',
                                                    ROOT/'scripts/build-wasm.py', ROOT/'scripts/run-wasm-parity.py',
                                                    ROOT/'scripts/verification_state.py', ROOT/'test/verification-state-test.py',
                                                    ROOT/'scripts/regression_inputs.py',
                                                    ROOT/'scripts/compare-results.py', ROOT/'scripts/browser-memory.ps1', ROOT/'test/compare-results-test.py',
                                                    ROOT/'test/wasm-test.mjs', ROOT/'research/dependencies.json']) if p.is_file()},
                  'files': {p.relative_to(output).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                            for p in sorted(output.rglob('*')) if p.is_file() and p.name != 'BUILD.json'}}
    (output/'BUILD.json').write_text(json.dumps(provenance, indent=2)+'\n', encoding='utf8')
    print('Built', output)


if __name__ == '__main__':
    main()
