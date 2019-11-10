#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include "bins.h"

int main() { 
    int *a = (int*)my_malloc(0);
//    int *b = (int*)my_malloc(16);
//    int *c = (int*)my_malloc(134528);
    my_free(a);
    a = (int*)my_malloc(496);
    return 0;
}
