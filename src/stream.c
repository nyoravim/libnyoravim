#include "nyoravim/stream.h"

#include <assert.h>

static size_t write_file(void* user, const void* data, size_t size) {
    return fwrite(data, 1, size, user);
}

static size_t read_file(void* user, void* buffer, size_t capacity) {
    return fread(buffer, 1, capacity, user);
}

static bool flush_file(void* user) { return fflush(user) == 0; }

void nv_stream_wrap_file(struct nv_stream* stream, FILE* file) {
    stream->read = read_file;
    stream->write = write_file;
    stream->flush = flush_file;

    stream->user = file;
}

bool nv_stream_write_chunk(const struct nv_stream* stream, const void* data, size_t size) {
    if (!stream->write) {
        return false;
    }

    while (size > 0) {
        size_t bytes_written = stream->write(stream->user, data, size);
        if (bytes_written == 0) {
            /* stream wont accept any more */
            return false;
        }

        assert(bytes_written <= size);

        data += bytes_written;
        size -= bytes_written;
    }

    return true;
}

bool nv_stream_read_chunk(const struct nv_stream* stream, void* buffer, size_t size) {
    if (!stream->read) {
        return false;
    }

    while (size > 0) {
        size_t bytes_read = stream->read(stream->user, buffer, size);
        if (bytes_read == 0) {
            /* EOF */
            return false;
        }

        assert(bytes_read <= size);

        buffer += bytes_read;
        size -= bytes_read;
    }

    return true;
}
