#!/bin/bash

function dev_to_bdf() {
    # get bdf, arg: nvme0 nvme1
    local dev=$1
    local bdf=$(basename $(readlink /sys/class/nvme/$dev/device))
    echo $bdf
}

function bdf_to_dev() {
  # get dev, arg: 0000:f2:00.0
  local bdf=$1
  local dev=$(basename /sys/bus/pci/devices/"$bdf"/nvme/nvme*)
  echo $dev
}

arg=$1

if [[ $arg == *"nvme"* ]]; then
    dev_to_bdf $arg
else
    bdf_to_dev $arg
fi