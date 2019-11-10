#include "sys_calls.h"

void *get_start_address(void) {
    char pid_folder[10], line[200], location[50] = "/proc/";
    size_t start_address;
    bool found;

    uint32_t pid = getpid();
    sprintf(pid_folder, "%d", pid);
    strcat(location, pid_folder);
    strcat(location, "/maps");

    FILE *fp;
    if ((fp = fopen(location, "r")) == NULL) {
        /* Program doesn't have access rights */
        fprintf(stderr, "Error: Access denied\n");
        return NULL;
    }
    while (!(found = strstr(line, "[heap]")) && !feof(fp)) {
        fscanf(fp, "%[^\n]%*c", line);
    }
    fclose(fp);

    if (!found) {
        fprintf(stderr, "Error: Start address couldn't be found\n");
        return NULL;
    }

    char *start_addr_str = strtok(line, "-");
    start_address = strtoull(start_addr_str, NULL, 16);

    return (void*)start_address;
}

void *get_end_address(void) {
    void *ptr = sbrk(0);
    if (ptr == (void*)(-1)) {
        return NULL;
    }
    return ptr;
}

void* get_mem_from_sys(size_t size) {
    int8_t allocated;
    void *ptr = get_end_address();
    allocated = brk(ptr + size);
    if (allocated == -1) {
        perror("Error");
        return NULL;
    }
    ptr = get_end_address();
    return ptr;
}

uint32_t return_mem_to_sys(void *ptr);
