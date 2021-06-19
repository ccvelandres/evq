// SPDX-License-Identifier: MIT

#ifndef __EVQ_PORT_H__
#define __EVQ_PORT_H__

/**
 * @file evq/evq_port.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#include <evq/evq_config.h>
#include <evq/evq_types.h>

    /**
     * @brief Memory allocation function wrappers
     * @{
     */
    void *evq_malloc(uint32_t size);
    void *evq_calloc(uint32_t nmemb, uint32_t size);
    void *evq_realloc(void *ptr, uint32_t size);
    void  evq_free(void *ptr);
    /** @}*/

#if defined(EVQ_RTOS_SUPPORT)
    /**
     * @brief Syncronization wrappers
     * @{
     */
    typedef void *evq_mutex_t;
    evq_status_t  evq_mutex_create(evq_mutex_t *mutex);
    evq_status_t  evq_mutex_destroy(evq_mutex_t mutex);
    evq_status_t  evq_mutex_lock(evq_mutex_t mutex, uint32_t timeout);
    evq_status_t  evq_mutex_lock_isr(evq_mutex_t mutex, uint32_t timeout);
    evq_status_t  evq_mutex_unlock(evq_mutex_t mutex);
    evq_status_t  evq_mutex_unlock_isr(evq_mutex_t mutex);

    typedef void *evq_egroup_t;
    evq_status_t  evq_egroup_create(evq_egroup_t *egroup);
    evq_status_t  evq_egroup_destroy(evq_egroup_t egroup);
    evq_status_t  evq_egroup_set(evq_egroup_t egroup, uint32_t flags, uint32_t timeout);
    evq_status_t  evq_egroup_set_isr(evq_egroup_t egroup, uint32_t flags, uint32_t timeout);
    evq_status_t  evq_egroup_wait(evq_egroup_t egroup,
                                  uint32_t     flags,
                                  uint32_t    *matchFlag,
                                  bool         waitForAll,
                                  uint32_t     timeout);
    /** @} */
#endif // EVQ_RTOS_SUPPORT

       /**
        * @brief Queue wrappers
        */

    // typedef void *evq_queue_t;
    // evq_status_t evq_queue_create(evq_queue_t *queue, uint16_t entrySize, uint16_t queueSize);
    // evq_status_t evq_queue_destroy(evq_queue_t queue);
    // evq_status_t evq_queue_send(evq_queue_t queue, const void* entry, uint32_t timeout);
    // evq_status_t evq_queue_send_isr(evq_queue_t queue, const void* entry, uint32_t timeout);
    // evq_status_t evq_queue_receive(evq_queue_t queue, void* entry, uint32_t timeout);
    // evq_status_t evq_queue_receive_isr(evq_queue_t queue, void* entry, uint32_t timeout);

    /**
     * @brief Timekeeping wrappers
     * @{
     */
    uint32_t evq_get_time();
    /** @}*/

    /**
     * @brief Logging wrappers
     * @{
     */
    void evq_flush_log(const char *str, uint32_t len);
    /** @}*/

#ifdef __cplusplus
}
#endif
#endif
