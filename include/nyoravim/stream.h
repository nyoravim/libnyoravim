#ifndef _NV_STREAM_H
#define _NV_STREAM_H

#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>

struct nv_stream {
    size_t (*write)(void* user, const void* data, size_t size);
    size_t (*read)(void* user, void* buffer, size_t capacity);
    bool (*flush)(void* user);

    void* user;
};

void nv_stream_wrap_file(struct nv_stream* stream, FILE* file);

bool nv_stream_write_chunk(const struct nv_stream* stream, const void* data, size_t size);
bool nv_stream_read_chunk(const struct nv_stream* stream, void* buffer, size_t size);

#endif
