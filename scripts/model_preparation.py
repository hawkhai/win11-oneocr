"""Resolve and validate a local model source before writing preparation outputs."""
import hashlib
import json
from pathlib import Path


def source_for_preparation(root, source, models, reference):
    root, models = Path(root).resolve(), Path(models).resolve()
    implicit = source is None
    if implicit:
        if not reference.get('legacy_binary_dir') or not reference.get('model_source_sha256'):
            raise ValueError('No active model binding; supply --source explicitly and use a fresh --models directory')
        source = root/reference['legacy_binary_dir']/'oneocr.onemodel'
    source = Path(source).resolve()
    actual = hashlib.sha256(source.read_bytes()).hexdigest()
    expected = reference.get('model_source_sha256')
    if (implicit or models == (root/'models').resolve()) and expected and actual != expected:
        raise ValueError('Model source differs from the active regression binding; use a separate --models directory for another version')
    manifest = models/'manifest.json'
    if models.exists() and (not models.is_dir() or (not manifest.is_file() and any(models.iterdir()))):
        raise ValueError('Refusing a nonempty model directory without a valid source manifest; choose a fresh --models directory')
    if manifest.exists():
        previous = json.loads(manifest.read_text(encoding='utf8'))['source_assets']['oneocr.onemodel']
        if previous != actual:
            raise ValueError('Refusing to overwrite a model directory prepared from another source; choose a fresh --models directory')
    return source
