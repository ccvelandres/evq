// SPDX-License-Identifier: MIT

#ifndef __EVQ_TYPES_H__
#define __EVQ_TYPES_H__

/**
 * @file ecs/evq_types.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define EVQ_TIMEOUT_MAX 0xFFFFFFFF

    typedef enum
    {
        EVQ_ERROR_NONE,         // No error
        EVQ_ERROR_EVENT,        // Incoming event
        EVQ_ERROR,              // General error
        EVQ_ERROR_UNSUPPORTED,  // Unsupported error
        EVQ_ERROR_LIST_FULL,    // List error
        EVQ_ERROR_HANDLE,       // Handle error
        EVQ_ERROR_IARG,         // Invalid argument
        EVQ_ERROR_NARG,         // Null argument
        EVQ_ERROR_TIMEOUT,      // Timeout error
        EVQ_ERROR_NMEM,         // Not enough memory
        EVQ_ERROR_MUTEX,        // Mutex error
        EVQ_ERROR_EGROUP,       // Event group error
        EVQ_ERROR_QUEUE,        // Queue error
        EVQ_ERROR_QUEUE_FULL,   // Queue full error
        EVQ_ERROR_QUEUE_EMPTY,  // Queue empty error
        EVQ_ERROR_STREAM,       // Stream error
        EVQ_ERROR_STREAM_FULL,  // Stream full error
        EVQ_ERROR_STREAM_EMPTY, // Stream empty error
    } evq_status_t;

    typedef uint16_t evq_id_t;

    typedef struct
    {
        evq_id_t srcId;
    } evq_config_t;

    typedef void *evq_handle_t;

#ifdef __cplusplus
}
#endif
#endif
