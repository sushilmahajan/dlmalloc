#ifndef BINS_H_INCLUDED 
#define BINS_H_INCLUDED 1

#include "chunks.h"

/* Bin types, widths and sizes */                                              
#define NSMALLBINS        (32U)                                                
#define NTREEBINS         (32U)                                                
#define SMALLBIN_SHIFT    (3U)                                                 
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)                       
#define TREEBIN_SHIFT     (8U)                                                 
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)                        
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZE_T_ONE)                        
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD) 

typedef uint32_t binmap_t;

#define is_small(s)             (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)          (index_t)((s)  >> SMALLBIN_SHIFT)
#define small_index_to_size(i)  ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX         (small_index(MIN_CHUNK_SIZE))

/* Addressing by index */
#define smallbin_at(M, i)   ((sbinptr)((char*)&((M)->smallbins[(i)<<1])))
#define treebin_at(M,i)     (&((M)->treebins[i]))

/* Assign tree index for size S to variable I */
#define compute_tree_index(S, I)                                        \
{                                                                       \
    size_t X = S >> TREEBIN_SHIFT;                                      \
    if (X == 0)                                                         \
    I = 0;                                                              \
    else if (X > 0xFFFF)                                                \
    I = NTREEBINS-1;                                                    \
    else {                                                              \
        uint32_t Y = (uint32_t)X;                                       \
        uint32_t N = ((Y - 0x100) >> 16) & 8;                           \
        uint32_t K = (((Y <<= N) - 0x1000) >> 16) & 4;                  \
        N += K;                                                         \
        N += K = (((Y <<= K) - 0x4000) >> 16) & 2;                      \
        K = 14 - N + ((Y <<= K) >> 15);                                 \
        I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));            \
    }                                                                   \
}

/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_tree_index(i) \
    (i == NTREEBINS-1) ? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_tree_index(i) \
    ((i == NTREEBINS-1)? 0 : \
     ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i) \
    ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
     (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))


/* bit corresponding to given index */
#define idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#define clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#define smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#define mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#define clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#define treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* Isolate the least set bit of a bitmap */
#define least_bit(x)         ((x) & -(x))

/* Mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/* Mask with all bits to left of or equal to least bit of x on */
#define same_or_left_bits(x) ((x) | -(x))

#endif
