/**
 *  \file heap_man.h
 */

#ifndef HEAP_MAN_H_INCLUDED 
#define HEAP_MAN_H_INCLUDED 1

#include "low_level_funcs.h"

/** \brief Allocates a block of given size in heap by finding suitable sized
 *         free block from heap 
 *  \param size Size of memory to be allocated 
 *  \return Pointer to the first byte of the allocated block
 * */
void *my_malloc(size_t size);

/** \brief Allocates a block of given size in heap by finding suitable sized
 *         free block from heap & initializes the memory with zero 
 *  \param num Number of elements to be allocated 
 *  \param size Size of each element
 *  \return Pointer to the first byte of the allocated block
 * */
void *my_calloc(size_t num, size_t size);

/** \brief Expands or shrinks size of the previously allocated block
 *  \param ptr Pointer to the previously allocated block 
 *  \param new_size New size of memory block 
 *  \return Pointer to the first byte of newly sized block
 * */
void *my_realloc(void *ptr, size_t new_size);

/** \brief Frees a block of memory from heap  
 *  \param ptr Pointer to the block of memory to be freed 
 *  \return nothing 
 * */
void my_free(void *ptr);

#endif
