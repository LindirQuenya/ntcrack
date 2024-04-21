#ifndef MD4_FPGA_LIB_H
#define MD4_FPGA_LIB_H

#if HAVE_VISIBILITY && BUILDING_LIBMD4FPGA
# define LIBMD4FPGA_EXPORTED __attribute__((__visibility__("default")))
#else
# define LIBMD4FPGA_EXPORTED
#endif

typedef struct {
	int fd;
	volatile void *ptr;
} mmapprops_t;

LIBMD4FPGA_EXPORTED void open_mmap(mmapprops_t *empty);
LIBMD4FPGA_EXPORTED void close_mmap(mmapprops_t *props);
LIBMD4FPGA_EXPORTED void compress_fpga(volatile void *ptr, unsigned int *state, const unsigned char *x);

#endif
