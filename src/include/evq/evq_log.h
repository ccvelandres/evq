// SPDX-License-Identifier: MIT

#ifndef __EVQ_LOG_H__
#define __EVQ_LOG_H__

/**
 * @file evq/evq_log.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <evq/evq_types.h>
#include <evq/evq_config.h>

    typedef uint8_t evq_log_level_t;
    evq_log_level_t evq_log_get_level();
    void            evq_log_set_level(const evq_log_level_t level);
    void            evq_log(evq_log_level_t level, const char *fmt, ...);
    void            evq_assert(const char *condstr, const char *file, int line, const char *str);

#define EVQ_LOG_LEVEL_NONE      0
#define EVQ_LOG_LEVEL_ASSERTION 1
#define EVQ_LOG_LEVEL_ERROR     2
#define EVQ_LOG_LEVEL_WARNING   3
#define EVQ_LOG_LEVEL_INFO      4
#define EVQ_LOG_LEVEL_DEBUG     5
#define EVQ_LOG_LEVEL_TRACE     6

#if defined(EVQ_LOG_LEVEL_SYM)
/** @brief Macro for adding prefix level symbols to log (See EVQ_LOG_SOURCE) */
#define EVQ_LOG_SYM_ASSERTION "A: "
/** @brief Macro for adding prefix level symbols to log (See EVQ_LOG_SOURCE) */
#define EVQ_LOG_SYM_ERROR "E: "
/** @brief Macro for adding prefix level symbols to log (See EVQ_LOG_SOURCE) */
#define EVQ_LOG_SYM_WARNING "W: "
/** @brief Macro for adding prefix level symbols to log (See EVQ_LOG_SOURCE) */
#define EVQ_LOG_SYM_INFO "I: "
/** @brief Macro for adding prefix level symbols to log (See EVQ_LOG_SOURCE) */
#define EVQ_LOG_SYM_DEBUG "D: "
/** @brief Macro for adding prefix level symbols to log (See EVQ_LOG_SOURCE) */
#define EVQ_LOG_SYM_TRACE "T: "
#else
#define EVQ_LOG_SYM_ASSERTION
#define EVQ_LOG_SYM_ERROR
#define EVQ_LOG_SYM_WARNING
#define EVQ_LOG_SYM_INFO
#define EVQ_LOG_SYM_DEBUG
#define EVQ_LOG_SYM_TRACE
#endif

#if defined(EVQ_LOG_SOURCE)
/** @brief Macro for adding source details to log (See EVQ_LOG_SOURCE) */
#define _EVQ_FUNC_STR "[%s] "
/** @brief Macro for adding source details to log (See EVQ_LOG_SOURCE) */
#define _EVQ_FUNC , __func__
/** @brief Macro for adding source details to log (See EVQ_LOG_SOURCE) */
#define _EVQ_LINE_STR "[%d]: "
/** @brief Macro for adding source details to log (See EVQ_LOG_SOURCE) */
#define _EVQ_LINE , __LINE__
#else
#define _EVQ_FUNC_STR
#define _EVQ_LINE
#define _EVQ_LINE_STR
#define _EVQ_FUNC
#endif

#if defined(EVQ_LOGGING)
#define _EVQ_LOG(LEVEL, STR, ...) \
    if (LEVEL <= EVQ_LOG_LEVEL_MAX) evq_log(LEVEL, STR, ##__VA_ARGS__)
/** @brief Helper macro for logging */
#define EVQ_LOG(LEVEL, STR, ...) _EVQ_LOG(LEVEL, STR, ##__VA_ARGS__)
/** @brief Helper macro for getting current log level */
#define EVQ_LOG_GET_LEVEL() evq_log_get_level()
/** @brief Helper macro for setting current log level */
#define EVQ_LOG_SET_LEVEL(LEVEL) evq_log_set_level(LEVEL)
/** @brief Helper macro for error logging */
#define EVQ_LOG_ERROR(STR, ...)                                                    \
    EVQ_LOG(EVQ_LOG_LEVEL_ERROR,                                                   \
            _EVQ_FUNC_STR _EVQ_LINE_STR EVQ_LOG_SYM_ERROR STR _EVQ_FUNC _EVQ_LINE, \
            ##__VA_ARGS__)
/** @brief Helper macro for warning logging */
#define EVQ_LOG_WARNING(STR, ...)                                                    \
    EVQ_LOG(EVQ_LOG_LEVEL_WARNING,                                                   \
            _EVQ_FUNC_STR _EVQ_LINE_STR EVQ_LOG_SYM_WARNING STR _EVQ_FUNC _EVQ_LINE, \
            ##__VA_ARGS__)
/** @brief Helper macro for info logging */
#define EVQ_LOG_INFO(STR, ...)                                                    \
    EVQ_LOG(EVQ_LOG_LEVEL_INFO,                                                   \
            _EVQ_FUNC_STR _EVQ_LINE_STR EVQ_LOG_SYM_INFO STR _EVQ_FUNC _EVQ_LINE, \
            ##__VA_ARGS__)
/** @brief Helper macro for debug logging */
#define EVQ_LOG_DEBUG(STR, ...)                                                    \
    EVQ_LOG(EVQ_LOG_LEVEL_DEBUG,                                                   \
            _EVQ_FUNC_STR _EVQ_LINE_STR EVQ_LOG_SYM_DEBUG STR _EVQ_FUNC _EVQ_LINE, \
            ##__VA_ARGS__)
/** @brief Helper macro for trace logging */
#define EVQ_LOG_TRACE(STR, ...)                                                    \
    EVQ_LOG(EVQ_LOG_LEVEL_TRACE,                                                   \
            _EVQ_FUNC_STR _EVQ_LINE_STR EVQ_LOG_SYM_TRACE STR _EVQ_FUNC _EVQ_LINE, \
            ##__VA_ARGS__)
#else
#define EVQ_LOG(...)
#define EVQ_LOG_GET_LEVEL(...)
#define EVQ_LOG_SET_LEVEL(...)
#define EVQ_LOG_ERROR(...)
#define EVQ_LOG_WARNING(...)
#define EVQ_LOG_INFO(...)
#define EVQ_LOG_DEBUG(...)
#define EVQ_LOG_TRACE(...)
#endif

#if defined(EVQ_ASSERT_ENABLE)
#define EVQ_ASSERT(COND, STR)                           \
    {                                                   \
        if ((COND) ? 0 : 1)                             \
        {                                               \
            evq_assert(#COND, __FILE__, __LINE__, STR); \
        }                                               \
    }
#else
#define EVQ_ASSERT(...)
#endif

#ifdef __cplusplus
}
#endif
#endif
