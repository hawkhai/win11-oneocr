"""Record measured status; never upgrades failed parity to a passing release."""
import hashlib
import json
from pathlib import Path
import statistics

ROOT=Path(__file__).resolve().parents[1]
CASE=ROOT/'work/oneocr-portable'
def read(path):return json.loads(path.read_text(encoding='utf8'))
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

if __name__=='__main__':
    corpus=read(CASE/'evidence/corpus-parity.json')
    runs=read(CASE/'corpus-native/measurements.json')
    baseline=read(CASE/'corpus-legacy/measurements.json')
    auxiliary=read(CASE/'evidence/auxiliary-conversion.json')
    models=read(ROOT/'models/manifest.json')
    offline=read(CASE/'linux-staged-offline.verification.json')
    windows_tests='100% tests passed' in (CASE/'evidence/native-ctest.log').read_text(encoding='utf8',errors='replace')
    ort_source_build='Build complete' in (CASE/'evidence/ort-source-build.log').read_text(encoding='utf8',errors='replace')
    report={
        'date':'2026-11-02',
        'goal_complete':False,
        'parity':'incomplete',
        'model_source_sha256':models['source_assets']['oneocr.onemodel'],
        'model_resources':67,
        'onnx_graphs':len(models['models']),
        'vocabularies':{Path(x['file']).stem.removeprefix('vocab_'):x['entries'] for x in models['vocabularies']},
        'private_layer_conversions':len(auxiliary),
        'private_layer_max_absolute_error':max(x['max_absolute_error'] for x in auxiliary),
        'platforms':{
            'windows-x64':{'native_build':'passed','native_unit_tests':windows_tests,'ort_source_build':ort_source_build,'ort_commit':'a83fc4d58cb48eb68890dd689f94f28288cf2278','ort_dll_sha256':sha(ROOT/'work/ort-install/bin/onnxruntime.dll'),'api_integration':read(CASE/'evidence/api-integration.json')['checks'],'legacy_api_adapter':read(CASE/'evidence/compat-parity.json')['passed']},
            'linux-x64':{'native_build':'passed','offline_run':offline['passed'],'network_probe':offline['network_error'],'forbidden_modules':offline['forbidden_modules'],'ort_origin':'checksum-verified official v1.23.2 CPU SDK; Linux ORT source build not executed','native_matches_windows':read(CASE/'evidence/windows-linux-parity.json')['passed']},
            'linux-arm64':{'native_build':'not_verified','native_run':'not_verified'},
            'macos-x64':{'native_build':'not_verified','native_run':'not_verified'},
            'macos-arm64':{'native_build':'not_verified','native_run':'not_verified'},
            'android-arm64':{'native_build':'not_verified','native_run':'not_verified','java_example_compile':(CASE/'android-java-check/com/example/oneocr/OneOcr.class').exists()}
        },
        'corpus':{'process_successes':sum(x['exit_code']==0 for x in runs),'original_wrapper_successes':sum(x['exit_code']==0 for x in baseline),'total':len(runs),'strict_parity_passes':corpus['passed_cases'],'strict_parity_total':corpus['total_cases'],'mean_native_seconds_including_startup':statistics.mean(x['seconds'] for x in runs),'max_native_peak_working_set_bytes':max(x['peak_working_set_bytes'] or 0 for x in runs),'coverage_gaps':[corpus['coverage_gap']]},
        'book_sample':{'original_lines':read(CASE/'baseline/ocr-book.json')['line_count'],'native_lines':read(CASE/'native-source-runtime-ocr-book.json')['line_count'],'reference_lines':read(CASE/'reference-ocr-book.json')['line_count'],'native_strict_parity':read(CASE/'evidence/parity-ocr-book.json')['passed'],'reference_strict_parity':read(CASE/'evidence/parity-reference-ocr-book.json')['passed']},
        'default_plugin_native_integration':read(CASE/'evidence/plugin-native-integration.json')['passed'],
        'remaining':['Exact region linking, curved-line rectification and line suppression','Reading order, line/word segmentation and word quadrilateral alignment','Original 21-feature rejection/confidence extraction and integration','Complete handwritten and orientation decision logic','Real handwriting fixtures and target device verification'],
        'case':'work/oneocr-portable'
    }
    (ROOT/'research/verification.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
    (CASE/'evidence/verification.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
    print('Recorded measured verification; goal_complete=false')
