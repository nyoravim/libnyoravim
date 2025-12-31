#ifndef _NV_UTIL_H
#define _NV_UTIL_H

#include <stddef.h>

/* hashes string */
size_t nv_hash_string(const char* str);

/* duplicates data in "str" up to '\0' into new chunk.
 * returns NULL if no string is provided or if allocation fails. */
char* nv_strdup(const char* str);

#endif
