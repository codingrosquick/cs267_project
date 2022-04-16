### Opening files

filename_in = './wikiElec.ElecBs3.txt'
fi = open(filename_in, 'r', errors='replace')
lines = fi.readlines()[6:]

filename_out = f'{filename_in[:-3]}.triples.txt'
fo = open(filename_out, 'w')
fo.write('#!c++\n')


### Conversion

current_nominee = None

def write_vote(nominee, voter, vote):
    # If they are neutral, we don't write this vote
    if (vote != '0'):
        if (nominee) and (voter):
            fo.write(f'{voter}\t{nominee}\t{vote}\n')
        else:
            print(Exception(f'undefined nodes!: n={nominee} v={voter}'))
    

for line in lines:
    if line[0] == 'U':
        current_nominee = line.split('\t')[-1][:-1]
    if line[0] == 'V':
        vote = line.split('\t')[1]
        voter = line.split('\t')[-1][:-1]
        write_vote(current_nominee, voter, vote)
    
