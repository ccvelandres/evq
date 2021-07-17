// SPDX-License-Identifier: MIT

#ifndef __EVQ_EVENT_P_H__
#define __EVQ_EVENT_P_H__

/**
 * @file evq/evq_event.h
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <evq/evq_core.h>
#include <evq/evq_core_p.h>
#include <evq/evq_types.h>

/**
 * @brief Core processing function for all event related core messages
 * @param ctx pointer to evq context
 * @param cMsg core message
 * @return EVQ_ERROR_NONE on success
 */
evq_status_t evq_event_process(evq_context_t *ctx, const evq_core_message_t *cMsg);

#ifdef __cplusplus
}
#endif
#endif
