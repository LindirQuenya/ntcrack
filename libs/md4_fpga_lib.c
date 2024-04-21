// Modified from https://github.com/erickrenz/peekpoke/blob/main/libs/peekpoke.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>

#include "md4_fpga_lib.h"

// This is just after a memory page boundary.
// TODO: this might not be true for other systems, but is
// for the PynqZ2 board. To check, run `getconf PAGE_SIZE`
// and confirm that it is 4096.
// You might need to rewrite some of the mmap stuff if it
// returns something different, possibly to use two pages
// if the peripheral happens to lie across a page boundary.
#define BASE_ADDR 0x40000000

// All of these were found by looking at the pynq register
// dictionary properties in Python.
// The width ones are kinda implied by the type, so I don't
// actually use them. If a uint32_t ever stops being 4 bytes,
// let me know. Also, if you have a different width, change
// the type of all the data to e.g. uint64_t.
#define CTRL_OFFSET 0
#define RETURN_OFFSET 16
#define RETURN_WIDTH 4
#define RETURN_COUNT 4
#define STATE_OFFSET 36
#define STATE_WIDTH 4
#define STATE_COUNT 4
#define X_OFFSET 56
#define X_WIDTH 4
#define X_COUNT 16

// To reduce the number of magic constants.
#define PAGE_SIZE 4096

void open_mmap(mmapprops_t *empty) {
	empty->fd = open("/dev/mem", O_RDWR);
	if (empty->fd < 1) {
		perror("md4fpga: unable to open /dev/mem");
		exit(EXIT_FAILURE);
	}
	empty->ptr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, empty->fd, (BASE_ADDR & ~(PAGE_SIZE - 1)));
	if (empty->ptr == (void*) -1) {
		perror("md4fpga: unable to map memory");
		exit(EXIT_FAILURE);
	}
}

void close_mmap(mmapprops_t *props) {
	if (munmap((void *)props->ptr, PAGE_SIZE) < 0) {
		perror("md4fpga: unable to unmap memory");
		exit(EXIT_FAILURE);
	}

	if (close(props->fd) < 0) {
		perror("md4fpga: unable to close /dev/mem");
		exit(EXIT_FAILURE);
	}
}

// buf has length >= RETURN_COUNT.
inline void read_out(volatile void *ptr, uint32_t *buf) {
	memcpy(buf, (uint32_t *)(ptr + RETURN_OFFSET), RETURN_COUNT*sizeof(uint32_t));
}

inline void write_state(volatile void *ptr, uint32_t *buf) {
	memcpy((uint32_t *)(ptr + STATE_OFFSET), buf, STATE_COUNT*sizeof(uint32_t));
}

inline void write_X(volatile void *ptr, uint32_t *buf) {
	memcpy((uint32_t *)(ptr + X_OFFSET), buf, X_COUNT*sizeof(uint32_t));
}

inline void write_X_byte(volatile void *ptr, uint8_t *buf) {
	memcpy((uint8_t *)(ptr + X_OFFSET), buf, X_COUNT*sizeof(uint32_t));
}

inline void start_digest(volatile void *ptr) {
	*((char *)(ptr + CTRL_OFFSET)) |= 1;
}

inline int wait_digest(volatile void *ptr) {
	int i = 0;
	while (!(*((char *)(ptr + CTRL_OFFSET)) & 2)){i++;}
	return i;
}

void compress_fpga(volatile void *ptr, uint32_t *state, uint8_t *x) {
	write_state(ptr, state);
	write_X_byte(ptr, x);
	start_digest(ptr);
	wait_digest(ptr);
	read_out(ptr, state);
}
