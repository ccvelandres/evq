#include <assert.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <unordered_set>
#include <vector>

#include <evq/evq_core.h>
#include <evq/evq_log.h>
#include <evq/evq_port.h>

// Sample configs
constexpr evq_id_t HANDLE_ID_1        = 0xAAAA;
constexpr evq_id_t HANDLE_ID_2        = 0xBBBB;
constexpr evq_id_t HANDLE_ID_3        = 0xCCCC;
constexpr uint32_t HANDLE_QUEUE_SIZE  = 8;
constexpr uint32_t HANDLE_TH2_TIMEOUT = 5;

// Thread status
static std::atomic<bool> g_th1_ready = false;
static std::atomic<bool> g_th2_ready = false;
static std::atomic<bool> g_th3_ready = false;

// Thread control
std::atomic<bool> g_th1_run  = true;
std::atomic<bool> g_th2_run  = true;
std::atomic<bool> g_th3_run  = true;
std::atomic<bool> g_core_run = true;

// Event test config
typedef enum
{
    SAMPLE_EVENT_ID_1,
    SAMPLE_EVENT_ID_2,
    SAMPLE_EVENT_ID_3 = 0x1F,
    SAMPLE_EVENT_ID_4,
    SAMPLE_EVENT_ID_5 = 0xFF,
} eventCodes;

constexpr uint32_t                             g_th1_postInterval  = 2;
const std::vector<std::pair<evq_id_t, size_t>> g_th1_eventPostList = {
    {SAMPLE_EVENT_ID_1, 8},
    {SAMPLE_EVENT_ID_2, 8},
    {SAMPLE_EVENT_ID_3, 8},
    {SAMPLE_EVENT_ID_4, 8},
    {SAMPLE_EVENT_ID_5, 8},
};

const std::vector<evq_id_t> g_th2_eventSubList = {
    SAMPLE_EVENT_ID_1,
    SAMPLE_EVENT_ID_3,
    SAMPLE_EVENT_ID_5,
};

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
        .queueSize    = HANDLE_QUEUE_SIZE,
        .eventHandler = NULL,
    };

    st = evq_handle_register(&handle, &handleConfig);
    assert(EVQ_ERROR_NONE == st);

    // Let other threads start
    g_th1_ready.store(true);

    // Wait for thread 2 to start
    while (!g_th2_ready.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Post events listed in g_th1_eventPostList
    for (auto &eList : g_th1_eventPostList)
    {
        for (size_t i = 0; i < eList.second; ++i)
        {

            std::this_thread::sleep_for(std::chrono::milliseconds(g_th1_postInterval));
        }

        // Wait 5 milliseconds per eventId
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // Wait for signal to terminate
    while (g_th1_run.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
        .queueSize    = HANDLE_QUEUE_SIZE,
        .eventHandler = NULL,
    };

    st = evq_handle_register(&handle, &handleConfig);
    assert(EVQ_ERROR_NONE == st);

    // Subscribe to events
    evq_subscribe_a(handle, g_th2_eventSubList.data(), g_th2_eventSubList.size());

    // Notify thread 2 is ready
    g_th2_ready.store(true);
    while (true == g_th2_run.load())
    {
        
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

void runEventTest()
{
    // Start threads
    std::thread th_hdl_1 = std::thread(threadFunction_1);
    std::thread th_hdl_2 = std::thread(threadFunction_2);
    std::thread th_core  = std::thread(threadFunction_evq);
}

int main(int argc, const char **argv)
{
    printf("Running sample_1\n");
    EVQ_LOG_INFO("Logging via evq_log\n");

    evq_status_t st = evq_init();
    assert(EVQ_ERROR_NONE == st);

    runEventTest();

    st = evq_shutdown();
    assert(EVQ_ERROR_NONE == st);

    EVQ_LOG_INFO("Exiting sample application\n");
    return 0;
}