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

static size_t getThreadId()
{
    thread_local size_t _threadNum = threadNum++;
    return _threadNum;
}

extern "C" void evq_flush_log(const char *str, uint32_t len)
{
    std::lock_guard<std::mutex> l(logMutex);
    size_t                      _threadNum = getThreadId();
    std::cout << "[" << _threadNum << "] ";
    std::cout.write(str, len);
    std::cout.flush();
}

void evq_assert(const char *condstr, const char *file, int line, const char *str)
{
    std::lock_guard<std::mutex> l(logMutex);
    size_t                      _threadNum = getThreadId();
    std::cout << "[" << _threadNum << "] ";
    std::cout << "Assertion Called:" << std::endl;
    std::cout << condstr << std::endl;
    std::cout << file << std::endl;
    std::cout << line << std::endl;
    std::cout.flush();
    exit(1);
}