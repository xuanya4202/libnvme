#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "libnvme/nvme.h"
#include "nvme/nvme_internal.h"

int main(int argc, char **argv) {
	  nvme_log_level log_level = NVME_LOG_DEBUG;
    char *dev = argv[1];
	  struct nvme_ctrlr *ctrlr;
	  struct nvme_qpair *qp;
	  struct nvme_ns		*ns;
    struct nvme_ns_stat ns_stat;
    
    struct cb_args {
        bool done;
        int ret;
    }; 
    auto nvme_cmd_cb_func =[](void *cmd_cb_arg, const struct nvme_cpl *cpl_status)->void {
          struct cb_args *cb_args = (struct cb_args *)cmd_cb_arg;
          cb_args->done = true;
          cb_args->ret = cpl_status->status.sc;
    };
    
    auto a = 1;
    int ret = nvme_lib_init(log_level, NVME_LOG_STDOUT, NULL);
    if (ret != 0) {
    	fprintf(stderr,
    		"libnvme init failed %d (%s)\n",
    		ret, strerror(-ret));
    	exit(1);
    }

    printf("Opening NVMe controller %s\n", dev);
    ctrlr = nvme_ctrlr_open(dev, NULL);
    if (!ctrlr) {
    	fprintf(stderr, "Open NVMe controller %s failed\n",
    		dev);
    	return -1;
    }
	  
    /* Open the name space */
	  ns = nvme_ns_open(ctrlr, 1);
	  if (!ns) {
	  	printf("Open NVMe controller"
	  	       "name space %u failed\n",
	  	       1);
	  	return -1;
	  }

	  if (nvme_ns_stat(ns, &ns_stat) != 0) {
	  	printf("Get name space %u info failed\n", 1);
	  	return -1;
	  }
    printf("sector size: %u sectors: %ld\n", ns_stat.sector_size, ns_stat.sectors);

		qp = nvme_ioqp_get(ctrlr, NVME_QPRIO_URGENT, 0);
    if (qp == NULL) {
      printf("Get I/O queue pair failed\n");
      return -1;
    }
    printf("I/O queue pair ID: %u\n", qp->id);
		
    struct nvme_qpair *qp2 = nvme_ioqp_get(ctrlr, NVME_QPRIO_URGENT, 0);
    if (qp2 == NULL) {
      printf("Get I/O queue pair failed\n");
      return -1;
    }
    printf("I/O queue pair ID: %u\n", qp2->id);
    
    void* w_buf = nvme_zmalloc(4096, 4096);
    memcpy(w_buf, "hello world", 11);
    void* r_buf = nvme_zmalloc(4096, 4096);
		
    struct cb_args w_args;
    struct cb_args r_args;
    memset(&w_args, 0, sizeof(w_args));
    memset(&r_args, 0, sizeof(r_args));

    w_args.done = false; 
    ret = nvme_ns_write(ns, qp,
					    w_buf,
					    0,
					    1,
					    nvme_cmd_cb_func, &w_args, 0);
    
		while (!w_args.done) {
        nvme_ioqp_poll(qp, 1);
    }
    printf("Write done. ret=%d\n", w_args.ret);
    
    r_args.done = false; 
    ret = nvme_ns_read(ns, qp2,
					    r_buf,
					    0,
					    1,
					    nvme_cmd_cb_func, &r_args, 0);
		while (!r_args.done) {
        nvme_ioqp_poll(qp2, 1);
    }
    printf("read done. ret=%d\n", r_args.ret);
    printf("read buf: %s\n", r_buf);
    if(memcmp(w_buf, r_buf, 4096) == 0) {
        printf("Write and read data match\n");
    } else {
        printf("Write and read data mismatch\n");
    }
    
    nvme_ioqp_release(qp);
	  nvme_ns_close(ns);
		nvme_ctrlr_close(ctrlr);
    return 0;
}
