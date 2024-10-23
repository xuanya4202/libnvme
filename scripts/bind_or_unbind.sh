#!/bin/bash

function get_driver_name() {
    local bdf=$1
    if [ -e "/sys/bus/pci/devices/$bdf/driver" ]; then
        local driver_nvme=$(basename $(readlink /sys/bus/pci/devices/$bdf/driver))
        echo "Driver $bdf bound $driver_nvme"
    else
        echo "Driver $bdf not bound"
    fi
}
function linux_unbind_driver() {
    bdf="$1"
    ven_dev_id=$(lspci -n -s $bdf | cut -d' ' -f3 | sed 's/:/ /')

    echo "$ven_dev_id" >"/sys/bus/pci/devices/$bdf/driver/remove_id" 2>/dev/null || true
    echo "$bdf" >"/sys/bus/pci/devices/$bdf/driver/unbind"
    echo "Driver $bdf unbound"
}

function linux_bind_driver() {
    bdf="$1"
    driver_name="$2"

    ven_dev_id=$(lspci -n -s $bdf | cut -d' ' -f3 | sed 's/:/ /')
    echo "$ven_dev_id" >"/sys/bus/pci/drivers/$driver_name/new_id" 2>/dev/null || true
    echo "$bdf" >"/sys/bus/pci/drivers/$driver_name/bind" 2>/dev/null || true

    if [ $driver_name == "vfio-pci" ]; then
        iommu_group=$(basename $(readlink -f /sys/bus/pci/devices/$bdf/iommu_group))
        if [ -e "/dev/vfio/$iommu_group" ]; then
            if [ -n "$TARGET_USER" ]; then
                chown "$TARGET_USER" "/dev/vfio/$iommu_group"
            fi
        fi
    fi
    echo "Driver $bdf bound to $driver_name"
}

bdf="$1"
if [ $# -eq 1 ]; then
    get_driver_name $bdf
elif [ $# -eq 2 ]; then
    driver_name="$2"
    if [ $driver_name == "unbind" ]; then
        linux_unbind_driver $bdf
    else
        linux_bind_driver $bdf $driver_name
    fi
else
    echo "Usage: $0 <bdf> [driver]"
    echo " eg get bind: $0 0000:00:02.0"
    echo " eg bind: $0 0000:00:02.0 vfio-pci"
    echo " eg bind: $0 0000:00:02.0 nvme"
    echo " eg unbind: $0 0000:00:02.0 unbind"
    exit 1
fi