#include <chrono>

#include <evq/evq_port.h>

////////////////////////////////////////////////////////////////////
// Timekeeping wrappers
////////////////////////////////////////////////////////////////////

extern "C" uint32_t evq_get_time()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}