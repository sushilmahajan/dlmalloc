*******************************************************************************
                                HEAP MANAGER
*******************************************************************************

# OS calls
1. Get the starting address of heap allocated to the program from
   /proc/pid/maps file.
2. Get the end address of heap(program break) from sbrk(0)
3. Increase the heap size by brk system call if size of requested memory is 
   less than 128 KB & no free block is available in existing heap 
4. If requested size is greater than 128 KB & no free block is available in 
   existing heap, allocate non contiguous block of memory by mmap system call
5. Shrink the heap size if amount of contiguous free memory becomes greater
   than some threshold value

# Data Structures 
1. Heap list - A single process/program can have multiple heaps(non contiguous 
               regions)
2. Free chunk list - A heap region will have free as well as allocated blocks.
                     In order to identify free blocks a list of them will be
                     used.

