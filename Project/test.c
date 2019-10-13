#include <stdlib.h>
#include <stdio.h>
#include "heap_man.h"

int main(void) 
{ 
    void *addr1, *addr2;
    size_t ptr;
    uint32_t pid = getpid(), size;
    printf("%d\n", pid);
    addr1 = get_start_address(pid);
    if (addr1 == (void*)1) {
        return 1;
    }
    addr2 = get_end_address();
    size = expand_heap(&ptr, MMAP_THRESHOLD + 1024);
    printf("%p %lx %d\n", addr2, ptr, size/1024);
    return 0; 
} 
