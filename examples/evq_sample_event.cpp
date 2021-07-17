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
#include <evq/evq_event.h>

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
std::atomic<bool> g_th1_done = false;
std::atomic<bool> g_th2_run  = true;
std::atomic<bool> g_th3_run  = true;
std::atomic<bool> g_core_run = true;

// Event test config
typedef enum
{
    SAMPLE_EVENT_ID_X_00,
    SAMPLE_EVENT_ID_X_01,
    SAMPLE_EVENT_ID_X_1F = 0x1F,
    SAMPLE_EVENT_ID_X_20,
    SAMPLE_EVENT_ID_X_FF = 0xFF,
} eventCodes;

constexpr uint32_t                             g_th1_postInterval  = 2;
const std::vector<std::pair<evq_id_t, size_t>> g_th1_eventPostList = {
    {SAMPLE_EVENT_ID_X_00, 8},
    {SAMPLE_EVENT_ID_X_01, 8},
    {SAMPLE_EVENT_ID_X_1F, 8},
    {SAMPLE_EVENT_ID_X_20, 8},
    {SAMPLE_EVENT_ID_X_FF, 8},
};

const std::vector<evq_id_t> g_th2_eventSubList = {
    SAMPLE_EVENT_ID_X_00,
    SAMPLE_EVENT_ID_X_01,
    SAMPLE_EVENT_ID_X_1F,
};

int error(const std::string &msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

bool eventHandler_1(evq_id_t srcId, evq_id_t evtId)
{
    EVQ_LOG_INFO("Event Handler Called for thread 2: srcId: %4x, evtId: %4x\n", srcId, evtId);
    return true;
}

// Thread 1 -- acts as producer
auto threadFunction_1 = []() {
    evq_status_t        st           = EVQ_ERROR_NONE;
    evq_handle_t        handle       = NULL;
    evq_handle_config_t handleConfig = {
        .handleName   = "thread_1",
        .handleId     = HANDLE_ID_1,
        .streamSize   = HANDLE_QUEUE_SIZE,
        .eventHandler = eventHandler_1,
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
        evq_post_event(handle, eList.first);
        std::this_thread::sleep_for(std::chrono::milliseconds(g_th1_postInterval));
    }

    // Wait for signal to terminate
    g_th1_done.store(true);
    while (g_th1_run.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EVQ_LOG_INFO("TH1: Thread terminating\n");
    st = evq_handle_unregister(&handle);
    assert(EVQ_ERROR_NONE == st);
};

bool eventHandler_2(evq_id_t srcId, evq_id_t evtId)
{
    EVQ_LOG_INFO("Event Handler Called for thread 2: srcId: %4x, evtId: %4x\n", srcId, evtId);
    return true;
}

// Thread 2 -- acts as loopback
// Any message this thread receive, sends it back
auto threadFunction_2 = []() {
    evq_status_t        st           = EVQ_ERROR_NONE;
    evq_handle_t        handle       = NULL;
    evq_handle_config_t handleConfig = {
        .handleName   = "thread_2",
        .handleId     = HANDLE_ID_2,
        .streamSize   = HANDLE_QUEUE_SIZE,
        .eventHandler = eventHandler_2,
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

    while (g_th1_done.load() != true) std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    g_th1_run.store(false);
    th_hdl_1.join();
    g_th2_run.store(false);
    th_hdl_2.join();
    g_core_run.store(false);
    th_core.join();
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