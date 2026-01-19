#ifndef _NV_LOG_H
#define _NV_LOG_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

enum {
    NV_LOG_LEVEL_TRACE = 0,
    NV_LOG_LEVEL_DEBUG = 1,
    NV_LOG_LEVEL_INFO = 2,
    NV_LOG_LEVEL_WARN = 3,
    NV_LOG_LEVEL_ERROR = 4,
};

struct nv_log_event {
    uint32_t line;
    const char* file;
    
    uint32_t level;
    const char* message;

    time_t timestamp;
};

struct nv_logger_sink {
    void (*on_log)(void* user, const struct nv_log_event* event);
    void* user;

    uint32_t level;
};

struct nv_logger {
    size_t sink_count;
    const struct nv_logger_sink* sinks;

    uint32_t level;
};

void nv_set_default_logger(const struct nv_logger* logger);
const struct nv_logger* nv_get_default_logger();

/* assumes user is a FILE* */
void nv_log_print_to_file(void* user, const struct nv_log_event* event);

void nv_create_stdout_sink(struct nv_logger_sink* sink);

void nv_log(const struct nv_logger* logger, uint32_t line, const char* file, uint32_t level,
            const char* fmt, ...);

#define NV_LOGGER_LOG(logger, level, ...) nv_log(logger, __LINE__, __FILE__, level, __VA_ARGS__)
#define NV_LOG(level, ...) nv_log(NULL, __LINE__, __FILE__, level, __VA_ARGS__)

#define NV_LOGGER_TRACE(logger, ...) NV_LOGGER_LOG(logger, NV_LOG_LEVEL_TRACE, __VA_ARGS__)
#define NV_LOGGER_DEBUG(logger, ...) NV_LOGGER_LOG(logger, NV_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define NV_LOGGER_INFO(logger, ...) NV_LOGGER_LOG(logger, NV_LOG_LEVEL_INFO, __VA_ARGS__)
#define NV_LOGGER_WARN(logger, ...) NV_LOGGER_LOG(logger, NV_LOG_LEVEL_WARN, __VA_ARGS__)
#define NV_LOGGER_ERROR(logger, ...) NV_LOGGER_LOG(logger, NV_LOG_LEVEL_ERROR, __VA_ARGS__)

#define NV_LOG_TRACE(...) NV_LOG(NV_LOG_LEVEL_TRACE, __VA_ARGS__)
#define NV_LOG_DEBUG(...) NV_LOG(NV_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define NV_LOG_INFO(...) NV_LOG(NV_LOG_LEVEL_INFO, __VA_ARGS__)
#define NV_LOG_WARN(...) NV_LOG(NV_LOG_LEVEL_WARN, __VA_ARGS__)
#define NV_LOG_ERROR(...) NV_LOG(NV_LOG_LEVEL_ERROR, __VA_ARGS__)

#endif
