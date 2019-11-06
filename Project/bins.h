#ifndef BINS_H_INCLUDED 
#define BINS_H_INCLUDED 1

#include "chunks.h"
#include <assert.h>
#include "debug.h"

#define PREV 0
#define DEBUG_BINS     (1)

/* Bin types, widths and sizes */                                              
#define NSMALLBINS        (64U)                                                
#define NTREEBINS         (32U)
#define SMALLBIN_SHIFT    (3U) 
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)                       
#if PREV
#define TREEBIN_SHIFT     (8U)                                                 
#else 
#define TREEBIN_SHIFT     (9U)                                                 
#endif 
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)                        
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZE_T_ONE)                        
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD)  // TODO remove CHUNK_ALIGN_MASK 

typedef uint32_t treemap_t;

typedef size_t binmap_t;

typedef struct malloc_state {                                                          
    binmap_t   smallmap;                                                         
    binmap_t   treemap;                                                          
    chunkptr   dv;                                                               
    size_t     dvsize;                                                           
    chunkptr   top;                                                              
    size_t     topsize;                                                          
    char*      least_addr;                                                       
    chunkptr   smallbins[NSMALLBINS];                                      
    tchunkptr  treebins[NTREEBINS];                                              
} mstate;                                                                             

/* Check if address a is at least as high as any from MORECORE or MMAP */      
#define ok_address(M, a) ((char*)(a) >= (M)->least_addr)                       

#define is_small(s)             (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)          (index_t)((s)  >> SMALLBIN_SHIFT)
#define small_index_to_size(i)  ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX         (small_index(MIN_CHUNK_SIZE))

/* Addressing by index */
#define smallbin_at(M, i)   ((sbinptr)((char*)&((M)->smallbins[i]))) //Removed left shift
#define treebin_at(M,i)     (&((M)->treebins[i]))

/* Assign tree index for size S to variable I TODO change else if & else */
#define compute_tree_index(S, I)                                        \
{                                                                       \
    size_t X = S >> TREEBIN_SHIFT;                                    \
    if (X == 0)                                                         \
    I = 0;                                                             \
    else if (X > 0xFFFF)                                                \
    I = NTREEBINS-1;                                                   \
    else {                                                              \
        size_t Y = (size_t)X;                                       \
        size_t N = ((Y - 0x100) >> 16) & 8;                           \
        size_t K = (((Y <<= N) - 0x1000) >> 16) & 4;                  \
        N += K;                                                         \
        N += K = (((Y <<= K) - 0x4000) >> 16) & 2;                      \
        K = 14 - N + ((Y <<= K) >> 15);                                 \
        I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));           \
    }                                                                   \
}                                                                              

/* Bit representing maximum resolved size in a treebin at i TODO */
#define bit_for_tree_index(i) \
    (i == NTREEBINS-1) ? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit TODO */
#define leftshift_for_tree_index(i) \
    ((i == NTREEBINS-1)? 0 : \
     ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i) \
    ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
     (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))    // 256, 384, 512, 768, 1024, 

#define MINSIZE_OF_LAST_TREE        (0x1800000)

/* bit corresponding to given index */
#define idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->smallmap |= idx2bit(i))
#define clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#define smallmap_is_marked(M,i) ((M)->smallmap & idx2bit(i))

#define mark_treemap(M,i)       ((M)->treemap |= idx2bit(i))
#define clear_treemap(M,i)      ((M)->treemap &= ~idx2bit(i))
#define treemap_is_marked(M,i)  ((M)->treemap & idx2bit(i))

/* Isolate the least set bit of a bitmap */
#define least_bit(x)         ((x) & -(x))

/* Mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/* Mask with all bits to left of or equal to least bit of x on TODO remove */
#define same_or_left_bits(x) ((x) | -(x))

// TODO  corruption_error_Action 

/* \brief Add a free small chunk into a smallbin 
 * \param mstate Global malloc state 
 * \param chunkptr Pointer to small chunk to be added 
 * \param size Size of the chunk
 * */                                        
void insert_small_chunk(mstate *state, chunkptr ptr, size_t size); 

/* \brief Remove a small chunk from a smallbin 
 * \param mstate Global malloc state 
 * \param chunkptr Pointer to small chunk to be removed, should point to last 
 *                 chunk in the bin 
 * \param size Size of the chunk
 * */                                        
void remove_small_chunk(mstate *state, chunkptr ptr, size_t size);                                      
    
/* Insert chunk into tree */
void insert_large_chunk(mstate *M, tchunk *X, size_t S); 

/*
   Unlink steps:

   1. If x is a chained node, unlink it from its same-sized fd/bk links
   and choose its bk node as its replacement.
   2. If x was the last node of its size, but not a leaf node, it must
   be replaced with a leaf node (not merely one with an open left or
   right), to make sure that lefts and rights of descendents
   correspond properly to bit masks.  We use the rightmost descendent
   of x.  We could use any other leaf, but this is easy to locate and
   tends to counteract removal of leftmosts elsewhere, and so keeps
   paths shorter than minimally guaranteed.  This doesn't loop much
   because on average a node in a tree is near the bottom.
   3. If x is the base of a chain (i.e., has parent links) relink
   x's parent and children to x's replacement (or null if none).
   */

void unlink_large_chunk(mstate *M, tchunk *X);

#endif
