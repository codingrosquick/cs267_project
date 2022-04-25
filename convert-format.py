### Opening files
import numpy as np

np.random.seed(1729)

filename_in = './wikiElec.txt'
fi = open(filename_in, 'r', errors='replace')
lines = fi.readlines()[6:]

filename_out = f'{filename_in[:-3]}triples'
print(f'filename_out is {filename_out}')
fo = open(filename_out, 'w')
fo.write('#!c++\n')


### Conversion

current_nominee = None

def write_vote(nominee, voter, vote):
    # If they are neutral, we don't write this vote
    if vote in ['-1', '0', '1']:
        if (nominee) and (voter):
            _vote = int(vote)
            mean = 0.5 + _vote * 0.4
            score = max(1e-5, np.random.normal(mean, 0.001))
            fo.write(f'{voter}\t{nominee}\t{score}\n')
        else:
            print(Exception(f'undefined nodes!: n={nominee} v={voter}'))
    

for line in lines:
    if line[0] == 'U':
        current_nominee = line.split('\t')[-1][:-1]
    if line[0] == 'V':
        vote = line.split('\t')[1]
        voter = line.split('\t')[-1][:-1]
        write_vote(current_nominee, voter, vote)

fo.close()

