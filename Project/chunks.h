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
    /* First 4 parameter names should be same as chunk */    
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
#define CHUNK_OVERHEAD      (SIZEOF_TWO_SIZE_T)

/* The smallest size we can malloc is an aligned minimal chunk */              
#define MIN_CHUNK_SIZE      (CHUNK_SIZE)

/* Conversion from malloc headers to user pointers, and back */                
#define CHUNK_TO_MEM(p)     ((void*)((char*)(p) + SIZEOF_TWO_SIZE_T))     
#define MEM_TO_CHUNK(mem)   ((chunkptr)((char*)(mem) - SIZEOF_TWO_SIZE_T))     

/* Treat space at ptr +/- offset as a chunk */                                 
#define CHUNK_PLUS_OFFSET(p, s)  ((chunkptr)(((char*)(p)) + (s)))             
#define CHUNK_MINUS_OFFSET(p, s) ((chunkptr)(((char*)(p)) - (s)))             

/* Pad request bytes into a usable size */                                     
#define PAD_REQUEST(req)        \
    (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)           


#define P_USED_BIT          (SIZE_T_ONE)                                       
#define C_USED_BIT          (SIZE_T_TWO)                                       
#define FLAG_BITS           (P_USED_BIT | C_USED_BIT)                            

/* Extraction of fields from head words */                                     
#define C_USED(p)           ((p)->head & C_USED_BIT)                           
#define P_USED(p)           ((p)->head & P_USED_BIT)   

#define CHUNKSIZE(p)        ((p)->head & ~(CHUNK_ALIGN_MASK))                          

#define SET_CHUNK_SIZE(p, s)                            \
    ((p)->head = ((s) | ((p)->head & CHUNK_ALIGN_MASK)))   

#define CLEAR_P_USED(p)     ((p)->head &= ~P_USED_BIT)                         
#define CLEAR_C_USED(p)     ((p)->head &= ~C_USED_BIT)                         

#define SET_P_USED(p)     ((p)->head |= P_USED_BIT)
#define SET_C_USED(p)     ((p)->head |= C_USED_BIT)

/* Pointer to next or previous physical malloc_chunk. */                           
#define NEXT_CHUNK(p) ((chunkptr)( ((char*)(p)) + ((p)->head & ~CHUNK_ALIGN_MASK))) 
#define PREV_CHUNK(p) ((chunkptr)( ((char*)(p)) - ((p)->prev_foot) ))         

/* Extract next chunk's p_used bit */                                          
#define next_p_used(p)  ((next_chunk(p)->head) & P_USED_BIT)                   

/* Get/set size at footer */                                                   
#define GET_FOOT(p, s)  (NEXT_CHUNK(p)->prev_foot)           
#define SET_FOOT(p, s)  (NEXT_CHUNK(p)->prev_foot = (s))     

/* Set size, p_used bit, and foot */                                           
#define SET_C_USED_AND_NEXT_P_USED(p) \
{                                       \
    SET_C_USED(p);                      \
    SET_P_USED(NEXT_CHUNK(p));          \
}

#define CLEAR_C_USED_AND_NEXT_P_USED(p) \
{                                       \
    CLEAR_C_USED(p);                    \
    CLEAR_P_USED(NEXT_CHUNK(p));        \
}
#define CHECK_INUSE_CHUNK(p)                    \
{                                               \
    assert(C_USED(p));                          \
    assert(P_USED(NEXT_CHUNK(p)));              \
}

#define IS_PREV_FREE(p)                 (!P_USED(p))
#define IS_NEXT_FREE(p)                 (!C_USED(NEXT_CHUNK(p)))

#endif
