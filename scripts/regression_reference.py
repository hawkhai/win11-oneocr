"""Read the active model-bound regression locations without importing inference code."""
import json
from pathlib import Path

def active_reference(snapshot=None):
    # Explicit snapshots are for historical saved-result verification only.
    # Fresh native inference always uses the current delivery manifest.
    if snapshot is not None:
        return json.loads(Path(snapshot).read_text(encoding='utf8'))
    manifest=Path(__file__).resolve().parents[1]/'research/delivery-manifest.json'
    return json.loads(manifest.read_text(encoding='utf8')).get('reference',{}) if manifest.is_file() else {}


def active_archive():
    manifest=Path(__file__).resolve().parents[1]/'research/delivery-manifest.json'
    return json.loads(manifest.read_text(encoding='utf8')).get('verification_archive','reports/review-fixes-2026-11-03')
