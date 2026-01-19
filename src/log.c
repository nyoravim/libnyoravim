#include "nyoravim/log.h"

#include "nyoravim/mem.h"
#include "nyoravim/util.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include <unistd.h>

static const struct nv_logger* s_logger = NULL;

void nv_set_default_logger(const struct nv_logger* logger) { s_logger = logger; }
const struct nv_logger* nv_get_default_logger() { return s_logger; }

static bool is_file_tty(FILE* file) {
    int fd = fileno(file);
    return isatty(fd);
}

static char* format_timestamp(time_t timestamp) {
    struct tm* local = localtime(&timestamp);

    int32_t year = local->tm_year + 1900;
    int32_t month = local->tm_mon + 1;

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%04d/%02d/%02d %02d:%02d:%02d", year, month, local->tm_mday,
             local->tm_hour, local->tm_min, local->tm_sec);

    return nv_strdup(buffer);
}

static const char* get_level_label(uint32_t level) {
    switch (level) {
    case NV_LOG_LEVEL_TRACE:
        return "trace";
    case NV_LOG_LEVEL_DEBUG:
        return "debug";
    case NV_LOG_LEVEL_INFO:
        return "info";
    case NV_LOG_LEVEL_WARN:
        return "warn";
    case NV_LOG_LEVEL_ERROR:
        return "error";
    default:
        return "?";
    }
}

static void print_color(FILE* file, const struct nv_log_event* event) {
    char* timestamp = format_timestamp(event->timestamp);
    const char* label = get_level_label(event->level);

    uint8_t level_color;
    switch (event->level) {
    case NV_LOG_LEVEL_TRACE:
        level_color = 0b100; /* blue */
        break;
    case NV_LOG_LEVEL_DEBUG:
        level_color = 0b110; /* cyan */
        break;
    case NV_LOG_LEVEL_INFO:
        level_color = 0b010; /* green */
        break;
    case NV_LOG_LEVEL_WARN:
        level_color = 0b011; /* yellow */
        break;
    case NV_LOG_LEVEL_ERROR:
        level_color = 0b001; /* red */
        break;
    default:
        level_color = 0b111; /* white */
        break;
    }

    fprintf(file, "\x1b[0m%s \x1b[38;5;%hhum%s \x1b[38;5;8m%s:%u \x1b[0m%s\n", timestamp,
            level_color, label, event->file, event->line, event->message);

    nv_free(timestamp);
}

static void print_monochrome(FILE* file, const struct nv_log_event* event) {
    char* timestamp = format_timestamp(event->timestamp);
    const char* label = get_level_label(event->level);

    fprintf(file, "%s %s %s:%u %s\n", timestamp, label, event->file, event->line, event->message);
    nv_free(timestamp);
}

void nv_log_print_to_file(void* user, const struct nv_log_event* event) {
    FILE* file = user; /* assume user is a file */

    if (is_file_tty(file)) {
        print_color(file, event);
    } else {
        print_monochrome(file, event);
    }
}

void nv_create_stdout_sink(struct nv_logger_sink* sink) {
    sink->user = stdout;
    sink->on_log = nv_log_print_to_file;
}

void nv_log(const struct nv_logger* logger, uint32_t line, const char* file, uint32_t level,
            const char* fmt, ...) {
    if (!logger) {
        logger = s_logger;
    }

    assert(logger);
    if (level < logger->level) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    int chars = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (chars <= 0) {
        return;
    }

    size_t buffer_size = chars + 1;
    char* buffer = nv_alloc(buffer_size);
    assert(buffer);

    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);

    struct nv_log_event event;
    event.line = line;
    event.file = file;
    event.message = buffer;
    event.level = level;
    event.timestamp = time(NULL);

    for (size_t i = 0; i < logger->sink_count; i++) {
        const struct nv_logger_sink* sink = &logger->sinks[i];
        if (level < sink->level) {
            continue;
        }

        sink->on_log(sink->user, &event);
    }
}
