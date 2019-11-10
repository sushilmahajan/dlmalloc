/**
 *  \file sys_calls.h
 */

#ifndef LOW_LEVEL_FUNCS_H_INCLUDED 
#define LOW_LEVEL_FUNCS_H_INCLUDED 1

#include "error.h"
#include "debug.h"
#include <stdlib.h>

/** \brief Reads the start address of heap memory from file 
 * /proc/pid/maps 
 *  \return Start address of heap of current process
 * */
void *get_start_address(void);

/** \brief Gives end address of heap memory of current process using sbrk 
 * system call 
 *  \return End address of heap memory
 * */
void *get_end_address(void);

/** \brief Increases heap size by brk system call by SYSTEM_ALLOCATION_SIZE
 * bytes
 *  \param size Number of bytes by which heap size is to be increased
 *  \return New end address if memory was allocated successfully, 
 *          NULL if nothing was allocated/error occurs while system allocation
 * */
void* get_mem_from_sys(size_t size);

/** \brief Shrinks heap size by brk
 *  \param ptr Address of free block after which memory has to be deallocated
 *  \return 0 if memory was deallocated successfully, 
 *          Error code if deallocation fails
 * */
uint32_t return_mem_to_sys(void *ptr);

#endif
