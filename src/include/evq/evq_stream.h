// SPDX-License-Identifier: MIT

#ifndef __EVQ_STREAM_P_H__
#define __EVQ_STREAM_P_H__

/**
 * @file evq/evq_stream.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <evq/evq_types.h>

    typedef struct
    {
        volatile uint32_t head;   /** position to write */
        volatile uint32_t tail;   /** position to read */
        uint32_t          size;   /** size of element */
        uint32_t          len;    /** size of container */
        uint8_t           data[]; /** container for items */
    } evq_stream_t;

    evq_status_t evq_stream_create(evq_stream_t **stream, uint32_t size, uint32_t len);
    evq_status_t evq_stream_destroy(evq_stream_t *stream);
    evq_status_t evq_stream_push(evq_stream_t *stream, const void *ptr);
    evq_status_t evq_stream_pop(evq_stream_t *stream, void *ptr);
    evq_status_t evq_stream_size(evq_stream_t *stream, uint32_t *size);
    evq_status_t evq_stream_clear(evq_stream_t *stream);

#ifdef __cplusplus
}
#endif
#endif
