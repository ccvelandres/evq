#include <stddef.h>
#include <string.h>

#include <evq/evq_core.h>
#include <evq/evq_config.h>
#include <evq/evq_port.h>
#include <evq/evq_log.h>

#include <evq/evq_core_p.h>
#include <evq/evq_handle_p.h>

////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////

#if defined(EVQ_RTOS_SUPPORT)
static evq_mutex_t g_handle_mutex;
#endif
static bool              g_initialized = false;
static evq_handle_priv_t g_handles[EVQ_MAX_HANDLES];
static uint16_t          g_handle_count = 0;

////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////

evq_status_t evq_handle_init()
{
    evq_status_t st = EVQ_ERROR_NONE;

    if (g_initialized)
    {
        EVQ_LOG_TRACE("evq_handle already initialized");
        return EVQ_ERROR_NONE;
    }

    g_handle_count = 0;

    st = evq_mutex_create(&g_handle_mutex);
    if (EVQ_ERROR_NONE != st)
    {
        EVQ_LOG_ERROR("Could not create mutex");
        return st;
    }

    return st;
}
