#include "nyoravim/list.h"
#include "nyoravim/mem.h"

void nv_list_init(struct nv_list* list) {
    list->head = NULL;
    list->tail = NULL;
}

bool nv_list_clear(struct nv_list* list, void (*free_func)(void* user, void* value), void* user) {
    if (!list) {
        return false;
    }

    struct nv_list_node* current = list->head;
    while (current) {
        if (free_func) {
            free_func(user, current->value);
        }

        struct nv_list_node* next = current->next;
        nv_free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;

    return true;
}

static struct nv_list_node* new_node(void* value) {
    struct nv_list_node* node = nv_alloc(sizeof(struct nv_list_node));
    node->value = value;

    return node;
}

struct nv_list_node* nv_list_push_front(struct nv_list* list, void* value) {
    if (!list) {
        /* failure */
        return NULL;
    }

    struct nv_list_node* prev_head = list->head;

    struct nv_list_node* node = new_node(value);
    node->next = prev_head;
    node->prev = NULL;

    list->head = node;
    if (prev_head) {
        prev_head->prev = node;
    } else {
        /* no nodes were in list */
        list->tail = node;
    }

    return node;
}

struct nv_list_node* nv_list_push_back(struct nv_list* list, void* value) {
    if (!list) {
        /* failure */
        return NULL;
    }

    struct nv_list_node* prev_tail = list->tail;

    struct nv_list_node* node = new_node(value);
    node->prev = prev_tail;
    node->next = NULL;

    list->tail = node;
    if (prev_tail) {
        prev_tail->next = node;
    } else {
        /* no nodes were in list */
        list->head = node;
    }

    return node;
}

struct nv_list_node* nv_list_insert_after(struct nv_list* list, struct nv_list_node* reference,
                                          void* value) {
    if (!list) {
        /* failure */
        return NULL;
    }

    if (!reference) {
        return nv_list_push_front(list, value);
    }

    struct nv_list_node* next = reference->next;

    struct nv_list_node* node = new_node(value);
    node->prev = reference;
    node->next = next;

    reference->next = node;
    if (next) {
        next->prev = node;
    } else {
        /* reference was tail */
        list->tail = node;
    }

    return node;
}

struct nv_list_node* nv_list_insert_before(struct nv_list* list, struct nv_list_node* reference,
                                           void* value) {
    if (!list) {
        /* failure */
        return NULL;
    }

    if (!reference) {
        return nv_list_push_back(list, value);
    }

    struct nv_list_node* prev = reference->prev;

    struct nv_list_node* node = new_node(value);
    node->next = reference;
    node->prev = prev;

    reference->prev = node;
    if (prev) {
        prev->next = node;
    } else {
        /* reference was head */
        list->head = node;
    }

    return node;
}

/* assumes both params are not null */
static void unlink_node(struct nv_list* list, struct nv_list_node* node) {
    struct nv_list_node* next = node->next;
    struct nv_list_node* prev = node->prev;

    if (prev) {
        prev->next = next;
    } else {
        /* node was head */
        list->head = next;
    }

    if (next) {
        next->prev = prev;
    } else {
        /* node was tail */
        list->tail = prev;
    }
}

bool nv_list_remove(struct nv_list* list, struct nv_list_node* node) {
    if (!list || !node) {
        /* failure */
        return false;
    }

    unlink_node(list, node);
    nv_free(node);

    return true;
}
