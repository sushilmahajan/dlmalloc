#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include "bins.h"

typedef struct node {
    int data;
    struct node *next;
} node;

int main() { 
    node *a = (node*)my_malloc(768);
    node *b = (node*)my_malloc(16);
    node *c = (node*)my_malloc(134272);
    my_free(a);
    a = (node*)my_malloc(504);
    return 0;
}
//    for (int i = 464; i <= 528; i++)
//        printf("%d %d\n", i, PAD_REQUEST(i));
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
   
