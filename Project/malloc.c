#include "malloc.h"

#define TOP_SIZE                (32)
#define BEGIN_SIZE              (32)
#define SYSTEM_ALLOCATION_SIZE  (100 * 1024)
#define TRACK_MALLOC            (1)
#define TRACK_FREE              (1)

mstate _g_mstate_;

chunkptr split_small_chunk(mstate *state, chunkptr ochunk_ptr, size_t size) {   //TODO test
    chunkptr new_chunk_ptr;
    size_t ochunk_new_size; 

    new_chunk_ptr = ochunk_ptr;
    ochunk_new_size = CHUNKSIZE(new_chunk_ptr) - size;
    ochunk_ptr = CHUNK_PLUS_OFFSET(new_chunk_ptr, size);

    *ochunk_ptr = *new_chunk_ptr;
    SET_CHUNK_SIZE(ochunk_ptr, ochunk_new_size);
    SET_P_USED(ochunk_ptr);
    SET_FOOT(ochunk_ptr, ochunk_new_size);

    insert_small_chunk(state, ochunk_ptr, ochunk_new_size);

    SET_CHUNK_SIZE(new_chunk_ptr, size);
    SET_C_USED(new_chunk_ptr);
    SET_FOOT(new_chunk_ptr, size);

    return new_chunk_ptr;
}

chunkptr split_tree_chunk(mstate *state, tchunkptr o_tchunk_ptr, size_t size) {   //TODO test
    tchunkptr new_chunk_ptr;
    size_t o_tchunk_new_size; 

    new_chunk_ptr = o_tchunk_ptr;
    o_tchunk_new_size = CHUNKSIZE(new_chunk_ptr) - size;
    o_tchunk_ptr = (tchunkptr)CHUNK_PLUS_OFFSET(new_chunk_ptr, size);
    *o_tchunk_ptr = *new_chunk_ptr;
    
    SET_CHUNK_SIZE(o_tchunk_ptr, o_tchunk_new_size);
    SET_P_USED(o_tchunk_ptr);
    SET_FOOT(o_tchunk_ptr, o_tchunk_new_size);

    if (o_tchunk_new_size <= MAX_SMALL_SIZE) {
        insert_small_chunk(state, (chunkptr)(o_tchunk_ptr), o_tchunk_new_size);
    }
    else {
        insert_large_chunk(state, o_tchunk_ptr, o_tchunk_new_size);
    }

    SET_CHUNK_SIZE(new_chunk_ptr, size);
    SET_C_USED(new_chunk_ptr);
    SET_FOOT(new_chunk_ptr, size);

    return (chunkptr)new_chunk_ptr;
}

chunkptr split_rchunk(mstate *state, size_t size) {   //TODO test
    chunkptr new_chunk_ptr, o_rchunk_ptr;
    size_t o_rchunk_new_size; 
    
    new_chunk_ptr = o_rchunk_ptr = state->rchunk;     
    o_rchunk_new_size = state->rsize - size;
    o_rchunk_ptr = CHUNK_PLUS_OFFSET(new_chunk_ptr, size);
    *o_rchunk_ptr = *new_chunk_ptr;

    SET_CHUNK_SIZE(o_rchunk_ptr, o_rchunk_new_size);
    SET_P_USED(o_rchunk_ptr);
    CLEAR_C_USED(o_rchunk_ptr);
    SET_FOOT(o_rchunk_ptr, o_rchunk_new_size);

    state->rchunk = o_rchunk_ptr;
    state->rsize = o_rchunk_new_size;

    SET_CHUNK_SIZE(new_chunk_ptr, size);
    SET_C_USED(new_chunk_ptr);
    SET_P_USED(new_chunk_ptr);
    SET_FOOT(new_chunk_ptr, size);

    return new_chunk_ptr;
}

void my_malloc_init(mstate *state) {
    state->initialized = 1;
    state->start_addr = (char*)get_start_address();
    state->end_addr = (char*)get_end_address();

    /* Set begin chunk */
    state->bchunk = (chunkptr)state->start_addr;
    state->bsize = BEGIN_SIZE;
    /* Set begin chunk parameters in physical memory */
    SET_CHUNK_SIZE(state->bchunk, state->bsize);
    SET_P_USED(state->bchunk);
    SET_C_USED(state->bchunk);
    SET_FOOT(state->bchunk, state->bsize);

    /* Set top chunk */
    state->top = CHUNK_MINUS_OFFSET(state->end_addr, TOP_SIZE);
    state->topsize = TOP_SIZE;
    /* Set top chunk parameters in physical memory */
    SET_CHUNK_SIZE(state->top, state->topsize);
    SET_C_USED(state->top);
    CLEAR_P_USED(state->top);

    /* Set rchunk */
    state->rchunk = CHUNK_PLUS_OFFSET(state->start_addr, 32);
    state->rsize = ((size_t)(state->end_addr - state->start_addr) - state->topsize - state->bsize);
    /* Set rchunk parameters  in physical memory */
    SET_CHUNK_SIZE(state->rchunk, state->rsize);
    SET_P_USED(state->rchunk);
    CLEAR_C_USED(state->rchunk);
    SET_FOOT(state->rchunk, state->rsize);
    //MAKE_CHUNK_USABLE(state->rchunk);

    state->smallmap = 0;
    state->treemap = 0;
}

uint8_t sys_alloc(mstate *state) {
    chunkptr topchunk, new_topchunk;
    void *end_ptr;

    topchunk = state->top;

    end_ptr = get_mem_from_sys(SYSTEM_ALLOCATION_SIZE);
    if (end_ptr == NULL) {
        return 1;
    }

    new_topchunk = CHUNK_PLUS_OFFSET(topchunk, SYSTEM_ALLOCATION_SIZE);
    *new_topchunk = *topchunk;

    state->rsize += SYSTEM_ALLOCATION_SIZE;
    SET_CHUNK_SIZE(state->rchunk, state->rsize);
    return 0;
}

void check_malloced_chunk(void *mem, size_t s) {           
    if (mem != 0) {                        
        chunkptr p = MEM_TO_CHUNK(mem);
        size_t sz = p->head & ~FLAG_BITS;
        CHECK_INUSE_CHUNK(p);
        assert((sz & CHUNK_ALIGN_MASK) == 0);
        assert(sz >= MIN_CHUNK_SIZE);
        assert(sz >= s);
    }
} 


chunkptr allocate_from_larger_smallbins(mstate *state, size_t size) {
    index_t i = small_index(size);
    binmap_t smallbits = state->smallmap;
    smallbits >>= (i+1);
    
    if (!smallbits) {
        return NULL;
    }
    else {
        chunkptr s_ptr, new_ptr;
        
        i += LEAST_BIT(smallbits);
        size_t big_size = IDX_TO_SIZE(i);
       
        s_ptr = remove_small_chunk(state, big_size);
        if ((big_size - size) < MIN_CHUNK_SIZE) {
            return s_ptr;
        }
        
        new_ptr = split_small_chunk(state, s_ptr, size);
        return new_ptr;
    }
}

chunkptr allocate_from_larger_treebins(mstate *state, size_t size) {
    index_t i;
    compute_tree_index(size, i);
    if (i)
        i++;
    binmap_t treebits = state->treemap;
    treebits >>= i;
    
    if (!treebits) {
        return NULL;
    }
    else {
        tchunkptr *base, o_tptr;
        chunkptr new_ptr;
        
        i += (LEAST_BIT(treebits)-1);
        base = treebin_at(state, i);
        
        o_tptr = min_sized_chunk(*base);
        if (o_tptr->left != NULL) {
            o_tptr = o_tptr->left;
        }
        
        o_tptr = remove_large_chunk_by_address(g_mstate, o_tptr);
        
        new_ptr = split_tree_chunk(state, o_tptr, size);

        return new_ptr;
    }
}

void *my_malloc(size_t bytes) 
{    
#if TRACK_MALLOC
            printf("\n**************Call to malloc begins***************\n\n");
#endif
    if (!(g_mstate->initialized)) {
        my_malloc_init(g_mstate);
    }

    // assert(!(size >> (SIZE_T_BITSIZE-1))); TODO introduce compiler warning

    bool allocated = 0;
    index_t i;
    chunkptr ptr;
    tchunkptr t_ptr, *tptr = &t_ptr;
    size_t size;
    void *mem;
    uint8_t x;

    size = (bytes <= MIN_REQUEST) ? MIN_CHUNK_SIZE : PAD_REQUEST(bytes); 

    if (size <= MAX_SMALL_REQUEST) {    // if small size, check in smallbin of same size 
        if (IS_SMALLBIN_NON_EMPTY(g_mstate, small_index(size))) {
#if TRACK_MALLOC
            printf("Small chunk of required size exists in smallbin of index %ld\n", small_index(size));
#endif
            ptr = remove_small_chunk(g_mstate, size);                               
            FINISH_ALLOCATION(ptr, size);
        }                                                                        
    }
    else {   // if large size, check in treebin of same size range
        compute_tree_index(size, i);

        if (IS_TREEBIN_NON_EMPTY(g_mstate, i)) { 
#if TRACK_MALLOC
            printf("Large chunk or required size exists in treebin of index %ld\n", i);
#endif
            x = search_large_chunk(g_mstate, tptr, size);   // store best found choice
            if (x) {                
                ptr = (chunkptr)remove_large_chunk_by_address(g_mstate, *tptr);
                FINISH_ALLOCATION(ptr, size);
                *tptr = NULL;
            }
        }
        else {
            *tptr = NULL;
        }
    }
    while (!allocated) {
        if (size <= g_mstate->rsize) {  // if rchunk is large enough split it
#if TRACK_MALLOC
            printf("Splitting rchunk to allocate new block\n");
#endif
            ptr = split_rchunk(g_mstate, size);
            FINISH_ALLOCATION(ptr, size);
            break;
        }
        else {
            if (size <= MAX_SMALL_REQUEST) { // if small request doesn't fit in rchunk, check for larger smallbins & treebins
                if ((ptr = allocate_from_larger_smallbins(g_mstate, size))) {
#if TRACK_MALLOC
                    printf("Allocated space from larger smallbin chunk\n");
#endif
                    FINISH_ALLOCATION(ptr, size);
                    break;
                }
                else if ((ptr = allocate_from_larger_treebins(g_mstate, size))) {
#if TRACK_MALLOC
                    printf("Allocated space from treebin chunk\n");
#endif
                    FINISH_ALLOCATION(ptr, size);
                    break;
                }
            }
            else {
                if (*tptr != NULL) {
#if TRACK_MALLOC
                    printf("Allocated best found treebin chunk\n");
#endif
                    FINISH_ALLOCATION(*tptr, size);
                    break;
                }
                else if ((ptr = allocate_from_larger_treebins(g_mstate, size))) {
#if TRACK_MALLOC
                    printf("Allocated space from larger treebin chunk\n");
#endif
                    FINISH_ALLOCATION(ptr, size);
                    break;
                }
            }
            if (!allocated) {
                if (sys_alloc(g_mstate)) {
#if TRACK_MALLOC
                    printf("System allocation performed\n");
#endif
                    allocated = 0;
                    break;
                }
            }
        }
    }
#if TRACK_MALLOC
            printf("\n**************Call to malloc ends***************\n\n");
#endif
    if (allocated == 0) {
        return NULL;
    }
    else {
        return mem;
    }
}

void *my_calloc(size_t num, size_t size) {
    void *ptr;

    ptr = my_malloc(num * size);
    if (ptr == NULL) {
        return ptr;
    }

    bzero(ptr, num * size);
    return ptr;
}

void *my_realloc(void *ptr, size_t new_size) {

    assert(OK_ADDRESS(g_mstate, ptr));  

    chunkptr fchunk = MEM_TO_CHUNK(ptr);
    check_malloced_chunk(ptr, CHUNKSIZE(fchunk));

    size_t prev_size;
    prev_size = CHUNKSIZE(fchunk);
    new_size = PAD_REQUEST(new_size);

    if (prev_size >= new_size) {
        if ((prev_size - new_size) >= MIN_CHUNK_SIZE) {
            chunkptr new_fchunk = CHUNK_PLUS_OFFSET(fchunk, new_size);

            SET_CHUNK_SIZE(fchunk, new_size);
            SET_C_USED_AND_NEXT_P_USED(fchunk);
            SET_FOOT(fchunk, prev_size - new_size);

            SET_CHUNK_SIZE(new_fchunk, prev_size - new_size);
            SET_C_USED_AND_NEXT_P_USED(new_fchunk);
            SET_FOOT(new_fchunk, prev_size - new_size);

            my_free(CHUNK_TO_MEM(new_fchunk));

        }
        return ptr;
    }
    else {
        void *new_ptr = my_malloc(new_size);
        memcpy(new_ptr, ptr, prev_size);
        my_free(ptr);
        return new_ptr;
    }
}
        

void my_free(void *ptr) {
#if TRACK_FREE
            printf("\n**************Call to free begins***************\n\n");
#endif
    
    assert(OK_ADDRESS(g_mstate, ptr));  

    chunkptr fchunk = MEM_TO_CHUNK(ptr);
    check_malloced_chunk(ptr, CHUNKSIZE(fchunk));

    if (IS_PREV_FREE(fchunk)) {
#if TRACK_FREE
        printf("Coalescing current free chunk with previous free chunk\n");
#endif 
        chunkptr prev_chunk, new_fchunk; 
        size_t old_curr_size, old_prev_size;

        prev_chunk = PREV_CHUNK(fchunk);
        old_curr_size = CHUNKSIZE(fchunk);
        old_prev_size = CHUNKSIZE(prev_chunk);
        
        if (old_prev_size <= MAX_SMALL_SIZE) {
            prev_chunk = remove_small_chunk_by_address(g_mstate, prev_chunk);
        }
        else {
            prev_chunk = (chunkptr)remove_large_chunk_by_address(g_mstate, (tchunkptr)prev_chunk);
        }
//        if (PREV_CHUNK == NULL) {
//            CORRUPTION_ERROR_ACTION();
//        }

        new_fchunk = prev_chunk;
        SET_CHUNK_SIZE(new_fchunk, old_prev_size + old_curr_size);
        SET_P_USED(new_fchunk);
        CLEAR_C_USED(new_fchunk);
        SET_FOOT(new_fchunk, old_prev_size + old_curr_size);

        fchunk = new_fchunk;
    }
    if (IS_NEXT_FREE(fchunk)) {
        if (NEXT_CHUNK(fchunk) == g_mstate->rchunk) {
#if TRACK_FREE
        printf("Next chunk is free and is rchunk, coalescing with rchunk & changing rchunk parameters\n");
#endif 
            chunkptr new_rchunk; 
            size_t old_free_size;
            
            assert(fchunk == PREV_CHUNK(g_mstate->rchunk));
            
            old_free_size = CHUNKSIZE(fchunk);
            new_rchunk = fchunk;

            SET_CHUNK_SIZE(new_rchunk, old_free_size + g_mstate->rsize);
            SET_P_USED(new_rchunk);
            CLEAR_C_USED(new_rchunk);
            SET_FOOT(new_rchunk, old_free_size + g_mstate->rsize);

            g_mstate->rchunk = new_rchunk;
            g_mstate->rsize = CHUNKSIZE(new_rchunk);
            return;
        }
        else {
#if TRACK_FREE
        printf("Next chunk is free, coalescing with it\n");
#endif 
            chunkptr next_chunk, new_fchunk; 
            size_t old_curr_size, old_next_size;

            new_fchunk = fchunk;
            next_chunk = NEXT_CHUNK(fchunk);
            old_curr_size = CHUNKSIZE(fchunk);
            old_next_size = CHUNKSIZE(next_chunk);

            if (old_next_size <= MAX_SMALL_SIZE) {
                next_chunk = remove_small_chunk_by_address(g_mstate, next_chunk);
            }
            else {
                next_chunk = (chunkptr)remove_large_chunk_by_address(g_mstate, (tchunkptr)next_chunk);
            }

            SET_CHUNK_SIZE(new_fchunk, old_next_size + old_curr_size);
            SET_P_USED(new_fchunk);
            CLEAR_C_USED(new_fchunk);
            SET_FOOT(new_fchunk, old_next_size + old_curr_size);

            fchunk = new_fchunk;
        }
    }
    
    CLEAR_C_USED_AND_NEXT_P_USED(fchunk);
    size_t size = CHUNKSIZE(fchunk);
#if TRACK_FREE
        printf("Adding the free chunk of size %ld to corresponding bin\n", size);
#endif 
    if (size <= MAX_SMALL_SIZE) {
        insert_small_chunk(g_mstate, fchunk, size);
    }
    else {
        insert_large_chunk(g_mstate, (tchunkptr)fchunk, size);
    }
#if TRACK_FREE
            printf("\n**************Call to free ends***************\n\n");
#endif
}
