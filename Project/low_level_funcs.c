#include "low_level_funcs.h"

void *get_start_address(uint32_t pid) {
    char pid_folder[10], line[200], location[50] = "/proc/";
    size_t start_address;
    bool found;

    sprintf(pid_folder, "%d", pid);
    strcat(location, pid_folder);
    strcat(location, "/maps");

    FILE *fp;
    if ((fp = fopen(location, "r")) == NULL) {
        /* Program doesn't have access rights */
        fprintf(stderr, "Error: Access to heap denied\n");
        return (void*)ERROR_ACCESS_PID;
    }
    while (!(found = strstr(line, "[heap]")) && !feof(fp)) {
        fscanf(fp, "%[^\n]%*c", line);
    }
    fclose(fp);

    if (!found) {
        fprintf(stderr, "Error: Start address couldn't be found\n");
        return (void*)ERROR_ADDRESS_NOT_FOUND;
    }

    char *start_addr_str = strtok(line, "-");
    start_address = strtoull(start_addr_str, NULL, 16);

    return (void*)start_address;
}

void *get_end_address(void) {
    return sbrk(0);
}

uint32_t get_mem_form_sys(size_t *new_start_ptr, uint32_t size) {
    *new_start_ptr = (size_t)get_end_address();
    int8_t allocated;
    /* If size is greater than 128 KB, use mmap to allocate another(non
     * contigulus) block of memory which will be linked to original heap TODO 
     * */
    if (size < MMAP_THRESHOLD) {
        allocated = brk(new_start_ptr + size);
        if (allocated == -1) {
            perror("Error");
            return errno;
        }
        return size;
    }

#if MMAP_ENABLED
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
#endif /* MMAP_ENABLED */
}

uint32_t return_mem_to_sys(void *ptr);

void print(void* start, void* end) {
    char *ptr = (char*)start, *end_ptr = (char*)end;
    while (ptr < end_ptr) {
        printf("%d", *ptr);
    }
}
