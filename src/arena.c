#include "nyoravim/arena.h"

#include <stdbool.h>
#include <assert.h>
#include <string.h>

/* linux! */
#include <unistd.h>
#include <sys/mman.h>

#define ALLOC_ALIGNMENT sizeof(size_t)

static size_t align_up_to_pow2(size_t x, size_t pow2) {
    size_t mask = pow2 - 1;
    return (x + mask) & ~mask;
}

static size_t align_down_to_pow2(size_t x, size_t pow2) {
    size_t mask = pow2 - 1;
    return x & ~mask;
}

typedef struct nv_arena {
    size_t pagesize;
    size_t total_size;

    size_t begin;
    struct arena_allocation* first;
} nv_arena_t;

struct arena_allocation {
    struct arena_allocation* previous;
    struct arena_allocation* next;

    /* size of allocation including this structure */
    size_t size;

    /* size of user memory itself */
    size_t used_size;
};

static bool commit_mem(void* block, size_t size) {
    return mprotect(block, size, PROT_READ | PROT_WRITE) == 0;
}

static bool uncommit_mem(void* block, size_t size) {
    if (mprotect(block, size, PROT_NONE) != 0) {
        return false;
    }

    if (madvise(block, size, MADV_DONTNEED) != 0) {
        return false;
    }

    return true;
}

nv_arena_t* nv_arena_create(size_t size) {
    size_t pagesize = (size_t)sysconf(_SC_PAGESIZE);
    size_t total_size = align_up_to_pow2(size, pagesize);

    if (sizeof(nv_arena_t) > total_size) {
        return NULL;
    }

    void* mapped = mmap(NULL, total_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!mapped || mapped == MAP_FAILED) {
        return NULL;
    }

    /* commit just the metadata */
    size_t meta_size = align_up_to_pow2(sizeof(nv_arena_t), pagesize);
    if (!commit_mem(mapped, meta_size)) {
        /* cant commit for writing */

        munmap(mapped, meta_size);
        return NULL;
    }

    /* metadata at the beginning */
    nv_arena_t* arena = mapped;

    arena->pagesize = pagesize;
    arena->total_size = total_size;

    arena->begin = meta_size;
    arena->first = NULL;

    return arena;
}

void nv_arena_destroy(nv_arena_t* arena) {
    if (!arena) {
        return;
    }

    /* quite shrimple */
    munmap(arena, arena->total_size);
}

size_t nv_arena_get_commit_size(const nv_arena_t* arena) { return arena->pagesize; }

struct free_region {
    struct arena_allocation* previous;
    struct arena_allocation* next;

    void* start;
    size_t available;
};

static size_t region_free_after(nv_arena_t* arena, struct arena_allocation* allocation) {
    void* start;
    void* end;

    if (allocation) {
        start = (void*)allocation + allocation->size;
        end = allocation->next;
    } else {
        start = (void*)arena + arena->begin;
        end = arena->first;
    }

    if (!end) {
        end = (void*)arena + arena->total_size;
    }

    return end - start;
}

static bool find_free_region(nv_arena_t* arena, size_t size, struct free_region* region) {
    assert(size % ALLOC_ALIGNMENT == 0);

    struct arena_allocation* alloc = NULL;
    do {
        size_t available = region_free_after(arena, alloc);
        struct arena_allocation* next = alloc ? alloc->next : arena->first;

        if (available >= size) {
            region->previous = alloc;
            region->next = next;

            region->start = alloc ? (void*)alloc + alloc->size : (void*)arena + arena->begin;
            region->available = available;

            return true;
        }

        alloc = next;
    } while (alloc != NULL);

    return false;
}

void* nv_arena_alloc(nv_arena_t* arena, size_t size) {
    assert(arena);

    if (size == 0) {
        /* dont bother */
        return NULL;
    }

    size_t size_required = size + sizeof(struct arena_allocation);
    size_t alloc_size = align_up_to_pow2(size_required, ALLOC_ALIGNMENT);

    struct free_region region;
    if (!find_free_region(arena, alloc_size, &region)) {
        /* too fragmented/not enough space */
        return NULL;
    }

    assert(region.available >= alloc_size);

    /* commit pages */
    void* prev_end = region.previous ? (void*)region.previous + region.previous->size
                                     : (void*)arena + arena->begin;

    void* commit_begin = (void*)align_up_to_pow2((size_t)prev_end, arena->pagesize);
    void* commit_end = (void*)align_up_to_pow2((size_t)region.start + alloc_size, arena->pagesize);

    if (region.next) {
        void* next_commit_begin = (void*)align_down_to_pow2((size_t)region.next, arena->pagesize);
        if (next_commit_begin < commit_end) {
            commit_end = next_commit_begin;
        }
    }

    if (commit_end > commit_begin) {
        /* commit! */
        size_t commit_size = commit_end - commit_begin;
        if (!commit_mem(commit_begin, commit_size)) {
            /* failed to commit memory */
            return NULL;
        }
    }

    struct arena_allocation* alloc = region.start;
    alloc->size = alloc_size;
    alloc->used_size = size;

    /* link this to others */
    alloc->previous = region.previous;
    alloc->next = region.next;

    /* link forward */
    if (alloc->previous) {
        alloc->previous->next = alloc;
    } else {
        arena->first = alloc;
    }

    /* link backward */
    if (alloc->next) {
        alloc->next->previous = alloc;
    }

    return (void*)alloc + sizeof(struct arena_allocation);
}

static bool recommit_block(nv_arena_t* arena, struct arena_allocation* alloc,
                           size_t new_alloc_size) {
    void* commit_begin = (void*)align_up_to_pow2((size_t)alloc + alloc->size, arena->pagesize);
    void* commit_end = (void*)align_up_to_pow2((size_t)alloc + new_alloc_size, arena->pagesize);

    if (alloc->next) {
        void* next_commit_begin = (void*)align_down_to_pow2((size_t)alloc->next, arena->pagesize);
        if (next_commit_begin < commit_end) {
            commit_end = next_commit_begin;
        }
    }

    if (commit_end > commit_begin) {
        size_t commit_size = commit_end - commit_begin;
        if (!commit_mem(commit_begin, commit_size)) {
            /* failed to commit */
            return false;
        }
    }

    return true;
}

void* nv_arena_realloc(nv_arena_t* arena, void* block, size_t size) {
    assert(arena);
    if (!block) {
        return nv_arena_alloc(arena, size);
    }

    assert(block >= (void*)arena + arena->begin + sizeof(struct arena_allocation));

    size_t new_required = size + sizeof(struct arena_allocation);
    size_t new_alloc_size = align_up_to_pow2(new_required, ALLOC_ALIGNMENT);

    struct arena_allocation* alloc = block - sizeof(struct arena_allocation);
    if (new_alloc_size <= alloc->size) {
        /* nothing to do */
        alloc->used_size = size;
        return block;
    }

    size_t available = region_free_after(arena, alloc);
    if (new_alloc_size <= alloc->size + available) {
        /* can extend allocation without moving */

        if (!recommit_block(arena, alloc, new_alloc_size)) {
            return NULL;
        }

        alloc->size = new_alloc_size;
        alloc->used_size = size;

        /* in same allocation */
        return block;
    }

    /* if all else fails, reallocate and move */

    void* new_block = nv_arena_alloc(arena, size);
    memcpy(new_block, block, alloc->used_size);

    nv_arena_free(arena, block);
    return new_block;
}

void nv_arena_free(nv_arena_t* arena, void* block) {
    assert(arena);
    if (!block) {
        return;
    }

    assert(block >= (void*)arena + arena->begin + sizeof(struct arena_allocation));
    struct arena_allocation* alloc = block - sizeof(struct arena_allocation);

    /* unlink forward */
    if (alloc->previous) {
        alloc->previous->next = alloc->next;
    } else {
        arena->first = alloc->next;
    }

    /* unlink backward */
    if (alloc->next) {
        alloc->next->previous = alloc->previous;
    }

    void* commit_begin = (void*)align_down_to_pow2((size_t)alloc, arena->pagesize);
    if (alloc->previous) {
        void* prev_commit_end = (void*)align_up_to_pow2(
            (size_t)alloc->previous + alloc->previous->size, arena->pagesize);

        if (prev_commit_end > commit_begin) {
            commit_begin = prev_commit_end;
        }
    }

    void* commit_end = (void*)align_up_to_pow2((size_t)alloc + alloc->size, arena->pagesize);
    if (alloc->next) {
        void* next_commit_begin = (void*)align_down_to_pow2((size_t)alloc->next, arena->pagesize);
        if (next_commit_begin < commit_end) {
            commit_end = next_commit_begin;
        }
    }

    if (commit_end > commit_begin) {
        size_t commit_size = commit_end - commit_begin;
        uncommit_mem(commit_begin, commit_size);
    }
}
