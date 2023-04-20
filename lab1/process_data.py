from pathlib import Path
import re

real_time = re.compile(r'real\s0m(\d+\.\d+)s')
user_time = re.compile(r'user\s0m(\d+\.\d+)s')
sys_time = re.compile(r'sys\s0m(\d+\.\d+)s')

threads_pattern = re.compile(r'num_thread=(\d+)')

data_dict = dict()

for t in ["2","4","6","8","10"]:
    data_dict[t] = dict(
        serial = dict(
            real = 0,
            user = 0,
            sys = 0
        ),
        parallel = dict(
            real = 0,
            user = 0,
            sys = 0
        )
    )

# a = real_time.findall('real	0m1.099s\n')
# for all .log files in the current directory
for file in Path('.').glob('*.out'):
    print(f"Processing {file}")
    with open(file, 'r') as f:
        current = "serial"
      
        lines = f.readlines()
        threads = ""
        for line in lines:
            # update threads
            threads_found = threads_pattern.findall(line)
            if threads_found:
                threads = str(int(threads_found[0]))
            
            if line.find("serial") != -1:
                current = "serial"
            elif line.find("parallel") != -1:
                current = "parallel"
            
            # update time
            real_found = real_time.findall(line)
            if real_found:
                data_dict[threads][current]['real'] += float(real_found[0])
            
            user_found = user_time.findall(line)
            if user_found:
                data_dict[threads][current]['user'] += float(user_found[0])

            sys_found = sys_time.findall(line)
            if sys_found:
                data_dict[threads][current]['sys'] += float(sys_found[0])

# divide all results by 4
for threads in data_dict:
    for mode in data_dict[threads]:
        for time in data_dict[threads][mode]:
            data_dict[threads][mode][time] /= 4

# print results
print("Threads\tMode\tReal\tUser\tSys")
for threads in data_dict:
    for mode in data_dict[threads]:
        # keep 3 decimal places
        print(f"{threads}\t{mode}\t{data_dict[threads][mode]['real']:.3f}\t{data_dict[threads][mode]['user']:.3f}\t{data_dict[threads][mode]['sys']:.3f}")

# output csv
with open("data.csv", 'w') as f:
    f.write("Threads\tMode\tReal\tUser\tSys")
    for threads in data_dict:
        for mode in data_dict[threads]:
            f.write(f"{threads}\t{mode}\t{data_dict[threads][mode]['real']}\t{data_dict[threads][mode]['user']}\t{data_dict[threads][mode]['sys']}")