#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <exception>

#include <evq/evq_port.h>
#include <evq/evq_log.h>
#include "mock_evq_port.hpp"

Mock_EVQ_PORT *Mock_EVQ_PORT::instance = NULL;

extern "C" void *evq_malloc(uint32_t size)
{
    if (NULL == Mock_EVQ_PORT::instance)
    {
        ADD_FAILURE() << "Mock_EVQ_PORT::instance == NULL";
        return NULL;
    }
    return Mock_EVQ_PORT::instance->evq_malloc(size);
}

extern "C" void *evq_calloc(uint32_t nmemb, uint32_t size)
{
    if (NULL == Mock_EVQ_PORT::instance)
    {
        ADD_FAILURE() << "Mock_EVQ_PORT::instance == NULL";
        return NULL;
    }
    return Mock_EVQ_PORT::instance->evq_calloc(nmemb, size);
}

extern "C" void *evq_realloc(void *ptr, uint32_t size)
{
    if (NULL == Mock_EVQ_PORT::instance)
    {
        ADD_FAILURE() << "Mock_EVQ_PORT::instance == NULL";
        return NULL;
    }
    return Mock_EVQ_PORT::instance->evq_realloc(ptr, size);
}

extern "C" void evq_free(void *ptr)
{
    if (NULL == Mock_EVQ_PORT::instance)
    {
        ADD_FAILURE() << "Mock_EVQ_PORT::instance == NULL";
        return;
    }
    Mock_EVQ_PORT::instance->evq_free(ptr);
}

extern "C" uint32_t evq_flush_log(const char *str, uint32_t len)
{
    // do nothing for logs
    return 0;
}

extern "C" void evq_assert(const char *condstr, const char *file, int line, const char *str)
{
    // exit on assert
    exit(1);
}
