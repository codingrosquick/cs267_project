import numpy as np

### Opening files

filename_in = './wikiElec.ElecBs3.triples'
fi = open(filename_in, 'r', errors='replace')
lines = fi.readlines()[1:]

filename_out = f'{filename_in[:-7]}prbs-025-001.triples'
fo = open(filename_out, 'w')
fo.write('#!c++\n')

stdev = 0.01
half_spacing = 0.25

for line in lines:
    addon = np.random.normal(half_spacing, stdev)
    elems = line.split('\t')
    prb_value = None
    if elems[2] == '1\n':
        prb_value = 0.5 + addon
    elif elems[2] == '-1\n':
        prb_value = 0.5 - addon
        # unlikely scenario (happens rarely) -> the value
        # would be very small so we just take its absolute value
        if prb_value < 0:
            prb_value = - prb_value
    fo.write(f'{elems[0]}\t{elems[1]}\t{prb_value}\n')