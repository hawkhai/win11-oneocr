"""Build the current, hash-checked input scope shared by saved-result verifiers."""
from pathlib import Path

from verification_state import contained, read, sha, signatures, same_json
from regression_options import requested_options, effective_options

IMAGE_EXTENSIONS={'.png','.jpg','.jpeg','.bmp','.gif','.tif','.tiff','.webp'}


def image_files(directory):
    return sorted(p.relative_to(directory).as_posix() for p in directory.rglob('*')
                  if p.is_file() and p.suffix.lower() in IMAGE_EXTENSIONS)


def current_cases(root,reference):
    root=Path(root).resolve()
    model=read(root/'models/manifest.json')
    if reference['model_source_sha256']!=model['source_assets']['oneocr.onemodel']:
        raise ValueError('Model and frozen reference differ')
    cases=[];samples=root/'samples';inventory=read(samples/'manifest.json')
    if image_files(samples)!=[item['file'] for item in inventory['cases']]:
        raise ValueError('Sample inventory changed; no omissions allowed')
    for item in inventory['cases']:
        contract=read(contained(root,reference['all_samples_baseline'])/(item['name']+'.contract.json'))
        golden=contained(root,reference['all_samples_baseline'])/(item['name']+'.json')
        path=contained(samples,item['file'])
        if sha(path)!=item['sha256'] or contract['image_sha256']!=item['sha256']:
            raise ValueError('Sample fingerprint changed: '+str(path))
        if contract['result_sha256'] and sha(golden)!=contract['result_sha256']:
            raise ValueError('Baseline fingerprint changed: '+str(golden))
        error=None
        if contract['measurement']['exit_code']:
            message=contract['measurement']['stderr']
            if 'Original wrapper OCR failed: -18' in message and not (50<=item['width']<=10000 and 50<=item['height']<=10000):
                error={'code':-18,'message':'Image dimensions must be 50..10000 pixels'}
            elif 'Original wrapper OCR failed: -15' in message and item['pixel_mode']=='LA':
                error={'code':-15,'message':'Unsupported image channel count'}
            else:raise ValueError('Unclassified original failure; cannot waive it: '+item['file'])
        cases.append({'name':'sample-'+item['name'],'path':str(path),'sha256':item['sha256'],
                      'baseline':str(golden.resolve()) if not error else None,'expected_error':error})
    corpus=root/'research/case/corpus';inventory=read(corpus/'manifest.json')
    # This is the fixed, explicitly registered corpus. Other research images
    # (e.g. the retained historical Bengali fixture) are not corpus cases.
    for item in inventory['cases']:
        path=contained(corpus,item['file']);fingerprint=sha(path)
        if item.get('sha256') and fingerprint!=item['sha256']:
            raise ValueError('Corpus fingerprint changed: '+item['name'])
        cases.append({'name':'corpus-'+item['name'],'path':str(path),'sha256':fingerprint,
                      'baseline':str((contained(root,reference['corpus_baseline'])/(item['name']+'.json')).resolve())})
    cases.append({'name':'book','path':str(root/'ocr-book.jpg'),'sha256':sha(root/'ocr-book.jpg'),
                  'baseline':str(contained(root,reference['book_baseline']))})
    if 'parameter_manifest' in reference:
        manifest=contained(root,reference['parameter_manifest'])
        if sha(manifest)!=reference['parameter_manifest_sha256']:raise ValueError('Parameter inventory fingerprint changed')
        for item in read(manifest)['cases']:
            path=contained(root,item['file']);options=requested_options(item['options'])
            golden=contained(root,reference['parameter_baseline'])/(item['name']+'.json')
            contract=read(golden.with_name(golden.stem+'.contract.json'))
            if sha(path)!=item['sha256'] or contract['image_sha256']!=item['sha256']:raise ValueError('Parameter input fingerprint changed: '+item['name'])
            if contract['measurement']['exit_code'] or not contract['result_sha256'] or sha(golden)!=contract['result_sha256']:raise ValueError('Invalid parameter baseline: '+item['name'])
            if not (same_json(contract['requested_options'],options) and same_json(contract['effective_options'],effective_options(options)) and same_json(contract['original_wrapper_arguments'],effective_options(options))):
                raise ValueError('Frozen parameter options changed: '+item['name'])
            if contract['dll_bindings']['oneocr.onemodel']!=reference['model_source_sha256']:raise ValueError('Parameter baseline model changed')
            cases.append({'name':'parameter-'+item['name'],'path':str(path),'sha256':item['sha256'],'options':options,'baseline':str(golden.resolve())})
    signatures(cases)
    return cases
