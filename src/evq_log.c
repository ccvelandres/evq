#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "pthread.h"

#include <evq/evq_log.h>
#include <evq/evq_port.h>

#if defined(EVQ_LOGGING)

static evq_log_level_t evq_log_level = EVQ_LOG_LEVEL;

evq_log_level_t evq_log_get_level() { return evq_log_level; }
void            evq_log_set_level(const evq_log_level_t level) { evq_log_level = level; }

void __attribute__((weak)) evq_log(evq_log_level_t level, const char *fmt, ...)
{
    // override this function

    va_list  args;
    char    *buf = NULL;
    uint32_t len = 0;
    // Compute needed buffer length
    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    // Write string to buffer
    va_start(args, fmt);
    len += 1;
    buf = evq_malloc(len);
    vsnprintf(buf, len, fmt, args);
    va_end(args);

    // Flush log
    evq_flush_log(buf, len);

    evq_free(buf);
}

void __attribute__((weak))
evq_assert(const char *condstr, const char *file, int line, const char *str)
{
    // override this function
    while(1);
}

#endif
