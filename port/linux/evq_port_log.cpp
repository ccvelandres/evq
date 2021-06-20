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
