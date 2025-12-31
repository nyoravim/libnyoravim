#ifndef _NV_MEM_H
#define _NV_MEM_H

#include <stddef.h>

struct nv_allocator {
    void* user;

    void* (*alloc)(void* user, size_t size);
    void* (*realloc)(void* user, void* block, size_t size);
    void* (*calloc)(void* user, size_t elements, size_t element_size);
    void* (*free)(void* user, void* block);
};

/* requires that allocator stays valid */
void nv_set_allocator(const struct nv_allocator* allocator);

void* nv_alloc(size_t size);
void* nv_realloc(void* block, size_t size);
void* nv_calloc(size_t elements, size_t element_size);
void nv_free(void* block);

#endif
