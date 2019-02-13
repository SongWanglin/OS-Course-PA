/* llist_t: A threadsafe linked list that uses a hand-over-hand
 *      (a.k.a. lock coupling) locking stragety
 */

#include "wrappers.h"

typedef struct __node_t {
    int key;
    struct __node_t *next;
    pthread_mutex_t lock;
} node_t;

typedef struct __llist_t {
    node_t *head;
    pthread_mutex_t lock;
} llist_t;

/* ll_init: initialize the new list */
void ll_init(llist_t *l) {
    Pthread_mutex_init(&l->lock, NULL);
    l->head = NULL;
}

/* ll_insert: create a new node with the value of key and 
 *      prepend it to the front of the list
 */
int ll_insert(llist_t *l, int key) {
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) {    // check malloc success
        return -1;
    } 
    new->key = key;
    Pthread_mutex_init(&new->lock, NULL); 
    Pthread_mutex_lock(&l->lock);   // get llist_t lock
    new->next = l->head;
    l->head = new;
    Pthread_mutex_unlock(&l->lock); // release lock
    return 0;
}

/* ll_indexof: find key, and return the index of the first node with 
 *      the value key in the list. If key is not in the list, return -1.
 */
int ll_indexof(llist_t *l, int key) {
    Pthread_mutex_lock(&l->lock);       // get llist_t lock
    if (l->head == NULL) {              // if the list is empty
        Pthread_mutex_unlock(&l->lock); // release llist_t lock and exit
        return -1;
    }
    node_t *node = l->head;
    Pthread_mutex_lock(&node->lock);   // get first node lock
    Pthread_mutex_unlock(&l->lock);    // release llist_t lock
    int i;
    for (i = 0; node->key != key; i++) {
        if (!node->next) {
            i = -1;
            break;
        }
        Pthread_mutex_lock(&node->next->lock); // get next node's lock
        Pthread_mutex_unlock(&node->lock);     // unlock this node's lock
        node = node->next;
    }
    Pthread_mutex_unlock(&node->lock);  // release node lock
    return i;
}

/* ll_length: return the number of nodes in the list */
int ll_length(llist_t *l) {
    Pthread_mutex_lock(&l->lock);       // get llist_t lock
    if (l->head == NULL) {              // if list is empty
        Pthread_mutex_unlock(&l->lock); // release llist_t lock and exit
        return 0;
    }
    node_t *node = l->head;
    Pthread_mutex_lock(&node->lock);    // get first node lock
    Pthread_mutex_unlock(&l->lock);     // release llist_t lock
    int i = 1;
    while (node->next) {
        Pthread_mutex_lock(&node->next->lock);  // get next node lock
        Pthread_mutex_unlock(&node->lock);      // release this node lock
        i++;
        node = node->next;
    }
    Pthread_mutex_unlock(&node->lock); // release llist_t lock
    return i;
}
