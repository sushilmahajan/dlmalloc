#include "bins.h"

void insert_small_chunk(mstate *state, chunkptr ptr, size_t size) { 
    index_t i  = SMALL_INDEX(size);
    chunkptr base = smallbin_at(state, i);
    chunkptr front = base;

    assert(size >= MIN_CHUNK_SIZE);                                                

    if (!smallmap_is_marked(state, i)) {        // Insert 1st chunk
        mark_smallmap(state, i);
    }
    else {   // Insert at next position 
        front = base->next;
    }

    base->next = ptr;
    front->prev = ptr;
    ptr->next = front;
    ptr->prev = base;

#if DEBUG_BINS
    printf("\n************Inserting small chunk of size %ld**************\n\n", size);
    printf("Small chunk of size %ld is inserted in smallbin with index %ld\n", size, i);
#endif
}                                                                              

chunkptr remove_small_chunk_by_address(mstate *state, chunkptr ptr) {
    chunkptr base, front, p;

    size_t size = CHUNKSIZE(ptr);
    index_t i = SMALL_INDEX(size);

    base = smallbin_at(state, i);
    p = base->prev;

#if DEBUG_BINS
    printf("\n************Removing small chunk of size %ld from smallbin of index %ld using address**************\n\n", size, i);
    bool empty = 0;
#endif

    while ((p != ptr) && (p != base)) {
        p = p->prev;
    }

    if (p == base) {
#if DEBUG_BINS
        printf("Small chunk of given address not found\n");
#endif
        return NULL;
    }

    base = p->prev;
    front = p->next;

    assert(front != ptr);
    assert(base != ptr);

    if ((front == smallbin_at(state, i)) && (front == base)) {     
        clear_smallmap(state, i);   // Remove only chunk, hence clear map
#if DEBUG_BINS
        empty = 1;
#endif
    }

    front->prev = base;
    base->next = front;
#if DEBUG_BINS
    printf("Small chunk of size %ld is removed from smallbin with index %ld\n", size, i);
    if (empty) {
        printf("Smallbin with index %ld is empty now\n", i);
    }
#endif

    return ptr;
}

chunkptr remove_small_chunk(mstate *state, size_t size) {                                      
    chunkptr base, front, ptr;      
    index_t i;

#if DEBUG_BINS
    printf("\n************Removing small chunk of size %ld**************\n\n", size);
    bool empty = 0;
#endif

    i = SMALL_INDEX(size); 
    base = smallbin_at(state, i);
    ptr = base->next;   
    assert(ptr != NULL);
    front = ptr->next;

    assert(front != ptr);
    assert(base != ptr);

    if ((front == smallbin_at(state, i)) && (front == base)) {     
        clear_smallmap(state, i);   // Remove only chunk, hence clear map
#if DEBUG_BINS
        empty = 1;
#endif
    }

    front->prev = base;
    base->next = front;
#if DEBUG_BINS
    printf("Small chunk of size %ld is removed from smallbin with index %ld\n", size, i);
    if (empty) {
        printf("Smallbin with index %ld is empty now\n", i);
    }
#endif

    return ptr;
}

void insert_large_chunk(mstate *state, tchunkptr ptr, size_t size) {
    index_t i;
    compute_tree_index(size, i);
    tchunkptr *p;
    p = treebin_at(state, i);
    ptr->left = ptr->right = 0; 

#if DEBUG_BINS 
    printf("\n************Inserting large chunk of size %ld**************\n\n", size);
#endif
    if (!treemap_is_marked(state, i)) { // Insert fisrt chunk at base of treebin
        mark_treemap(state, i);          
        *p = ptr; 
        ptr->next = ptr->prev = ptr;
#if DEBUG_BINS 
        printf("Inserted first tree chunk of size %ld in treebin at index %ld\n", size, i);
#endif
    } 
    else { 
        while (1) {
            if (*p == NULL) {
                *p = ptr;
                ptr->next = ptr->prev = ptr;
#if DEBUG_BINS 
                printf("Inserted tree chunk of size %ld in treebin at index %ld\n", size, i);
#endif
                break;
            }
            else if (size == CHUNKSIZE(*p)) {
                tchunkptr front = (*p)->next; 
                (*p)->next = front->prev = ptr; 
                ptr->next = front; 
                ptr->prev = *p; 
#if DEBUG_BINS 
                printf("Inserted tree chunk of size %ld in treebin at index %ld in a linked list\n", size, i);
#endif
                break; 
            }
            else if (size > CHUNKSIZE(*p)) {
                p = &(*p)->right;
#if DEBUG_BINS 
                printf("Traversing treebin at index %ld & selected right child\n", i);
#endif
            }
            else {
                p = &(*p)->left;
#if DEBUG_BINS 
                printf("Traversing treebin at index %ld & selected left child\n", i);
#endif
            }
        }
    }
}

uint8_t search_large_chunk(mstate *state, tchunkptr *ptr,  size_t size) {
    index_t i;
    compute_tree_index(size, i);
    tchunkptr *p;
    p = treebin_at(state, i);

#if DEBUG_BINS 
    printf("\n************Searching large chunk of size %ld**************\n\n", size);
#endif
    while (1) {
        if (*p == NULL) {
            if (CHUNKSIZE(*ptr) < size) {
                *ptr = NULL;
            }
#if DEBUG_BINS 
            else {
                printf("Not Found, best found choice is %ld\n", CHUNKSIZE(*ptr));
            }
#endif
            return 0;
        }
        else if (CHUNKSIZE(*p) == size) {
            *ptr = *p;
#if DEBUG_BINS 
            printf("Chunk of size %ld is found\n", size);
#endif
            return 1;
        }
        else if (CHUNKSIZE(*p) < size) {
            *ptr = *p;
            p = &(*p)->right;
#if DEBUG_BINS 
            printf("Traversing treebin at index %ld & selected right child\n", i);
#endif
        }
        else {
            *ptr = *p;
            p = &(*p)->left;
#if DEBUG_BINS 
            printf("Traversing treebin at index %ld & selected left child\n", i);
#endif
        }
    }
}

tchunkptr min_sized_chunk(tchunkptr ptr) { 
    tchunkptr current = ptr, prev = ptr; 

    /* loop sizeown to find the leftmost leaf */
    while (current && current->left != NULL) {
        ptr = current;
        current = current->left; 
    }

    return prev; 
}  

tchunkptr remove_large_chunk_by_address(mstate *state, tchunkptr ptr) {
    tchunkptr *base, *p = &ptr, temp = ptr;
    index_t i;
    size_t size = CHUNKSIZE(ptr);
    compute_tree_index(size, i);
    base = treebin_at(state, i);

#if DEBUG_BINS 
    printf("\n************Removing large chunk of size %ld from tree of index %ld by address**************\n\n", size, i);
#endif
    if (ptr->next != ptr) {
        tchunkptr front, back;
#if DEBUG_BINS 
        printf("Chunk of size %ld found, replacing it with node of same size in linked list\n", size);
#endif
        back = ptr->prev;
        front = ptr->next;
        front->prev = back;
        back->next = front;
        return ptr;
    }
    else {
        if (*base == ptr) {
#if DEBUG_BINS 
            printf("Only chunk left in treebin of index %ld of size %ld is being removed\n", i, size);
#endif
            clear_treemap(state, i);
        }
        if ((*p)->left == NULL) {
#if DEBUG_BINS 
            printf("Chunk of size %ld found, replacing it with right child\n", size);
#endif
            *p = (*p)->right;
        }
        else if ((*p)->right == NULL) {
#if DEBUG_BINS 
            printf("Chunk of size %ld found, replacing it with left child\n", size);
#endif
            *p = (*p)->left;
        }
        else {
            tchunkptr prev_ptr = min_sized_chunk((*p)->right);
#if DEBUG_BINS 
            printf("Chunk of size %ld found, replacing it with minimum sized chunk in right sub-tree\n", size);
#endif
            tchunkptr temp1 = (*p)->left, temp2 = (*p)->right;
            if (prev_ptr->left != NULL) {
                **p = *(prev_ptr->left);
                (*p)->left = temp1;
                (*p)->right = temp2;
                prev_ptr->left = NULL;
            }
            else {
                **p = *prev_ptr;
                (*p)->left = temp1;
                (*p)->right = temp2;
                (*p)->right = NULL;
            }
        }
        return temp;
    }
}

tchunkptr remove_large_chunk(mstate *state, size_t size) {
    tchunkptr *p, *base;
    index_t i;
    compute_tree_index(size, i);

#if DEBUG_BINS 
    printf("\n************Removing large chunk of size %ld from tree of index %ld**************\n\n", size, i);
#endif
    base = p = treebin_at(state, i);

    while (1) {
        if (*p == NULL) {
#if DEBUG_BINS 
            printf("Chunk of size %ld not found\n", size);
#endif
            return *p;
        }
        else if (CHUNKSIZE(*p) == size) {
            tchunkptr ptr = *p;
            if (ptr->next != ptr) {
                tchunkptr front, back;
#if DEBUG_BINS 
                printf("Chunk of size %ld found, replacing it with node of same size in linked list\n", size);
#endif
                back = ptr->prev;
                front = ptr->next;
                front->prev = back;
                back->next = front;
                return ptr;
            }
            else {
                if (ptr == *base) {
#if DEBUG_BINS 
                    printf("Only chunk left in treebin of index %ld of size %ld is being removed\n", i, size);
#endif
                    clear_treemap(state, i);
                }
                if ((*p)->left == NULL) {
#if DEBUG_BINS 
                    printf("Chunk of size %ld found, replacing it with right child\n", size);
#endif
                    *p = (*p)->right;
                    return ptr;
                }
                else if ((*p)->right == NULL) {
#if DEBUG_BINS 
                    printf("Chunk of size %ld found, replacing it with left child\n", size);
#endif
                    *p = (*p)->left;
                    return ptr;
                }
                else {
                    tchunkptr prev_ptr = min_sized_chunk((*p)->right);
#if DEBUG_BINS 
                    printf("Chunk of size %ld found, replacing it with minimum sized chunk in right sub-tree\n", size);
#endif
                    tchunkptr temp1 = (*p)->left, temp2 = (*p)->right;
                    if (prev_ptr->left != NULL) {
                        **p = *(prev_ptr->left);
                        (*p)->left = temp1;
                        (*p)->right = temp2;
                        prev_ptr->left = NULL;
                    }
                    else {
                        **p = *prev_ptr;
                        (*p)->left = temp1;
                        (*p)->right = temp2;
                        (*p)->right = NULL;
                    }
                    return ptr;
                }
            }
        }
        else if (size > CHUNKSIZE(*p)) {
#if DEBUG_BINS 
            printf("Traversing treebin at index %ld & selected right child\n", i);
#endif
            p = &(*p)->right;
        }
        else {
#if DEBUG_BINS 
            printf("Traversing treebin at index %ld & selected left child\n", i);
#endif
            p = &(*p)->left;
        }
    }
}
