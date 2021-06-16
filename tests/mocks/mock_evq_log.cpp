#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <evq/evq_log.h>

extern "C" void evq_flush_log(evq_log_level_t level, const char *fmt, ...)
{
    // do nothing for logs
}

extern "C" void evq_assert(const char *condstr, const char *file, int line, const char *str)
{
    // Exit when we enter assertion
    exit(1);
}
