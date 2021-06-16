#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <evq/evq_log.h>

#if defined(EVQ_LOGGING)

// Override max level
#ifdef EVQ_LOG_LEVEL_MAX
#undef EVQ_LOG_LEVEL_MAX
#define EVQ_LOG_LEVEL_MAX EVQ_LOG_LEVEL_TRACE
#endif

using ::testing::_;
using ::testing::Return;
using ::testing::ExitedWithCode;
using ::testing::StrictMock;
using ::testing::StrEq;
using ::testing::HasSubstr;

class Mock_EVQ_LOG
{
public:
    static Mock_EVQ_LOG *instance;
    Mock_EVQ_LOG() { Mock_EVQ_LOG::instance = this; }
    ~Mock_EVQ_LOG() { Mock_EVQ_LOG::instance = nullptr; }

    MOCK_METHOD(uint32_t, evq_flush_log, (const char *, uint32_t));
};

Mock_EVQ_LOG *Mock_EVQ_LOG::instance = NULL;

extern "C" uint32_t evq_flush_log(const char *str, uint32_t len)
{
    if (NULL == Mock_EVQ_LOG::instance)
    {
        ADD_FAILURE() << "Mock_EVQ_LOG::instance == NULL";
        return 0;
    }
    return Mock_EVQ_LOG::instance->evq_flush_log(str, len);
}

extern "C" void evq_assert(const char *condstr,
                           const char *file,
                           int         line,
                           const char *str)
{
    // Exit when we enter assertion
    exit(1);
}

class F_evq_log : public ::testing::Test
{
public:
    Mock_EVQ_LOG mock_evq_log;

    F_evq_log() {}
    ~F_evq_log() {}

    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(F_evq_log, log_level)
{
    evq_log_level_t level;

    EXPECT_EQ(EVQ_LOG_LEVEL, evq_log_get_level());
    evq_log_set_level(EVQ_LOG_LEVEL_DEBUG);
    EXPECT_EQ(EVQ_LOG_LEVEL_DEBUG, evq_log_get_level());

    // LEVEL - NONE
    evq_log_set_level(EVQ_LOG_LEVEL_NONE);
    EXPECT_CALL(mock_evq_log, evq_flush_log(_, _)).Times(0);
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_ASSERTION, "this should not print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_ERROR, "this should not print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_WARNING, "this should not print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_INFO, "this should not print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_DEBUG, "this should not print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_TRACE, "this should not print"));

    // LEVEL - INFO
    evq_log_set_level(EVQ_LOG_LEVEL_INFO);
    EXPECT_CALL(mock_evq_log, evq_flush_log(_, _)).Times(4);
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_ASSERTION, "this should print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_ERROR, "this should print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_WARNING, "this should print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_INFO, "this should print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_DEBUG, "this should not print"));
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_TRACE, "this should not print"));
}

TEST_F(F_evq_log, format)
{
    evq_log_set_level(EVQ_LOG_LEVEL_TRACE);

    // Format - String
    EXPECT_CALL(mock_evq_log, evq_flush_log(StrEq("test"), strlen("test") + 1)).Times(1);
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_ERROR, "%s", "test"));
    // Format - Integer
    EXPECT_CALL(mock_evq_log, evq_flush_log(StrEq("5"), strlen("5") + 1)).Times(1);
    EXPECT_NO_THROW(evq_log(EVQ_LOG_LEVEL_ERROR, "%d", 5));

    // Prefix Test
    EXPECT_CALL(mock_evq_log, evq_flush_log(HasSubstr("ERROR:"), _)).Times(1);
    EXPECT_NO_THROW(EVQ_LOG_ERROR("test"));
    EXPECT_CALL(mock_evq_log, evq_flush_log(HasSubstr("WARNING:"), _)).Times(1);
    EXPECT_NO_THROW(EVQ_LOG_WARNING("test"));
    EXPECT_CALL(mock_evq_log, evq_flush_log(HasSubstr("INFO:"), _)).Times(1);
    EXPECT_NO_THROW(EVQ_LOG_INFO("test"));
    EXPECT_CALL(mock_evq_log, evq_flush_log(HasSubstr("DEBUG:"), _)).Times(1);
    EXPECT_NO_THROW(EVQ_LOG_DEBUG("test"));
    EXPECT_CALL(mock_evq_log, evq_flush_log(HasSubstr("TRACE:"), _)).Times(1);
    EXPECT_NO_THROW(EVQ_LOG_TRACE("test"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
