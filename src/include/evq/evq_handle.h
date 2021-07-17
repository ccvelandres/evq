// SPDX-License-Identifier: MIT

#ifndef __EVQ_HANDLE_H__
#define __EVQ_HANDLE_H__

/**
 * @file evq/evq_handle.h
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <evq/evq_types.h>
#include <evq/evq_event.h>

    typedef struct
    {
        uint8_t dummy[20];
    } evq_static_handle_t;

    typedef void *evq_handle_t;

    typedef struct
    {
        const char         *handleName;
        evq_id_t            handleId;
        uint32_t            streamSize;
        evq_event_handler_t eventHandler;
    } evq_handle_config_t;

    /** @brief Allocate evq handle */
    evq_status_t evq_handle_register(evq_handle_t *handle, const evq_handle_config_t *config);
    evq_status_t evq_handle_unregister(evq_handle_t *handle);

#ifdef __cplusplus
}
#endif
#endif
