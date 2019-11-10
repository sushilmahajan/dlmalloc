#ifndef BINS_H_INCLUDED 
#define BINS_H_INCLUDED 1

#include "chunks.h"
#include <assert.h>
#include <math.h>
#include "debug.h"

#define DEBUG_BINS     (0)

/* Bin types, widths and sizes */                                              
#define NSMALLBINS        (64U)                                                
#define NTREEBINS         (32U)
#define SMALLBIN_SHIFT    (3U) 
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)                       
#define TREEBIN_SHIFT     (9U)                                                 
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)         // 512 
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZEOF_TWO_SIZE_T)  // 496
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_OVERHEAD)     // 496-16=480
#define MIN_LARGE_REQUEST (MAX_SMALL_REQUEST + SIZE_T_ONE)      // 481
#define MIN_REQUEST       (CHUNK_SIZE - CHUNK_OVERHEAD)

typedef size_t binmap_t;

typedef struct malloc_state {
    bool        initialized;
    char*       start_addr;
    char*       end_addr;
    chunkptr    bchunk;
    size_t      bsize; 
    chunkptr    top;
    size_t      topsize; 
    chunkptr    rchunk;     
    size_t      rsize;   
    binmap_t    smallmap;
    binmap_t    treemap; 
    chunk       smallbins[NSMALLBINS];
    tchunkptr   treebins[NTREEBINS];  
} mstate;                                                                             

/* Check if address a is at least as high as any from MORECORE or MMAP */      
#define OK_ADDRESS(M, a) (((char*)(a) >= (M)->start_addr) && ((char*)(a) <= (M)->end_addr))                       

#define small_index(s)          (index_t)((s)  >> SMALLBIN_SHIFT)
#define IDX_TO_SIZE(i)          ((i)  << SMALLBIN_SHIFT)

#define IS_SMALLBIN_NON_EMPTY(M, i)     \
    (((M->smallmap) >> i) & 0x1U)
#define IS_TREEBIN_NON_EMPTY(M, i)      \
    (((M->treemap) >> i) & 0x1U)

/* Addressing by index */
#define smallbin_at(M, i)   ((chunkptr)((char*)&((M)->smallbins[i]))) 
#define treebin_at(M,i)     ((tchunkptr*)((char*)&((M)->treebins[i])))

/* Assign tree index for size S to variable I */
#define compute_tree_index(S, I)                                        \
{                                                                       \
    size_t X = (S) >> TREEBIN_SHIFT;                                    \
    if (X == 0)                                                         \
    I = 0;                                                              \
    else if (X > 0xFFFF)                                                \
    I = NTREEBINS-1;                                                    \
    else {                                                              \
        size_t Y = (size_t)X;                                           \
        size_t N = ((Y - 0x100) >> 16) & 8;                             \
        size_t K = (((Y <<= N) - 0x1000) >> 16) & 4;                    \
        N += K;                                                         \
        N += K = (((Y <<= K) - 0x4000) >> 16) & 2;                      \
        K = 14 - N + ((Y <<= K) >> 15);                                 \
        I = (K << 1) + (((S) >> (K + (TREEBIN_SHIFT-1)) & 1));          \
    }                                                                   \
}                                                                              

#define MINSIZE_OF_LAST_TREE        (0x1800000)

/* bit corresponding to given index */
#define IDX_TO_BIT(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->smallmap |= IDX_TO_BIT(i))
#define clear_smallmap(M,i)     ((M)->smallmap &= ~IDX_TO_BIT(i))
#define smallmap_is_marked(M,i) ((M)->smallmap & IDX_TO_BIT(i))

#define mark_treemap(M,i)       ((M)->treemap |= IDX_TO_BIT(i))
#define clear_treemap(M,i)      ((M)->treemap &= ~IDX_TO_BIT(i))
#define treemap_is_marked(M,i)  ((M)->treemap & IDX_TO_BIT(i))

/* Isolate the least set bit of a bitmap */
#define LEAST_BIT(x)         ((size_t)(log2((x) & -(x))) + 1)

/* Mask with all bits to left of least bit of x on */
#define LEFT_BITS(x)         ((x<<1) | -(x<<1))

// TODO  CORRUPTION_ERROR_ACTION 

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
chunkptr remove_small_chunk(mstate *state, size_t size);                            

chunkptr remove_small_chunk_by_address(mstate *state, chunkptr ptr);

/* Insert chunk into tree */

void insert_large_chunk(mstate *state, tchunkptr ptr, size_t size);

uint8_t search_large_chunk(mstate *state, tchunkptr *ptr,  size_t size);

tchunkptr min_sized_chunk(tchunkptr ptr);

tchunkptr remove_large_chunk_by_address(mstate *state, tchunkptr ptr);

tchunkptr remove_large_chunk(mstate *state, size_t size);

#endif
