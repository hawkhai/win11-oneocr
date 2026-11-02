"""Compare native/C++ fragment multisets; final line grouping is a later stage.

This diagnostic does not replace or weaken scripts/compare-results.py.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import numpy as np

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native',type=Path,required=True)
    parser.add_argument('--cpp',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    native=json.loads(args.native.read_text(encoding='utf8'))
    groups=json.loads(args.cpp.read_text(encoding='utf8'))
    segments=[segment for group in groups if group['level']==native['level'] and group['vertical']==(native['direction']=='vert') for segment in group['segments']]
    expected=Counter(np.array(row[3:],np.float32).tobytes() for row in native['rows'])
    actual=Counter(np.array([segment['score']]+segment['box'],np.float32).tobytes() for segment in segments)
    report={'scope':'fragment_coordinates_and_confidence_multiset_only','passed':expected==actual,'native_fragments':sum(expected.values()),'cpp_fragments':sum(actual.values()),'missing':sum((expected-actual).values()),'additional':sum((actual-expected).values()),'final_line_grouping_and_order':'not_compared_here; full strict comparison is separate'}
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps(report))
    raise SystemExit(0 if report['passed'] else 1)
