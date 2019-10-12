#include <stdlib.h>
#include <stdio.h>
#include "heap_man.h"

int main(void) 
{ 
    size_t addr1, addr2, addr3;
    uint32_t pid = getpid();
    printf("%d\n", pid);
    addr1 = get_start_address(pid);
    addr2 = get_end_address();
    addr3 = expand_heap(1024);
    printf("%lu %lu %lu\n", addr1, addr2, addr3);
    printf("%lu\n", addr3-addr2);
    return 0; 
} 
