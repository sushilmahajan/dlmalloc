#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include "bins.h"

int main(void) 
{ 
    chunk c1 = {64, 32};
    chunk c2 = {48, 32};
    chunk c3 = {128, 32};
    insert_small_chunk(g_mstate, &c1, 32);
    insert_small_chunk(g_mstate, &c2, 32);
    insert_small_chunk(g_mstate, &c3, 32);
    remove_small_chunk(g_mstate, &c1, 32);
    remove_small_chunk(g_mstate, &c2, 32);
    remove_small_chunk(g_mstate, &c3, 32);
    return 0;
} 
