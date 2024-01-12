import re

r_count = 0
w_count = 0
Rtypes = set()
Wtypes = set()

with open('/home/femu/StRAID/straid/code/Traces/fileserver_1.log') as f:
  for line in f:
    if re.search(r'^R', line):
      r_count += 1
      Rtypes.add(line.split()[-1])   
    elif re.search(r'^W', line):
      w_count += 1
      Wtypes.add(line.split()[-1])

print('R line count:', r_count)  
print('W line count:', w_count)
print('R Last number types:', list(Rtypes))
print('W Last number types:', list(Wtypes))