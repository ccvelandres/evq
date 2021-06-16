#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocks/mock_evq_port.hpp"

#include <evq/evq_stream.h>

using ::testing::_;
using ::testing::Return;
using ::testing::InvokeWithoutArgs;
using ::testing::ExitedWithCode;
using ::testing::StrictMock;
using ::testing::StrEq;
using ::testing::HasSubstr;

// Payload struct for testing and matcher overload
struct payloadStruct
{
    int a = 0;
    int b = 0;
};

namespace testing::internal
{
    bool operator==(const ::payloadStruct &lhs, const ::payloadStruct &rhs)
    {
        return (lhs.a == rhs.a) && (lhs.b == rhs.b);
    }
} // namespace testing::internal

class F_evq_stream : public ::testing::Test
{
public:
    StrictMock<Mock_EVQ_PORT> mock_evq_port;
    F_evq_stream() {}
    ~F_evq_stream() {}

    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(F_evq_stream, create)
{
    constexpr uint32_t itemSize        = sizeof(void *);
    constexpr uint32_t len             = 8;
    constexpr uint32_t dummyBufferSize = sizeof(evq_stream_t) + (itemSize * len);

    evq_status_t         st          = EVQ_ERROR_NONE;
    evq_stream_t        *stream      = nullptr;
    std::vector<uint8_t> dummyBuffer = std::vector<uint8_t>(dummyBufferSize);

    // Success
    EXPECT_CALL(mock_evq_port, evq_malloc(_))
        .Times(1)
        .WillOnce(Return(static_cast<void *>(dummyBuffer.data())));
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_create(&stream, itemSize, len));
    EXPECT_EQ(stream, static_cast<void *>(dummyBuffer.data()));
    EXPECT_EQ(stream->head, 0);
    EXPECT_EQ(stream->tail, 0);

    // Nullarg
    stream = nullptr;
    EXPECT_CALL(mock_evq_port, evq_malloc(sizeof(evq_stream_t) + (sizeof(void *) * len)))
        .Times(1)
        .WillOnce(Return(nullptr));
    EXPECT_EQ(EVQ_ERROR_NMEM, evq_stream_create(&stream, itemSize, len));
    EXPECT_EQ(stream, nullptr);
}

TEST_F(F_evq_stream, destroy)
{
    evq_stream_t  stream    = {};
    evq_stream_t *streamPtr = &stream;

    // Success
    EXPECT_CALL(mock_evq_port, evq_free(streamPtr)).Times(1).WillOnce(Return());
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_destroy(streamPtr));

    // nullarg
    ASSERT_DEATH(evq_stream_destroy(NULL), _);
}

TEST_F(F_evq_stream, size)
{
    constexpr uint32_t itemSize        = sizeof(void *);
    constexpr uint32_t len             = 8;
    constexpr uint32_t dummyBufferSize = sizeof(evq_stream_t) + (itemSize * len);

    evq_status_t         st     = EVQ_ERROR_NONE;
    evq_stream_t        *stream = nullptr;
    std::vector<uint8_t> dummyBuffer(dummyBufferSize);

    uint32_t actualStreamSize   = 0;
    uint32_t expectedStreamSize = 0;

    EXPECT_CALL(mock_evq_port, evq_malloc(_))
        .Times(1)
        .WillOnce(Return(static_cast<void *>(dummyBuffer.data())));
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_create(&stream, itemSize, len));

    // Empty
    actualStreamSize   = 1;
    expectedStreamSize = 0;
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_size(stream, &actualStreamSize));
    EXPECT_EQ(expectedStreamSize, actualStreamSize);

    // Leading Head
    stream->head       = 1;
    stream->tail       = 0;
    expectedStreamSize = 1;
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_size(stream, &actualStreamSize));
    EXPECT_EQ(expectedStreamSize, actualStreamSize);

    // Prewrap
    stream->head       = 7;
    stream->tail       = 0;
    expectedStreamSize = 7;
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_size(stream, &actualStreamSize));
    EXPECT_EQ(expectedStreamSize, actualStreamSize);

    // Wrap around
    stream->head       = 0;
    stream->tail       = 1;
    expectedStreamSize = stream->head + (itemSize - stream->tail);
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_size(stream, &actualStreamSize));
    EXPECT_EQ(expectedStreamSize, actualStreamSize);

    // Equal index
    stream->head       = 4;
    stream->tail       = 4;
    expectedStreamSize = 0;
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_size(stream, &actualStreamSize));
    EXPECT_EQ(expectedStreamSize, actualStreamSize);
}

TEST_F(F_evq_stream, put_pop_ptr)
{
    constexpr uint32_t itemSize        = sizeof(uint8_t *);
    constexpr uint32_t len             = 8;
    constexpr uint32_t dummyBufferSize = sizeof(evq_stream_t) + (itemSize * len);

    evq_status_t         st          = EVQ_ERROR_NONE;
    evq_stream_t        *stream      = nullptr;
    std::vector<uint8_t> dummyBuffer = std::vector<uint8_t>(dummyBufferSize);

    const std::vector<uint8_t> dummyPayload = {0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    const uint8_t *pushPayload = 0;
    uint8_t       *popPayload  = NULL;

    EXPECT_CALL(mock_evq_port, evq_malloc(_))
        .Times(1)
        .WillOnce(Return(static_cast<void *>(dummyBuffer.data())));
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_create(&stream, itemSize, len));

    // Push until full full
    for (size_t i = 0; i < len - 1; ++i)
    {
        pushPayload = &dummyPayload[i];
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_push(stream, &pushPayload));
    }
    // Expect Full
    EXPECT_EQ(EVQ_ERROR_STREAM_FULL, evq_stream_push(stream, &dummyPayload[len + 1]));

    // Pop data and compare until empty
    for (size_t i = 0; i < len - 1; ++i)
    {
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_pop(stream, &popPayload));
        EXPECT_EQ(*popPayload, dummyPayload[i]);
    }
    // Expect empty
    EXPECT_EQ(EVQ_ERROR_STREAM_EMPTY, evq_stream_pop(stream, &popPayload));

    // Alternate push pop -- starts in 7,7
    for (size_t i = 0; i < len - 1; ++i)
    {
        pushPayload = &dummyPayload[i];
        popPayload  = NULL;
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_push(stream, &pushPayload));
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_pop(stream, &popPayload));
        EXPECT_EQ(pushPayload, popPayload);
    }
}

TEST_F(F_evq_stream, put_pop_sruct)
{
    constexpr uint32_t itemSize        = sizeof(payloadStruct);
    constexpr uint32_t len             = 8;
    constexpr uint32_t dummyBufferSize = sizeof(evq_stream_t) + (itemSize * len);

    evq_status_t         st          = EVQ_ERROR_NONE;
    evq_stream_t        *stream      = nullptr;
    std::vector<uint8_t> dummyBuffer = std::vector<uint8_t>(dummyBufferSize);

    payloadStruct                    pushPayload;
    payloadStruct                    popPayload;
    const std::vector<payloadStruct> dummyPayload = {
        {0x11, 0x12},
        {0x21, 0x22},
        {0x31, 0x32},
        {0x41, 0x42},
        {0x51, 0x52},
        {0x61, 0x62},
        {0x71, 0x72},
        {0x81, 0x82},
    };

    EXPECT_CALL(mock_evq_port, evq_malloc(_))
        .Times(1)
        .WillOnce(Return(static_cast<void *>(dummyBuffer.data())));
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_create(&stream, itemSize, len));

    // Push until full full
    for (size_t i = 0; i < len - 1; ++i)
    {
        pushPayload = dummyPayload[i];
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_push(stream, &pushPayload));
    }
    // Expect Full
    EXPECT_EQ(EVQ_ERROR_STREAM_FULL, evq_stream_push(stream, &dummyPayload[len + 1]));

    // Pop data and compare until empty
    for (size_t i = 0; i < len - 1; ++i)
    {
        popPayload = payloadStruct();
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_pop(stream, &popPayload));
        EXPECT_EQ(popPayload, dummyPayload[i]);
    }
    // Expect empty
    EXPECT_EQ(EVQ_ERROR_STREAM_EMPTY, evq_stream_pop(stream, &popPayload));

    // Alternate push pop -- starts in 7,7
    for (size_t i = 0; i < len - 1; ++i)
    {
        pushPayload = dummyPayload[i];
        popPayload  = payloadStruct();
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_push(stream, &pushPayload));
        EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_pop(stream, &popPayload));
        EXPECT_EQ(pushPayload, popPayload);
    }
}

TEST_F(F_evq_stream, clear)
{
    constexpr uint32_t itemSize        = sizeof(uint8_t *);
    constexpr uint32_t len             = 8;
    constexpr uint32_t dummyBufferSize = sizeof(evq_stream_t) + (itemSize * len);

    evq_status_t         st          = EVQ_ERROR_NONE;
    evq_stream_t        *stream      = nullptr;
    std::vector<uint8_t> dummyBuffer = std::vector<uint8_t>(dummyBufferSize);

    const std::vector<uint8_t> dummyPayload  = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    const uint8_t             *actualPayload = 0;

    EXPECT_CALL(mock_evq_port, evq_malloc(_))
        .Times(1)
        .WillOnce(Return(static_cast<void *>(dummyBuffer.data())));
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_create(&stream, itemSize, len));

    // force random index
    stream->head = 4;
    stream->tail = 2;
    EXPECT_EQ(EVQ_ERROR_NONE, evq_stream_clear(stream));
    EXPECT_EQ(stream->head, stream->tail);
}
