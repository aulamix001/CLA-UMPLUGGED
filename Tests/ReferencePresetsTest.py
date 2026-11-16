"""Compare the compiled preset exporter with the original XML parameter records."""
import json
from pathlib import Path
import subprocess
import sys
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
names = [x.get('Name') for x in ET.parse(root / 'Presets/Reference/parameters.xml').getroot()]
compiled = json.loads(subprocess.check_output([sys.argv[1]], text=True))
assert compiled['schemaVersion'] == 2
bank = compiled['presets']
assert len(bank) == 185
assert bank[151]['name'] == 'Default / all modules bypassed'
for index, preset in enumerate(ET.parse(root / 'Presets/Reference/factory.xml').getroot(), 152):
    values = dict(zip(names, preset.find('PresetData/Parameters').text.split()))
    actual = bank[index]
    assert actual['index'] == index and actual['name'] == preset.get('Name')
    pairs = {'input': 'INPUT_GAIN', 'output': 'OUTPUT_GAIN', 'balance': 'BALANCE',
             'predelay1': 'PREDELAY_1', 'predelay2': 'PREDELAY_2'}
    for internal, external in [('bass','BASS'), ('treble','TREBLE'), ('compress','COMPRESS'),
                               ('reverb1','REVERB_1'), ('reverb2','REVERB_2'), ('delay','DELAY')]:
        pairs[internal] = external
        pairs[internal + 'Colour'] = external + '_COLOR'
    for internal, external in pairs.items():
        assert abs(actual['parameters'][internal] - float(values['PARAM_' + external])) < 0.00001, (preset.get('Name'), internal)
    for i in (1, 2):
        assert actual['parameters'][f'predelay{i}On'] == bool(int(values[f'PARAM_PREDELAY_{i}_ON_OFF']))
    assert actual['parameters']['direct'] == (values['PARAM_DRY_OUTPUT_GAIN_DIRECT_IN_OUT'] == '0')
    assert actual['parameters']['bypass'] == bool(int(values['PARAM_BYPASS_STATE']))
print('PASS all 21 exposed parameters of 33 reference presets; 152 legacy indices retained')
