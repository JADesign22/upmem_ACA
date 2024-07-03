#!/bin/env python3

from dpu import DpuSet
from sys import stdout

with DpuSet(nr_dpus=1, binary="helloworld_bin", log=stdout) as dpu:
    dpu.exec()