#ifndef __NVME_PCI_H__
#define __NVME_PCI_H__
int mynvme_pcicfg_map_bar(char *bdf, unsigned int bar,
				      bool read_only, void **mapped_addr);

#endif