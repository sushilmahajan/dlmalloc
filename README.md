****************************************************************************
                                HEAP MANAGER
****************************************************************************

# OS calls
1. Get the starting address of heap allocated to the program from
   /proc/pid/maps file.
2. Get the end address of heap(program break) from sbrk(0)
3. Increase the heap size by brk system call if size of requested memory is 
   less than 128 KB & no free block is available in existing heap 
4. Shrink the heap size if amount of contiguous free memory becomes greater
   than some threshold value

# Data Structures 
1. Circular doubly linked list - 
        If size of free block is less than 256 bytes, a circular doubly 
    linked list will be used to link such blocks of equal size. 32 such lists
    will be used, 1 corresponding to each smallbin.
2. Tree -
        If size of free block is greater than 256 bytes, a tree will be used to
    link such blocks.

# Algorithms -
1. Best fit -               
        Uses a list of free blocks & searches for free block with size just 
    greater than required size 
