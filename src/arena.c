#include "nyoravim/arena.h"

#include <stdbool.h>
#include <assert.h>
#include <string.h>

/* linux! */
#include <unistd.h>
#include <sys/mman.h>

static size_t align_up_to_pow2(size_t x, size_t pow2) {
    size_t mask = pow2 - 1;
    return (x + mask) & ~mask;
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
    size_t commit_size;

    /* size of user memory itself */
    size_t size;
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
        start = (void*)allocation + allocation->commit_size;
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

static bool find_free_region(nv_arena_t* arena, size_t commit_size, struct free_region* region) {
    assert(commit_size % arena->pagesize == 0);

    struct arena_allocation* alloc = NULL;
    do {
        size_t available = region_free_after(arena, alloc);
        struct arena_allocation* next = alloc ? alloc->next : arena->first;

        if (available >= commit_size) {
            region->previous = alloc;
            region->next = next;

            region->start = alloc ? (void*)alloc + alloc->commit_size : (void*)arena + arena->begin;
            region->available = available;

            return true;
        }

        alloc = next;
    } while (alloc != NULL);

    return false;
}

void* nv_arena_alloc(nv_arena_t* arena, size_t size) {
    assert(arena);

    size_t size_required = size + sizeof(struct arena_allocation);
    size_t commit_size = align_up_to_pow2(size_required, arena->pagesize);

    struct free_region region;
    if (!find_free_region(arena, commit_size, &region)) {
        /* too fragmented/not enough space */
        return NULL;
    }

    assert(region.available >= commit_size);
    if (!commit_mem(region.start, commit_size)) {
        /* failed to commit */
        return NULL;
    }

    struct arena_allocation* alloc = region.start;
    alloc->commit_size = commit_size;
    alloc->size = size;

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

void* nv_arena_realloc(nv_arena_t* arena, void* block, size_t size) {
    assert(arena);
    if (!block) {
        return nv_arena_alloc(arena, size);
    }

    assert(block >= (void*)arena + arena->begin + sizeof(struct arena_allocation));

    size_t new_required = size + sizeof(struct arena_allocation);
    size_t new_commit_size = align_up_to_pow2(new_required, arena->pagesize);

    struct arena_allocation* alloc = block - sizeof(struct arena_allocation);
    if (new_commit_size <= alloc->commit_size) {
        /* nothing to do */
        alloc->size = size;
        return block;
    }

    size_t available = region_free_after(arena, alloc);
    size_t available_commit = alloc->commit_size + available;
    
    if (new_commit_size <= available_commit) {
        /* can extend commit without moving */

        size_t to_commit = new_commit_size - alloc->commit_size;
        void* new_commit_begin = (void*)alloc + alloc->commit_size;

        if (!commit_mem(new_commit_begin, to_commit)) {
            /* failed to commit */

            return NULL;
        }

        alloc->commit_size = new_commit_size;
        alloc->size = size;

        /* in same allocation */
        return block;
    }

    /* if all else fails, reallocate and move */

    void* new_block = nv_arena_alloc(arena, size);
    memcpy(new_block, block, alloc->size);

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

    uncommit_mem(alloc, alloc->commit_size);
}
