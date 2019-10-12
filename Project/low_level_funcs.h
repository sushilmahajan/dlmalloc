#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "error.h"

size_t get_start_address(uint32_t pid);

size_t get_end_address(void);

size_t expand_heap(uint32_t size);

void print_heap(size_t start, size_t end);

