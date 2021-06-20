#include <assert.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <unordered_set>

#include <evq/evq_core.h>
#include <evq/evq_log.h>
#include <evq/evq_port.h>

constexpr evq_id_t HANDLE_ID_1        = 0xAAAA;
constexpr evq_id_t HANDLE_ID_2        = 0xBBBB;
constexpr evq_id_t HANDLE_ID_3        = 0xCCCC;
constexpr evq_id_t HANDLE_MSG_ID      = 0xBEEF;
constexpr uint32_t HANDLE_QUEUE_SIZE  = 8;
constexpr uint32_t HANDLE_TH2_TIMEOUT = 5;

// Direct message test config
constexpr evq_id_t DIRECT_TARGET_HANDLE = HANDLE_ID_2;
constexpr uint32_t DIRECT_MSG_COUNT     = 128;
constexpr uint32_t DIRECT_MSG_INTERVAL  = 3;
constexpr uint32_t DIRECT_MSG_WAIT_TIME = 10;

// Thread status
static std::atomic<bool> g_th1_ready = false;
static std::atomic<bool> g_th2_ready = false;
static std::atomic<bool> g_th3_ready = false;

// Direct test results
std::atomic<bool>            g_th1_directTestDone  = false;
uint32_t                     g_th1_directSentOk    = 0;
uint32_t                     g_th1_directSentError = 0;
uint32_t                     g_th2_receiveCount    = 0;
std::unordered_set<evq_id_t> g_th2_receiveIds;

// Loopback test config
constexpr evq_id_t LOOKUP_TARGET_HANDLE = HANDLE_ID_3;
constexpr uint32_t LOOPBACK_COUNT       = 128;
constexpr uint32_t LOOPBACK_WAIT_TIME   = 10;

// Loopback test results
std::atomic<bool>            g_th1_loopbackTestStart    = false;
std::atomic<bool>            g_th1_loopbackTestDone     = false;
uint32_t                     g_th1_loopbackSentOk       = 0;
uint32_t                     g_th1_loopbackSentError    = 0;
uint32_t                     g_th2_loopbackReceiveCount = 0;
std::unordered_set<evq_id_t> g_th2_loopbackReceiveIds;

std::atomic<bool> g_th2_run  = true;
std::atomic<bool> g_core_run = true;

int error(const std::string &msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

// Thread 1 -- acts as producer
auto threadFunction_1 = []() {
    evq_status_t        st           = EVQ_ERROR_NONE;
    evq_handle_t        handle       = NULL;
    evq_handle_config_t handleConfig = {
        .handleName   = "thread_1",
        .handleId     = HANDLE_ID_1,
        .streamSize    = HANDLE_QUEUE_SIZE,
        .eventHandler = NULL,
    };

    st = evq_handle_register(&handle, &handleConfig);
    assert(EVQ_ERROR_NONE == st);

    // Let other threads start
    g_th1_ready.store(true);
    while (!g_th2_ready.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // One-way message flood -- 10ms max wait
    for (size_t i = 0; i < DIRECT_MSG_COUNT; ++i)
    {
        // build message
        evq_message_t msg = NULL;
        evq_message_allocate(&msg, 0);

        st = evq_send(handle, DIRECT_TARGET_HANDLE, i, msg);
        if (EVQ_ERROR_NONE == st)
        {
            g_th1_directSentOk++;
        }
        else
        {
            EVQ_LOG_ERROR("DIRECT: Error(0x%X) sending %d\n", st, i);
            g_th1_directSentError++;
        }

        // interval wait
        std::this_thread::sleep_for(std::chrono::milliseconds(DIRECT_MSG_INTERVAL));
    }
    // Signal Direct Test is done
    g_th1_directTestDone.store(true);

    while (!g_th1_loopbackTestStart.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EVQ_LOG_INFO("TH1: Thread terminating\n");
    st = evq_handle_unregister(&handle);
    assert(EVQ_ERROR_NONE == st);
};

// Thread 2 -- acts as loopback
// Any message this thread receive, sends it back
auto threadFunction_2 = []() {
    evq_status_t        st           = EVQ_ERROR_NONE;
    evq_handle_t        handle       = NULL;
    evq_handle_config_t handleConfig = {
        .handleName   = "thread_2",
        .handleId     = HANDLE_ID_2,
        .streamSize    = HANDLE_QUEUE_SIZE,
        .eventHandler = NULL,
    };

    st = evq_handle_register(&handle, &handleConfig);
    assert(EVQ_ERROR_NONE == st);

    g_th2_ready.store(true);
    while (true == g_th2_run.load())
    {
        evq_message_t msg = NULL;

        st = evq_receive(handle, &msg, HANDLE_TH2_TIMEOUT);
        if (EVQ_ERROR_NONE == st)
        {
            g_th2_receiveCount++;
            g_th2_receiveIds.insert(msg->msgId);

            // destroy message after use
            evq_message_destroy(msg);
        }
        else
        {
            // we didnt receive message, either
            // check return for cause
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EVQ_LOG_INFO("TH2: Thread terminating\n");
    st = evq_handle_unregister(&handle);
    assert(EVQ_ERROR_NONE == st);
};

auto threadFunction_evq = []() {
    while (true == g_core_run.load())
    {
        evq_process();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
};

void runMultiThread()
{
    // Start threads
    std::thread th_hdl_1 = std::thread(threadFunction_1);
    std::thread th_hdl_2 = std::thread(threadFunction_2);
    std::thread th_core  = std::thread(threadFunction_evq);

    // Wait till direct test is done
    while (!g_th1_directTestDone.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // Wait to receive all
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    g_th2_run.store(false);

    EVQ_LOG_INFO("Direct test done\n");
    EVQ_LOG_INFO("    Message Count: %d\n", DIRECT_MSG_COUNT);
    EVQ_LOG_INFO("       Sent OK:    %d\n", g_th1_directSentOk);
    EVQ_LOG_INFO("       Sent ERROR: %d\n", g_th1_directSentError);
    EVQ_LOG_INFO("       Recv OK:    %d\n", g_th2_receiveCount);
    EVQ_LOG_INFO("       Recv ERROR: %d\n", DIRECT_MSG_COUNT - g_th2_receiveCount);
    if (g_th2_receiveCount != DIRECT_MSG_COUNT)
    {
        EVQ_LOG_INFO("       Missing IDs:\n", g_th2_receiveCount);
        std::string idString;
        for (size_t i = 0; i < DIRECT_MSG_COUNT; ++i)
        {
            if (g_th2_receiveIds.find(i) == g_th2_receiveIds.end())
            {
                idString += std::to_string(i) + ", ";
            }
        }
        EVQ_LOG_INFO("           %s\n", idString.c_str());
    }

    // terminate thread 2
    g_th2_run.store(false);
    th_hdl_2.join();

    // start loopback test
    g_th1_loopbackTestStart.store(true);

    // terminate thread 3

    // terminate thread 1
    th_hdl_1.join();

    // terminate core thread
    g_core_run.store(false);
    th_core.join();
}

void runSinglethread()
{
    evq_handle_t        hdl[2]       = {};
    evq_message_t       msg[3]       = {};
    evq_handle_config_t hdlConfig[2] = {
        {.handleId = HANDLE_ID_1, .streamSize = HANDLE_QUEUE_SIZE},
        {.handleId = HANDLE_ID_2, .streamSize = HANDLE_QUEUE_SIZE}
    };

    auto runProcess = [](size_t cycles) {
        for (size_t i = 0; i < cycles; ++i) evq_process();
    };

    // register handle
    assert(EVQ_ERROR_NONE == evq_handle_register(&hdl[0], &hdlConfig[0]));
    assert(EVQ_ERROR_NONE == evq_handle_register(&hdl[1], &hdlConfig[1]));
    runProcess(16); // Let process run

    // Allocate and send from hdl_1 to hdl_2
    evq_message_allocate(&msg[0], 0);
    assert(EVQ_ERROR_NONE == evq_send(hdl[0], HANDLE_ID_2, HANDLE_MSG_ID, msg[0]));
    runProcess(16); // Let process run

    // Receive the mssage from hdl_1
    assert(EVQ_ERROR_NONE == evq_receive(hdl[1], &msg[1], 0));
    runProcess(16); // Let process run

    // Resend the message back to hdl_1
    assert(EVQ_ERROR_NONE == evq_send(hdl[1], msg[1]->srcId, msg[1]->msgId, msg[1]));
    runProcess(16); // Let process run

    // Receive the mssage from hdl_2
    assert(EVQ_ERROR_NONE == evq_receive(hdl[0], &msg[2], 0));
    runProcess(16); // Let process run

    // Message should match
    assert(msg[0] == msg[2]);
    assert(msg[1] != NULL);

    // unregister handle
    assert(EVQ_ERROR_NONE == evq_handle_unregister(&hdl[0]));
    assert(EVQ_ERROR_NONE == evq_handle_unregister(&hdl[1]));
}

int main(int argc, const char **argv)
{
    printf("Running sample_1\n");
    EVQ_LOG_INFO("Logging via evq_log\n");

    evq_status_t st = evq_init();
    assert(EVQ_ERROR_NONE == st);

    runMultiThread();
    // runSinglethread();

    st = evq_shutdown();
    assert(EVQ_ERROR_NONE == st);

    EVQ_LOG_INFO("Exiting sample application\n");
    return 0;
}