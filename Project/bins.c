#include "bins.h"

void insert_small_chunk(mstate *state, chunkptr ptr, size_t size) { 
    index_t i  = small_index(size);
    chunkptr back = smallbin_at(state, i);
    chunkptr front = back;

    assert(size >= MIN_CHUNK_SIZE);                                                

    if (!smallmap_is_marked(state, i)) {        // Insert 1st chunk
        mark_smallmap(state, i);
    }
    else if (ok_address(state, back->next)) {   // Insert at next position 
        front = back->next;
    }
    //  else {
    //    CORRUPTION_ERROR_ACTION(state);       TODO
    //  }                                                
    back->next = ptr;
    front->prev = ptr;
    ptr->next = front;
    ptr->prev = back;
#if DEBUG_BINS
    printf("Small chunk of size %ld is inserted in smallbin with index %ld\n", size, i);
#endif
}                                                                              

void remove_small_chunk(mstate *state, chunkptr ptr, size_t size) {                                      
    chunkptr front = ptr->next;      
    chunkptr back = ptr->prev;      
    index_t i = small_index(size);
#if DEBUG_BINS
    bool empty = 0;
#endif

    assert(ptr != back);            
    assert(ptr != front);            
    assert(chunksize(ptr) == small_index_to_size(i));

    if ((ok_address(state, front)) && (ok_address(state, back))) {
        if ((front == smallbin_at(state, i)) && (front == back)) {     
            clear_smallmap(state, i);   // Remove only chunk, hence clear map
#if DEBUG_BINS
            empty = 1;
#endif
        }
    }
    //  else {
    //    CORRUPTION_ERROR_ACTION(state);       TODO
    //  } 

    front->prev = back;
    back->next = front;
#if DEBUG_BINS
    printf("Small chunk of size %ld is removed from smallbin with index %ld\n", size, i);
    if (empty) {
        printf("Smallbin with index %ld is empty now\n", i);
    }
#endif
}

/* Insert chunk into tree */
void insert_large_chunk(mstate *M, tchunk *X, size_t S) { 
    tbinptr* H; 
    index_t I; 
    compute_tree_index(S, I); 
    H = treebin_at(M, I); 
    X->index = I; 
    X->child[0] = X->child[1] = 0; 
    if (!treemap_is_marked(M, I)) { 
        mark_treemap(M, I); 
        *H = X; 
        X->parent = (tchunkptr)H; 
        X->next = X->prev = X; 
    } 
    else { 
        tchunkptr T = *H; 
        size_t K = S << leftshift_for_tree_index(I); 
        while (1) { 
            if (chunksize(T) != S) { 
                tchunkptr* C = &(T->child[(K >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1]); 
                K <<= 1; 
                if (*C != 0) 
                    T = *C; 
                else if (ok_address(M, C)) { 
                    *C = X; 
                    X->parent = T; 
                    X->next = X->prev = X; 
                    break; 
                } 
                else { 
                    //           CORRUPTION_ERROR_ACTION(M); 
                    break; 
                } 
            } 
            else { 
                tchunkptr F = T->next; 
                if (ok_address(M, T) && ok_address(M, F)) { 
                    T->next = F->prev = X; 
                    X->next = F; 
                    X->prev = T; 
                    X->parent = 0; 
                    break; 
                } 
                else { 
                    //           CORRUPTION_ERROR_ACTION(M); 
                    break; 
                } 
            } 
        } 
    } 
}

/*
   Unlink steps:

   1. If x is a chained node, unlink it from its same-sized next/prev links
   and choose its prev node as its replacement.
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

void unlink_large_chunk(mstate *M, tchunk *X) { 
    tchunkptr XP = X->parent; 
    tchunkptr R; 
    if (X->prev != X) { 
        tchunkptr F = X->next; 
        R = X->prev; 
        if (ok_address(M, F) && F->prev == X && R->next == X) { 
            F->prev = R; 
            R->next = F; 
        } 
        else { 
            //CORRUPTION_ERROR_ACTION(M); 
        } 
    } 
    else { 
        tchunkptr* RP; 
        if (((R = *(RP = &(X->child[1]))) != 0) || 
                ((R = *(RP = &(X->child[0]))) != 0)) { 
            tchunkptr* CP; 
            while ((*(CP = &(R->child[1])) != 0) || 
                    (*(CP = &(R->child[0])) != 0)) { 
                R = *(RP = CP); 
            } 
            if (ok_address(M, RP)) 
                *RP = 0; 
            else { 
                //    CORRUPTION_ERROR_ACTION(M); 
            } 
        } 
    } 
    if (XP != 0) { 
        tbinptr* H = treebin_at(M, X->index); 
        if (X == *H) { 
            if ((*H = R) == 0)  
                clear_treemap(M, X->index); 
        } 
        else if (ok_address(M, XP)) {
            if (XP->child[0] == X)  
                XP->child[0] = R; 
            else  
                XP->child[1] = R; 
        } 
        else { 
            //            CORRUPTION_ERROR_ACTION(M); 
        }
        if (R != 0) { 
            if (ok_address(M, R)) { 
                tchunkptr C0, C1; 
                R->parent = XP; 
                if ((C0 = X->child[0]) != 0) { 
                    if (ok_address(M, C0)) { 
                        R->child[0] = C0; 
                        C0->parent = R; 
                    } 
                    else {
                        //                      CORRUPTION_ERROR_ACTION(M); 
                    }
                } 
                if ((C1 = X->child[1]) != 0) { 
                    if (ok_address(M, C1)) { 
                        R->child[1] = C1; 
                        C1->parent = R; 
                    } 
                    //    else 
                    //        CORRUPTION_ERROR_ACTION(M); 
                } 
            } 
            //    else 
            //        CORRUPTION_ERROR_ACTION(M); 
        } 
    } 
}
