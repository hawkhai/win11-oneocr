"""Exercise the result contract, malformed metadata and unchanged tolerances."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec=importlib.util.spec_from_file_location('strict_compare',Path(__file__).resolve().parents[1]/'scripts/compare-results.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)


class StrictComparison(unittest.TestCase):
    def setUp(self):
        word={'index':0,'text':'A','bounding_box':[0.]*8,'confidence':0.}
        self.golden={'file':'input.png','image':{'width':50,'height':50,'step':200},'image_angle':0.,'line_count':1,
                     'lines':[{'index':0,'text':'A','bounding_box':[0.]*8,'style':{'type':'printed','confidence':0.},'word_count':1,'words':[word]}]}

    def result(self,change):
        actual=copy.deepcopy(self.golden);change(actual)
        return module.compare(self.golden,actual)['passed']

    def test_valid_and_host_specific_fields(self):
        self.assertTrue(self.result(lambda x:x.update(file='another-host/input.png',engine={'backend':'wasm'})))

    def test_counts_and_indices(self):
        for value in [999,-1,True,1.,None,'1']:
            for change in [lambda x:x.update(line_count=value),lambda x:x['lines'][0].update(index=value),
                           lambda x:x['lines'][0].update(word_count=value),lambda x:x['lines'][0]['words'][0].update(index=value)]:
                with self.subTest(value=value):self.assertFalse(self.result(change))
        for field in ['line_count','lines']:
            self.assertFalse(self.result(lambda x:x.pop(field)))

    def test_bad_baseline_is_not_accepted(self):
        actual=copy.deepcopy(self.golden);self.golden['line_count']=999
        self.assertFalse(module.compare(self.golden,actual)['passed'])

    def test_malformed_records(self):
        for change in [lambda x:x.update(lines={}),lambda x:x.update(lines=[None]),lambda x:x.update(image=None),
                       lambda x:x['lines'][0].update(words=None),lambda x:x['lines'][0].update(style=None),
                       lambda x:x['lines'][0].update(words=[None]),lambda x:x['lines'][0].update(bounding_box=[0.]*7)]:
            self.assertFalse(self.result(change))

    def test_nonfinite_and_boolean_numbers(self):
        for value in [float('nan'),float('inf'),-float('inf'),True,1<<2048]:
            self.assertFalse(self.result(lambda x:x['lines'][0]['words'][0].update(confidence=value)))
            self.assertFalse(self.result(lambda x:x.update(image_angle=value)))

    def test_original_tolerances(self):
        for field,limit in [('coordinate',1.),('angle',.1),('confidence',1e-3)]:
            def change(x,value):
                if field=='coordinate':x['lines'][0]['words'][0]['bounding_box'][0]=value
                elif field=='angle':x['image_angle']=value
                else:x['lines'][0]['words'][0]['confidence']=value
            self.assertTrue(self.result(lambda x:change(x,limit)),field)
            self.assertFalse(self.result(lambda x:change(x,limit+1e-5)),field)

    def test_angle_representation_range(self):
        for angle in [-360,-.00001,360,720]:
            self.assertFalse(self.result(lambda x:x.update(image_angle=angle)))
        self.assertTrue(self.result(lambda x:x.update(image_angle=359.999)))
        self.golden['image_angle']=360
        self.assertFalse(module.compare(self.golden,self.golden)['passed'])

    def test_confidence_range_precedes_tolerance(self):
        for target in ['word','style']:
            for endpoint,outside in [(0.,-.0005),(1.,1.0005)]:
                self.setUp()
                record=self.golden['lines'][0]['words'][0] if target=='word' else self.golden['lines'][0]['style']
                record['confidence']=endpoint
                def change(x):
                    entry=x['lines'][0]['words'][0] if target=='word' else x['lines'][0]['style']
                    entry['confidence']=outside
                self.assertFalse(self.result(change))
                self.assertTrue(module.compare(self.golden,self.golden)['passed'])
                record['confidence']=outside
                self.assertFalse(module.compare(self.golden,self.golden)['passed'])


if __name__=='__main__':unittest.main()
