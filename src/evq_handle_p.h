// SPDX-License-Identifier: MIT

#ifndef __EVQ_HANDLE_P_H__
#define __EVQ_HANDLE_P_H__

/**
 * @file ecs/evq_handle_p.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <evq/evq_types.h>
#include <evq/evq_port.h>
#include <evq_core_p.h>

/**
 * @brief Perform handle lookup for matching id.
 * @param[in] handleId id to match
 * @return pointer to handle with matching id,
 *          NULL if no matching id
 */
evq_handle_t evq_priv_lookup_handle_with_id(evq_context_t *ctx, evq_id_t handleId);

/**
 * @brief Core processing function for all handle related core messages
 * @param ctx pointer to evq context
 * @param cMsg core message
 * @return EVQ_ERROR_NONE on success
 */
evq_status_t evq_handle_process(evq_context_t *ctx, const evq_core_message_t *cMsg);

#ifdef __cplusplus
}
#endif
#endif
