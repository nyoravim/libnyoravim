#include <nyoravim/arena.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(int argc, const char** argv) {
    size_t arena_size = 1 * 1024 * 1024; /* 1 mb */

    nv_arena_t* arena = nv_arena_create(arena_size);
    assert(arena);

    const char* strings[] = { "foo", "bar", "baz", "hi nora" };
    char* copies[4];

    for (uint32_t i = 0; i < 4; i++) {
        const char* str = strings[i];
        size_t length = strlen(str);

        char* copy = nv_arena_alloc(arena, length + 1);
        assert(copy);

        strncpy(copy, str, length + 1);
        copies[i] = copy;
    }

    for (uint32_t i = 0; i < 4; i++) {
        assert(strcmp(copies[i], strings[i]) == 0);
    }

    char* dst = copies[0];
    char* src = copies[3];

    size_t src_len = strlen(src);
    dst = nv_arena_realloc(arena, dst, src_len + 1);
    assert(strcmp(dst, strings[0]) == 0);

    strncpy(dst, src, src_len + 1);
    assert(strcmp(dst, strings[3]) == 0);

    nv_arena_free(arena, copies[2]);
    nv_arena_free(arena, copies[1]);
    nv_arena_free(arena, copies[3]);
    nv_arena_free(arena, copies[0]);

    nv_arena_destroy(arena);
    return 0;
}
