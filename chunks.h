#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED 1

#include <stdint.h>
#include <stdlib.h>

typedef size_t index_t;

typedef struct chunk {
    size_t prev_foot;
    size_t head;
    struct chunk *prev;
    struct chunk *next;
} chunk;

typedef struct tree_chunk {            
    size_t prev_foot;
    size_t head;     
    struct tree_chunk *prev;
    struct tree_chunk *next;                                                               
    struct tree_chunk *left;
    struct tree_chunk *right;
} tchunk;                                                                            

typedef chunk* chunkptr;

typedef tchunk* tchunkptr;

#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZEOF_SIZE_T       (sizeof(size_t))
#define SIZEOF_TWO_SIZE_T   (SIZEOF_SIZE_T * 2)
#define SIZE_T_BITSIZE      (64)

#define MALLOC_ALIGNMENT    (SIZEOF_TWO_SIZE_T)
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - 1)
#define CHUNK_SIZE          (sizeof(chunk))

/* This overhead is due to header & footer each of size 8 at start & end of
 * chunk respectively */
#define CHUNK_OVERHEAD      (SIZEOF_TWO_SIZE_T)

/* The smallest sized chunk can be of this size, thus this is the minimum size
 * that can be malloced */              
#define MIN_CHUNK_SIZE      (CHUNK_SIZE)

/* Conversion from chunk headers to user pointers, and vice versa */                
#define CHUNK_TO_MEM(p)     ((void*)((char*)(p) + SIZEOF_TWO_SIZE_T))     
#define MEM_TO_CHUNK(mem)   ((chunkptr)((char*)(mem) - SIZEOF_TWO_SIZE_T))     

/* Treat space at some distance relative to one chunk as another chunk */                                 
#define CHUNK_PLUS_OFFSET(p, s)  ((chunkptr)(((char*)(p)) + (s)))             
#define CHUNK_MINUS_OFFSET(p, s) ((chunkptr)(((char*)(p)) - (s)))             

/* Pad bytes requested by user into allocatable chunk size */                                     
#define PAD_SIZE(bytes)     \
    (((bytes) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)           

/* Flag bits in a chunk */
#define P_USED_BIT          (SIZE_T_ONE)                                       
#define C_USED_BIT          (SIZE_T_TWO)                                       

/* Check whether flag bits are set */                                     
#define C_USED(p)           ((p)->head & C_USED_BIT)                           
#define P_USED(p)           ((p)->head & P_USED_BIT)   

/* Gives the chunksize of a chunk by reading its header */
#define CHUNKSIZE(p)        ((p)->head & ~(CHUNK_ALIGN_MASK))                          

/* Sets the chunksize of a chunk by writing to its header in memory */
#define SET_CHUNK_SIZE(p, s)    \
    ((p)->head = ((s) | ((p)->head & CHUNK_ALIGN_MASK)))   

/* Clear the flag bits(required when chunk is freed) */
#define CLEAR_P_USED(p)     ((p)->head &= ~P_USED_BIT)                         
#define CLEAR_C_USED(p)     ((p)->head &= ~C_USED_BIT)                         

/* Set the flag bits(required when chunk is allocated) */
#define SET_P_USED(p)     ((p)->head |= P_USED_BIT)
#define SET_C_USED(p)     ((p)->head |= C_USED_BIT)

/* Pointer to next or previous physical chunk in memory */                           
#define NEXT_CHUNK(p) ((chunkptr)( ((char*)(p)) + ((p)->head & ~CHUNK_ALIGN_MASK))) 
#define PREV_CHUNK(p) ((chunkptr)( ((char*)(p)) - ((p)->prev_foot) ))         

/* Get/set size at footer */                                                   
#define GET_FOOT(p, s)  (NEXT_CHUNK(p)->prev_foot)           
#define SET_FOOT(p, s)  (NEXT_CHUNK(p)->prev_foot = (s))     

/* Set c_used bit of p chunk & p_used bit of chunk next to p(required when 
 * chunk is allocated) */                                           
#define SET_C_USED_AND_NEXT_P_USED(p)   \
{                                       \
    SET_C_USED(p);                      \
    SET_P_USED(NEXT_CHUNK(p));          \
}

/* Set c_used bit of p chunk & p_used bit of chunk next to p(required when 
 * chunk is freed) */                                           
#define CLEAR_C_USED_AND_NEXT_P_USED(p) \
{                                       \
    CLEAR_C_USED(p);                    \
    CLEAR_P_USED(NEXT_CHUNK(p));        \
}

/* Abort if chunk is not in use & passed to realloc / free */
#define CHECK_INUSE_CHUNK(p)                    \
{                                               \
    assert(C_USED(p));                          \
    assert(P_USED(NEXT_CHUNK(p)));              \
}

/* Check adjacent chunks in physical memory */
#define IS_PREV_FREE(p)                 (!P_USED(p))
#define IS_NEXT_FREE(p)                 (!C_USED(NEXT_CHUNK(p)))

#endif
