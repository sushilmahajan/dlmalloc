#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED 1

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/** \brief Prints the contents of heap memory between given addresses, Using
 *         this to verify allocation.
 *  \param start Start address
 *  \param end Address upto which contents will be printed 
 *  \return nothing
 * */
void print(void* start, void* end);

#endif 
