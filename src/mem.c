#include "nyoravim/mem.h"

#include <malloc.h>

static const struct nv_allocator* s_allocator = NULL;

void nv_set_allocator(const struct nv_allocator* allocator) { s_allocator = allocator; }

void* nv_alloc(size_t size) {
    if (s_allocator) {
        return s_allocator->alloc(s_allocator->user, size);
    } else {
        return malloc(size);
    }
}

void* nv_realloc(void* block, size_t size) {
    if (s_allocator) {
        return s_allocator->realloc(s_allocator->user, block, size);
    } else {
        return realloc(block, size);
    }
}

void* nv_calloc(size_t elements, size_t element_size) {
    if (s_allocator) {
        return s_allocator->calloc(s_allocator->user, elements, element_size);
    } else {
        return calloc(elements, element_size);
    }
}

void nv_free(void* block) {
    if (s_allocator) {
        s_allocator->free(s_allocator->user, block);
    } else {
        free(block);
    }
}
