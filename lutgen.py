#!/usr/bin/python3

import pymmh3
import sys


with open(sys.argv[1],"r") as f:
    for s in f.readlines():
        s = s.replace('\n','').replace('\r','')
        print(f'{int.from_bytes(pymmh3.hash(s).to_bytes(4,"big",signed=True),"little"):08X}:{s}')