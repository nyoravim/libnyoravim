#ifndef _NV_LIST_H
#define _NV_LIST_H

#include <stdbool.h>

struct nv_list_node {
    struct nv_list_node* next;
    struct nv_list_node* prev;

    void* value;
};

struct nv_list {
    struct nv_list_node* head;
    struct nv_list_node* tail;
};

/* initializes list to empty. */
void nv_list_init(struct nv_list* list);

/* clears the list.
 * free_func will be called with user passed through on each node with its corresponding value.
 * returns true on success; false on failure. */
bool nv_list_clear(struct nv_list* list, void (*free_func)(void* user, void* value), void* user);

/* returns pushed node on success; NULL on failure. */
struct nv_list_node* nv_list_push_front(struct nv_list* list, void* value);

/* returns pushed node on success; NULL on failure. */
struct nv_list_node* nv_list_push_back(struct nv_list* list, void* value);

/* if reference is NULL, inserts node at back.
 * returns inserted node on success; NULL on failure. */
struct nv_list_node* nv_list_insert_after(struct nv_list* list, struct nv_list_node* reference,
                                          void* value);

/* if reference is NULL, inserts node at front.
 * returns inserted node on success; NULL on failure. */
struct nv_list_node* nv_list_insert_before(struct nv_list* list, struct nv_list_node* reference,
                                           void* value);

/* removes node from list.
 * note: will not free data contained within. that is the user's responsibility.
 * returns true on success; false on failure. */
bool nv_list_remove(struct nv_list* list, struct nv_list_node* node);

#endif
