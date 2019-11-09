#include "debug.h"

void print(void* start, void* end) {
    int *ptr = (int*)start, *end_ptr = (int*)end;
    while (ptr < end_ptr) {
        printf("%d ", *ptr);
        ptr++;
    }
}
