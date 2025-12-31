#include "nyoravim/util.h"
#include "nyoravim/mem.h"

#include <string.h>

/* https://stackoverflow.com/questions/7666509/hash-function-for-string */
size_t nv_hash_string(const char* str) {
    size_t hash = 5381; /* magic */

    for (const char* cursor = str; *cursor != '\0'; cursor++) {
        char c = *cursor;

        hash = ((hash << 5) + hash) + (size_t)c; /* hash * 33 + c? what the fuck */
    }

    return hash;
}

char* nv_strdup(const char* str) {
    if (!str) {
        /* no data provided */
        return NULL;
    }

    size_t len = strlen(str);
    size_t size = (len + 1) * sizeof(char);

    char* dst = nv_alloc(size);
    if (!dst) {
        /* failed */
        return NULL;
    }

    strncpy(dst, str, len);
    return dst;
}
