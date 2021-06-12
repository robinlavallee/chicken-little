import os

fragments = [124, 173060, 358896, 546204, 763164, 814876, 892188,
                925250,
             1152578,
             1506882,
             1519354,
             1544318,
             1569282,
             1768962,
             1799682]

file_size = os.path.getsize('gamesfx.dat')
with open('gamesfx.dat', 'rb') as inf:
    for i in range(len(fragments)):
        
        seek_pos = fragments[i]
    
        if i + 1 >= len(fragments): # last item
            sub_file_length = file_size - seek_pos
        else:
            sub_file_length = fragments[i+1] - seek_pos
            
        with open(str(i) + '.bin', 'wb') as outf:
            inf.seek(seek_pos)
            outf.write(inf.read(sub_file_length))