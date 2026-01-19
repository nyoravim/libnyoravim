#include "nyoravim/log.h"

#include <stdio.h>
#include <string.h>

int main(int argc, const char** argv) {
    struct nv_logger_sink sinks[2];
    memset(sinks, 0, sizeof(sinks));

    nv_create_stdout_sink(&sinks[0]);
    sinks[1].user = fopen("test.log", "w");
    sinks[1].on_log = nv_log_print_to_file;

    struct nv_logger logger;
    memset(&logger, 0, sizeof(struct nv_logger));

    logger.sink_count = 2;
    logger.sinks = sinks;

    nv_set_default_logger(&logger);

    NV_LOG_TRACE("testing trace");
    NV_LOG_DEBUG("testing debug");
    NV_LOG_INFO("testing info");
    NV_LOG_WARN("testing warn");
    NV_LOG_ERROR("testing error");

    const char* names[] = { "world", "foo", "bar", "baz" };
    for (uint32_t i = 0; i < 4; i++) {
        const char* name = names[i];
        NV_LOG_INFO("hello %s (#%u)", name, i + 1);
    }

    fclose(sinks[1].user);
}
