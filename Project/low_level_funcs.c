#include "low_level_funcs.h"

void *get_start_address(uint32_t pid) {
    char pid_folder[10], line[200], location[50] = "/proc/";
    size_t start_address;

    sprintf(pid_folder, "%d", pid);
    strcat(location, pid_folder);
    strcat(location, "/maps");

    FILE *fp;
    if ((fp = fopen(location, "r")) == NULL) {
        fprintf(stderr, "Error: Access to heap denied\n");
        return (void*)ERROR_ACCESS_PID;
    }
    while (!strstr(line, "[heap]")) {
        fscanf(fp, "%[^\n]%*c", line);
    }
    fclose(fp);

    char *start_addr_str = strtok(line, "-");
    start_address = strtoull(start_addr_str, NULL, 16);

    return (void*)start_address;
}

void *get_end_address(void) {
    return (void*)(sbrk(0));
}

uint32_t expand_heap(size_t *new_start_ptr, uint32_t size) {
    *new_start_ptr = (size_t)get_end_address();
    int8_t allocated;
    if (size < MMAP_THRESHOLD) {
        allocated = brk(new_start_ptr + size);
        if (allocated == -1) {
            perror("Error");
            return errno;
        }
        return size;
    }
    else {
        size_t pagesize = getpagesize();
        uint32_t num_of_pages = size / pagesize + 1;
        
        *new_start_ptr = (size_t)mmap(new_start_ptr + MMAP_THRESHOLD,
                num_of_pages * pagesize,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANON | MAP_PRIVATE,
                0,
                0
                );

        if (*new_start_ptr == -1) {
            perror("Error");
            return errno;
        }
        return size;
    }
}

void print(size_t start, size_t end) {
    char *ptr = (char*)start, *end_ptr = (char*)end;
    while (ptr < end_ptr) {
        printf("%d", *ptr);
    }
}
