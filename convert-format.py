### Opening files
import numpy as np

np.random.seed(1729)

filename_in = './wikiElec.txt'
fi = open(filename_in, 'r', errors='replace')
lines = fi.readlines()[6:]

filename_out = f'{filename_in[:-3]}triples'
filename_orig_out = f'{filename_in[:-3]}orig'

fo = open(filename_out, 'w')



### Conversion
user_name_to_idx = {}
idx = 0
fo2_out_string = ""
current_nominee = None

def write_vote(nominee, voter, vote):
    # If they are neutral, we don't write this vote
    global idx
    global fo2_out_string
    if vote in ['-1', '0', '1']:
        if (nominee) and (voter):
            nval, vval = -1, -1
            if nominee in user_name_to_idx:
                nval = user_name_to_idx[nominee]
            else:
                nval = idx
                user_name_to_idx[nominee] = idx
                idx += 1
            if voter in user_name_to_idx:
                vval = user_name_to_idx[voter]
            else:
                vval = idx
                user_name_to_idx[voter] = idx
                idx += 1
            _vote = int(vote)
            mean = 0.5 + _vote * 0.4
            score = max(1e-5, np.random.normal(mean, 0.001))

            fo.write(f'{vval}\t{nval}\t{score}\n')
            fo2_out_string = ''.join([fo2_out_string, f'{vval}\t{nval}\t{vote}\n'])
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
with open(filename_orig_out, 'w') as fo2:
    fo2.write(f"{idx}\n")
    fo2.write(fo2_out_string)
