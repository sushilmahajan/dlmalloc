/**
 *  \file sys_calls.h
 */

#ifndef LOW_LEVEL_FUNCS_H_INCLUDED 
#define LOW_LEVEL_FUNCS_H_INCLUDED 1

#include "error.h"
#include "debug.h"

#define MMAP_ENABLED            (0)
#define MMAP_THRESHOLD          (128 * 1024)

/** \brief Reads the start address of heap memory from file 
 * /proc/pid/maps 
 *  \param pid Process id of current process/program
 *  \return Start address of heap of current process
 * */
void *get_start_address(uint32_t pid);

/** \brief Gives end address of heap memory of current process using sbrk 
 * system call 
 *  \return End address of heap memory
 * */
void *get_end_address(void);

/** \brief Increases heap size by brk or mmap(if MMAP_ENABLE is 1) depending on 
 * size of requested data
 *  \param *new_start_ptr Address of variable which will contain pointer to
 *                        newly allocated block
 *  \param size Size of new block to be allocated 
 *  \return size if memory was allocated successfully, 
 *          0 if nothing was allocated
 * */
uint32_t get_mem_from_sys(size_t *new_start_ptr, uint32_t size);

/** \brief Shrinks heap size by brk
 *  \param ptr Address of free block after which memory has to be deallocated
 *  \return 0 if memory was deallocated successfully, 
 *          Error code if deallocation fails
 * */
uint32_t return_mem_to_sys(void *ptr);

#endif
