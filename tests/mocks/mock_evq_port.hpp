#ifndef __MOCK_EVQ_PORT_HPP__
#define __MOCK_EVQ_PORT_HPP__

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <evq/evq_port.h>
#include <evq/evq_log.h>

using ::testing::Return;

class Mock_EVQ_PORT
{
public:
    static Mock_EVQ_PORT *instance;

    Mock_EVQ_PORT() { Mock_EVQ_PORT::instance = this; }

    ~Mock_EVQ_PORT() { Mock_EVQ_PORT::instance = NULL; }

    MOCK_METHOD(void *, evq_malloc, (uint32_t));
    MOCK_METHOD(void *, evq_calloc, (uint32_t, uint32_t));
    MOCK_METHOD(void *, evq_realloc, (void *, uint32_t));
    MOCK_METHOD(void, evq_free, (void *));
};

#endif
