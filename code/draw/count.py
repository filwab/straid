import re

r_count = 0
w_count = 0
types = set()

with open('/home/femu/StRAID/straid/code/Traces/mytest.log') as f:
  for line in f:
    if re.search(r'^R', line):
      r_count += 1
      types.add(line.split()[-1])   
    elif re.search(r'^W', line):
      w_count += 1
      types.add(line.split()[-1])

print('R line count:', r_count)  
print('W line count:', w_count)
print('Last number types:', list(types))