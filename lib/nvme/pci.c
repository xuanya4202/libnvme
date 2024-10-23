
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define SYS_BUS_PCI "/sys/bus/pci/devices"


// 获取设备资源大小
static uint64_t
pci_device_resource_size(char *bdf) {
    char     name[256];
    char     resource[512];
    int fd;
    uint64_t bytes;
    char *next;
    
    memset(name, 0, 256);
    snprintf(name, 255, "%s/%s/resource",
         SYS_BUS_PCI,
         bdf);
    
    fd = open(name, O_RDONLY);
    if (fd == -1) {
        return 0;
    }
    
    memset(resource, 0, 512);
    bytes = read( fd, resource, 512 );
    resource[511] = '\0'; 
    close( fd );
    
    next = resource;
    uint64_t low_addr = strtoull( next, & next, 16 );
    uint64_t high_addr = strtoull( next, & next, 16 );
    uint64_t flags = strtoull( next, & next, 16 );
    uint64_t rom_size = (high_addr - low_addr) + 1;
    return rom_size;
}

int mynvme_pcicfg_map_bar(char *bdf, unsigned int bar,
				      bool read_only, void **mapped_addr) {
    char name[256];
    int fd;
    int err = 0;
    //size_t map_size = 64 * 1024;
    size_t map_size;

    map_size = pci_device_resource_size(bdf);
    if (map_size == 0) {
        return -1;
    }

    memset(name, 0, 256);
    snprintf(name, 255, "%s/%s/resource%u",
         SYS_BUS_PCI,
         bdf,
         bar);

    fd = open(name, O_RDWR);
    if (fd == -1) {
        return -1;
    }


    void *memory = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        memory = NULL;
	      close(fd);
	      return -1;
    }
    *mapped_addr = memory;
    return 0;
}