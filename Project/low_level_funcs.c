#include "low_level_funcs.h"

size_t get_start_address(uint32_t pid) {
    char pid_folder[10], line[200], location[50] = "/proc/";
    size_t start_address;

    sprintf(pid_folder, "%d", pid);
    strcat(location, pid_folder);
    strcat(location, "/maps");

    FILE *fp;
    if ((fp = fopen(location, "r")) == NULL) {
        fprintf(stderr, "Error: Access to heap denied\n");
        return ERROR_ACCESS_PID;
    }
    while (!strstr(line, "[heap]")) {
        fscanf(fp, "%[^\n]%*c", line);
    }
    fclose(fp);

    char *start_addr_str = strtok(line, "-");
    start_address = strtoull(start_addr_str, NULL, 16);

    return start_address;
}

size_t get_end_address(void) {
    return (size_t)(sbrk(0));
}

size_t expand_heap(uint32_t size) {
    size_t end_address = get_end_address();
    int8_t allocated = -1;

    while ((allocated == -1) && size) {
        allocated = brk((void*)end_address + size); 
        size /= 2;
    }
    return get_end_address();
}

void print_heap(size_t start, size_t end) {
    char *ptr = (char*)start, *end_ptr = (char*)end;
    while (ptr < end_ptr) {
        printf("%d", *ptr);
    }
}

