/**
 *  \file low_level_funcs.h
 */

#ifndef LOW_LEVEL_FUNCS_H_INCLUDED 
#define LOW_LEVEL_FUNCS_H_INCLUDED 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "error.h"

#define MMAP_ENABLED            (0)
#define MMAP_THRESHOLD          (128 * 1024)

/** \brief Reads the heap address from file /proc/pid/maps 
 *  \param pid Process id of current process
 *  \return Start address of heap of current process
 * */
void *get_start_address(uint32_t pid);

/** \brief Gives end address of data segment(including heap)
 *  \return End address of heap of current process
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

/** \brief Prints the contents of heap memory between given addresses, Using
 *         this to verify allocation.
 *  \param start Start address
 *  \param end Address upto which contents will be printed 
 *  \return nothing
 * */
void print(void* start, void* end);

#endif
