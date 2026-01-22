#ifndef _NV_ARENA_H
#define _NV_ARENA_H

#include <stddef.h>

typedef struct nv_arena nv_arena_t;

nv_arena_t* nv_arena_create(size_t size);
void nv_arena_destroy(nv_arena_t* arena);

size_t nv_arena_get_commit_size(const nv_arena_t* arena);

void* nv_arena_alloc(nv_arena_t* arena, size_t size);
void* nv_arena_realloc(nv_arena_t* arena, void* block, size_t size);
void nv_arena_free(nv_arena_t* arena, void* block);

#endif
