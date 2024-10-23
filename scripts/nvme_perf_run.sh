#!/bin/bash
# -rw 读比例, 写比例=1-读比例
# -rnd 随机
# 4096 io size
./nvme_perf -rw 100 -rnd pci://0000:f2:00.0 4096
