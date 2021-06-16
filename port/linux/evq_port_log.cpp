#include <iostream>
#include <thread>
#include <mutex>
#include <stdarg.h>

#include <evq/evq_log.h>

////////////////////////////////////////////////////////////////////
// Logging wrappers
////////////////////////////////////////////////////////////////////

static std::mutex logMutex;
static size_t     threadNum = 1;

extern "C" void evq_flush_log(const char *str, uint32_t len)
{
    std::lock_guard<std::mutex> l(logMutex);
    thread_local size_t         _threadNum = threadNum++;
    std::cout << "[" << _threadNum << "] ";
    std::cout.write(str, len);
    std::cout.flush();
}

extern "C" void evq_log(evq_log_level_t level, const char *fmt, ...)
{
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
    buf = new char[len];
    vsnprintf(buf, len, fmt, args);
    va_end(args);

    // Flush log
    evq_flush_log(buf, len);

    delete[] buf;
}