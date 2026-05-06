import os
import platform
import struct
import re
import sys

if len(sys.argv) != 3:
    print("!!not set input file")
    sys.exit(0)

input_file = sys.argv[1]
output_file = sys.argv[2]

if os.path.exists(input_file) == False:
    print("!!not found " + input_file)
    sys.exit(0)

ff = open(input_file, encoding='utf8')
out = open(output_file, "wb")

line = ff.readline()
while line:
    result_list = line.split(",")
    c0 = result_list[1].strip()
    d0 = result_list[2].strip()
    c = int(c0,16)
    d = int(d0,16)
    result_list1 = re.findall(r'["](.*?)["]', line)
    a = result_list1[0]
    e = 1
    if "UNLOCKED" in line:
        e = 0
    byte=struct.pack('16s3I',bytes(a,'UTF-8'),c,d,e)
    out.write(byte)
    line = ff.readline()
out.close()
ff.close()