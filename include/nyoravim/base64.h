#ifndef _NV_BASE64_H
#define _NV_BASE64_H

#include <stddef.h>

/* return allocated with nv_alloc */
char* nv_base64_encode(const void* src, size_t size);

/* dst can be null. if not null, expects enough memory to fill, as returned.
 * returns 0 in all cases if decoding fails */
size_t nv_base64_decode(const char* src, void* dst);

#endif
