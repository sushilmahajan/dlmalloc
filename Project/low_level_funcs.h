#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "error.h"

#define MMAP_THRESHOLD      (128 * 1024)

void *get_start_address(uint32_t pid);

void *get_end_address(void);

uint32_t expand_heap(size_t *new_start_ptr, uint32_t size);

void print(size_t start, size_t end);

