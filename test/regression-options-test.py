"""Option identity rejects malformed metadata before reuse or inference."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from regression_options import requested_options,effective_options,cli_options,option_signature

class RegressionOptions(unittest.TestCase):
    def test_zero_aliases_use_explicit_original_default_resolution(self):
        self.assertEqual(effective_options(),{'max_lines':1000,'resize_width':1152,'resize_height':768})
        self.assertEqual(cli_options({'max_lines':2}),['--max-lines','2'])
        self.assertNotEqual(option_signature({'max_lines':1}),option_signature({'max_lines':2}))

    def test_boolean_nonfinite_fractional_and_unknown_options_are_rejected(self):
        for value in [True,False,float('nan'),float('inf'),1.5,'1',None]:
            with self.subTest(value=value),self.assertRaises(ValueError):requested_options({'max_lines':value})
        for options in [{'unknown':1},{'max_lines':-1},{'max_lines':1001},{'resize_width':640},
                        {'resize_width':0,'resize_height':384},{'resize_width':31,'resize_height':384},
                        {'resize_width':8193,'resize_height':384},[]]:
            with self.subTest(options=options),self.assertRaises(ValueError):requested_options(options)

if __name__=='__main__':unittest.main()
