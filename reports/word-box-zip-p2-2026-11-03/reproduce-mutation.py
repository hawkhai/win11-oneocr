from pathlib import Path
import json,shlex,subprocess
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'work/word-box-zip-p2';BUILD=ROOT/'build/linux-x64'
OUT.mkdir(parents=True,exist_ok=True)
source=(ROOT/'native/tests/boundary_test.cpp').read_text()
marker='oneocr_test::require_word_box_equal(box,expected_box);'
assert marker in source
source=source.replace('#include <stdexcept>','#include <stdexcept>\n#include <limits>').replace(marker,'box[0].x=std::numeric_limits<float>::quiet_NaN();\n                '+marker)
file=OUT/'boundary_nan_mutant.cpp';file.write_text(source)
commands=subprocess.check_output(['ninja','-t','commands','bin/oneocr_boundary_test'],cwd=BUILD,text=True).splitlines()
args=shlex.split(next(c for c in commands if ' -c ' in c and '/native/tests/boundary_test.cpp' in c))
if '-MD' in args:args.remove('-MD')
for opt in ['-MT','-MF']:
    if opt in args:i=args.index(opt);del args[i:i+2]
args[args.index('-o')+1]=str(OUT/'boundary_nan_mutant.o');args[-1]=str(file);args.insert(1,'-I'+str(ROOT/'native/tests'))
compile_args=args.copy()
subprocess.run(args,cwd=BUILD,check=True)
link=next(c for c in commands if ' -o bin/oneocr_boundary_test ' in c)
args=shlex.split(link[link.index('/usr/bin/c++'):].split(' && ')[0])
args=[a for a in args if not a.startswith('-Wl,--dependency-file=')]
args[args.index('-o')+1]=str(OUT/'boundary_nan_mutant')
args=[str(OUT/'boundary_nan_mutant.o') if a=='native/CMakeFiles/oneocr_boundary_test.dir/tests/boundary_test.cpp.o' else a for a in args]
link_args=args.copy()
subprocess.run(args,cwd=BUILD,check=True)
result=subprocess.run([str(OUT/'boundary_nan_mutant')],capture_output=True,text=True)
assert result.returncode==1 and 'Nonfinite final word box coordinate' in result.stderr
records={'injected_nan_in_real_oracle_loop':{'exit_code':result.returncode,'stderr':result.stderr,'expected_rejection':True}}
before=OUT/'boundary_before_original.cpp'
if not before.exists():before=ROOT/'reports/word-box-zip-p2-2026-11-03/evidence/boundary_before_original.cpp'
old=before.read_text()
old_marker='for(int k=0;k<4;k++)if(std::abs(box[k].x'
assert old_marker in old
old=old.replace('#include <stdexcept>','#include <stdexcept>\n#include <limits>').replace(old_marker,'box[0].x=std::numeric_limits<float>::quiet_NaN();\n                '+old_marker)
(OUT/'boundary_nan_before.cpp').write_text(old)
before_compile=[str(OUT/'boundary_nan_before.cpp') if a==str(file) else str(OUT/'boundary_nan_before.o') if a==str(OUT/'boundary_nan_mutant.o') else a for a in compile_args]
before_link=[str(OUT/'boundary_nan_before.o') if a==str(OUT/'boundary_nan_mutant.o') else str(OUT/'boundary_nan_before') if a==str(OUT/'boundary_nan_mutant') else a for a in link_args]
subprocess.run(before_compile,cwd=BUILD,check=True);subprocess.run(before_link,cwd=BUILD,check=True)
result=subprocess.run([str(OUT/'boundary_nan_before')],capture_output=True,text=True)
assert result.returncode==0 and 'oracles: PASS' in result.stdout
records['before_fix_same_nan_injection']={'source_commit':'912d0ac','exit_code':result.returncode,'stdout':result.stdout,'false_pass_reproduced':True}
mutation=OUT/'validator-mutant';mutation.mkdir(exist_ok=True)
header=(ROOT/'native/tests/word_box_assertions.hpp').read_text()
guard='            if(!std::isfinite(a[axis]) || !std::isfinite(e[axis]))\n                throw std::runtime_error("Nonfinite final word box coordinate");\n'
assert guard in header
(mutation/'word_box_assertions.hpp').write_text(header.replace(guard,''))
(mutation/'test.cpp').write_text((ROOT/'native/tests/word_box_assertions_test.cpp').read_text())
subprocess.run(['/usr/bin/c++','-O3','-ffp-contract=off','-std=c++20','-I'+str(ROOT/'native/src'),str(mutation/'test.cpp'),'-o',str(mutation/'test')],check=True)
result=subprocess.run([str(mutation/'test')],capture_output=True,text=True)
assert result.returncode==1 and 'Nonfinite word box was accepted' in result.stderr
records['removed_finite_guard_is_detected']={'exit_code':result.returncode,'stderr':result.stderr,'negative_suite_detected_regression':True}
(OUT/'mutation-results.json').write_text(json.dumps({'passed':True,**records},indent=2)+'\n')
print(json.dumps({'passed':True,**records}))
