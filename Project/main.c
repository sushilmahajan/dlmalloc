#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include "bins.h"

int main() { 
    int *a = (int*)my_calloc(64, 4);
    int *b = (int*)my_calloc(64, 4);
    int *c = (int*)my_calloc(64, 4);
    int *d = (int*)my_calloc(64, 4);
    my_free(c);
    my_free(b);
    my_free(a);
    a = (int*)my_calloc(200, 4);
    return 0;
}
//    chunk c1 = {640, 480};
//    chunk c2 = {480, 496};
//    chunk c3 = {128, 304};
//    chunk c4 = {128, 304};
//    chunk c5 = {128, 304};
//    
//    insert_small_chunk(g_mstate, &c1, 480);
//    insert_small_chunk(g_mstate, &c2, 496);
//    insert_small_chunk(g_mstate, &c3, 304);
//    insert_small_chunk(g_mstate, &c4, 304);
//    insert_small_chunk(g_mstate, &c5, 304);
   
