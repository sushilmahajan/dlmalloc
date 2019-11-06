#include "debug.h"

void print(void* start, void* end) {
    char *ptr = (char*)start, *end_ptr = (char*)end;
    while (ptr < end_ptr) {
        printf("%d", *ptr);
        ptr++;
    }
}
