#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED 1

#include <stdint.h>
#include <stdlib.h>

typedef uint32_t index_t;

typedef struct chunk {
    size_t prev_foot;
    size_t head;
    struct chunk *prev;
    struct chunk *next;
} chunk;

typedef struct tree_chunk {            
    /* First 4 parameters should be same as chunk */    
    size_t prev_foot;                                         
    size_t head;                                              
    struct tree_chunk *prev;                                                
    struct tree_chunk *next;                                                                                                          
    struct tree_chunk *child[2];                                          
    struct tree_chunk *parent;                                            
    index_t index;                                             
} tchunk;                                                                            

typedef chunk* chunkptr;
typedef chunk* sbinptr;

typedef tchunk* tchunkptr;

#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZEOF_SIZE_T       (sizeof(size_t))
#define SIZEOF_TWO_SIZE_T   (SIZEOF_SIZE_T * 2)

#define MALLOC_ALIGNMENT    (SIZEOF_TWO_SIZE_T)
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - 1)
#define CHUNK_SIZE          (sizeof(chunk))
#define CHUNK_OVERHEAD      (SIZEOF_TWO_SIZE_T)

/* The number of bytes to offset an address to align it */                     
#define ALIGN_OFFSET(A)                             \
    ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :   \
     ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))  

/* The smallest size we can malloc is an aligned minimal chunk */              
#define MIN_CHUNK_SIZE      (CHUNK_SIZE)

/* Conversion from malloc headers to user pointers, and back */                
#define CHUNK_TO_MEM(p)     ((void*)((char*)(p) + SIZEOF_TWO_SIZE_T))     
#define MEM_to_CHUNK(mem)   ((chunkptr)((char*)(mem) - SIZEOF_TWO_SIZE_T))     

/* Chunk associated with aligned address A */                                  
#define ALIGN_AS_CHUNK(A)   (chunkptr)((A) + ALIGN_OFFSET(CHUNK_TO_MEM(A)))      
                                                                               
/* Bounds on request (not chunk) sizes. */                                     
#define MAX_REQUEST         ((-MIN_CHUNK_SIZE) << 2)                           
#define MIN_REQUEST         (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)     
                                                                               
/* Pad request bytes into a usable size */                                     
#define PAD_REQUEST(req)        \
    (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)           
                                                                               
/* Pad request, checking for minimum (but not maximum) */
#define REQUEST_TO_SIZE(req)    \
    (((req) < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(req))


#define P_USED_BIT          (SIZE_T_ONE)                                       
#define C_USED_BIT          (SIZE_T_TWO)                                       
#define FLAG_BITS           (P_USED_BIT | C_USED_BIT)                            
                                                                               
/* Extraction of fields from head words */                                     
#define c_used(p)           ((p)->head & C_USED_BIT)                           
#define p_used(p)           ((p)->head & P_USED_BIT)                           
#define is_inuse(p)         (((p)->head & FLAG_BITS) != P_USED_BIT)           
                                                                               
#define chunksize(p)        ((p)->head & ~(FLAG_BITS))                         
                                                                               
#define clear_p_used(p)     ((p)->head &= ~P_USED_BIT)                         
                                                                               
/* Pointer to next or previous physical malloc_chunk. */                           
#define next_chunk(p) ((chunkptr)( ((char*)(p)) + ((p)->head & ~FLAG_BITS)))  
#define prev_chunk(p) ((chunkptr)( ((char*)(p)) - ((p)->prev_foot) ))         
                                                                               
/* Extract next chunk's p_used bit */                                          
#define next_p_used(p)  ((next_chunk(p)->head) & P_USED_BIT)                   

/* Get/set size at footer */                                                   
#define get_foot(p, s)  (((chunkptr)((char*)(p) + (s)))->prev_foot)           
#define set_foot(p, s)  (((chunkptr)((char*)(p) + (s)))->prev_foot = (s))     
                                                                               
/* Set size, p_used bit, and foot */                                           
#define set_size_and_p_used_of_free_chunk(p, s) \
    ((p)->head = (s|P_USED_BIT), set_foot(p, s))                                 
                                                                               
/* Set size, p_used bit, foot, and clear next p_used */                        
#define set_free_with_p_used(p, s, n)   \
    (clear_p_used(n), set_size_and_p_used_of_free_chunk(p, s))                   
                                                                               
#endif
