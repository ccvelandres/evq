#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocks/mock_evq_port.hpp"

#include <evq/evq_core_p.h>
#include <evq/evq_message.h>

using ::testing::_;
using ::testing::Return;
using ::testing::InvokeWithoutArgs;
using ::testing::ExitedWithCode;
using ::testing::StrictMock;

class F_evq_message : public ::testing::Test
{
public:
    StrictMock<Mock_EVQ_PORT> mock_evq_port;

    F_evq_message() {}
    ~F_evq_message() {}

    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(F_evq_message, allocate)
{
    evq_message_t              msg         = NULL;
    uint32_t                   len         = 0;
    std::unique_ptr<uint8_t *> blockBuffer = std::make_unique<uint8_t *>(new uint8_t[64]);

    // Success test
    msg = NULL;
    len = 16;
    EXPECT_CALL(mock_evq_port, evq_malloc(_))
        .Times(1)
        .WillOnce(Return(static_cast<void *>(blockBuffer.get())));
    EXPECT_EQ(EVQ_ERROR_NONE, evq_message_allocate(&msg, len));
    EXPECT_EQ(msg, static_cast<void *>(blockBuffer.get()));

    // Nmem test
    msg = NULL;
    len = 16;
    EXPECT_CALL(mock_evq_port, evq_malloc(_)).Times(1).WillOnce(Return(static_cast<void *>(NULL)));
    EXPECT_EQ(EVQ_ERROR_NMEM, evq_message_allocate(&msg, len));
    EXPECT_EQ(msg, static_cast<void *>(NULL));

    // Null arg
    EXPECT_EQ(EVQ_ERROR_NARG, evq_message_allocate(NULL, len));
}

TEST_F(F_evq_message, destroy)
{
    evq_message_priv_t  priv;
    evq_message_priv_t *privPtr = &priv;
    evq_message_t       msg     = privPtr;

    // Success
    EXPECT_CALL(mock_evq_port, evq_free(privPtr)).Times(1).WillOnce(Return());
    EXPECT_NO_THROW(evq_message_destroy(privPtr));

    // nullarg
    ASSERT_DEATH(evq_message_destroy(NULL), _);
}

TEST_F(F_evq_message, get_src_id)
{
    evq_message_priv_t  priv;
    evq_id_t            srcId   = 0;
    evq_message_priv_t *privPtr = &priv;
    evq_message_t       msg     = NULL;

    // Success
    memset(&priv, 0, sizeof(evq_message_priv_t));
    privPtr->srcId = 0xF;
    srcId          = 0;
    msg            = static_cast<evq_message_t>(privPtr);
    EXPECT_EQ(EVQ_ERROR_NONE, evq_message_get_src_id(msg, &srcId));
    EXPECT_EQ(srcId, privPtr->srcId);

    // Nullarg
    srcId = 0;
    msg   = static_cast<evq_message_t>(privPtr);
    ASSERT_DEATH(evq_message_get_src_id(NULL, &srcId), _);
    ASSERT_DEATH(evq_message_get_src_id(msg, NULL), _);
    ASSERT_DEATH(evq_message_get_src_id(NULL, NULL), _);
}

TEST_F(F_evq_message, get_dst_id)
{
    evq_message_priv_t  priv;
    evq_id_t            dstId   = 0;
    evq_message_priv_t *privPtr = &priv;
    evq_message_t       msg     = NULL;

    // Success
    memset(&priv, 0, sizeof(evq_message_priv_t));
    privPtr->dstId = 0xF;
    dstId          = 0;
    msg            = static_cast<evq_message_t>(privPtr);
    EXPECT_EQ(EVQ_ERROR_NONE, evq_message_get_dst_id(msg, &dstId));
    EXPECT_EQ(dstId, privPtr->dstId);

    // Nullarg
    dstId = 0;
    msg   = static_cast<evq_message_t>(privPtr);
    ASSERT_DEATH(evq_message_get_dst_id(NULL, &dstId), _);
    ASSERT_DEATH(evq_message_get_dst_id(msg, NULL), _);
    ASSERT_DEATH(evq_message_get_dst_id(NULL, NULL), _);
}

TEST_F(F_evq_message, get_msg_id)
{
    evq_message_priv_t  priv;
    evq_id_t            msgId   = 0;
    evq_message_priv_t *privPtr = &priv;
    evq_message_t       msg     = NULL;

    // Success
    memset(&priv, 0, sizeof(evq_message_priv_t));
    privPtr->msgId = 0xF;
    msgId          = 0;
    msg            = static_cast<evq_message_t>(privPtr);
    EXPECT_EQ(EVQ_ERROR_NONE, evq_message_get_msg_id(msg, &msgId));
    EXPECT_EQ(msgId, privPtr->msgId);

    // Nullarg
    msgId = 0;
    msg   = static_cast<evq_message_t>(privPtr);
    ASSERT_DEATH(evq_message_get_msg_id(NULL, &msgId), _);
    ASSERT_DEATH(evq_message_get_msg_id(msg, NULL), _);
    ASSERT_DEATH(evq_message_get_msg_id(NULL, NULL), _);
}

TEST_F(F_evq_message, get_data)
{
    evq_status_t        st = EVQ_ERROR;
    evq_message_priv_t  priv;
    evq_message_priv_t *privPtr = &priv;
    evq_message_t       msg     = NULL;

    privPtr->len  = 0xE;
    uint8_t *data = 0;
    uint32_t len  = 0;

    // Success
    msg = static_cast<evq_message_t>(privPtr);
    st  = evq_message_pop_data(msg, &data, &len);
    EXPECT_EQ(EVQ_ERROR_NONE, st);
    EXPECT_EQ(data, privPtr->data);
    EXPECT_EQ(len, privPtr->len);

    // Nullarg
    ASSERT_DEATH(evq_message_pop_data(msg, NULL, NULL), _);
    ASSERT_DEATH(evq_message_pop_data(NULL, &data, NULL), _);
    ASSERT_DEATH(evq_message_pop_data(NULL, NULL, &len), _);
    ASSERT_DEATH(evq_message_pop_data(NULL, NULL, NULL), _);
}
