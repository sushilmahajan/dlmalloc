#include "queue.h"

void qinit(queue *q) {
    *q = NULL;
}

bool qfull(queue *q) {
    node *temp_node = (node*)my_malloc(sizeof(node));
    if (!temp_node) {
        return 1;
    }
    my_free(temp_node);
    return 0;
}

bool qempty(queue *q) {
    if (*q == NULL) {
        return 1;
    }
    return 0;
}

void enq(queue *q, data d) {
    node *new_node;
    new_node = (node*)my_malloc(sizeof(node));
    new_node->d = d;
//    if (qfull(q)) {
//        return;
//    }
    if (qempty(q)) {
        *q = new_node;
        new_node->next = new_node;
    }
    else {
        new_node->next = (*q)->next;
        (*q)->next = new_node;
        *q = new_node;
    }
}

data deq(queue *q) {
    node *first_node = (*q)->next;
    data d;
    if (*q == first_node) {
        *q = NULL;
    }    
    else {    
        (*q)->next = first_node->next;
    }
    d = first_node->d;
    my_free(first_node);
    return d;
}
     
