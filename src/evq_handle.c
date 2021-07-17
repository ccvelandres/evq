#include <stddef.h>
#include <string.h>

#include <evq/evq_core.h>
#include <evq_core_p.h>
#include <evq/evq_config.h>
#include <evq/evq_port.h>
#include <evq/evq_log.h>

#include <evq_handle_p.h>

////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////

/**
 * @brief Add handle to handle array.
 *
 * Function does not lock context, caller is expected to lock.
 *
 * @param[in] privHandle handle to add
 * @return true if handle was added, else false
 */
static bool evq_priv_add_handle(evq_context_t *ctx, evq_handle_priv_t *privHandle)
{
    bool           added = false;

    for (uint16_t i = 0; (!added) && (i < EVQ_MAX_HANDLES); ++i)
    {
        if (NULL == ctx->handles[i])
        {
            ctx->handles[i] = privHandle;
            ctx->handleCount++;
            added = true;
        }
    }
    return added;
}

/**
 * @brief Removes handle to handle array.
 *
 * Function does not lock context, caller is expected to lock.
 *
 * @param[in] privHandle handle to remove
 * @return true if handle was removed, else false
 */
static bool evq_priv_remove_handle(evq_context_t *ctx, const evq_handle_priv_t *privHandle)
{
    bool           removed = false;

    for (uint16_t i = 0; (!removed) && (i < EVQ_MAX_HANDLES); ++i)
    {
        if (privHandle == ctx->handles[i])
        {
            ctx->handles[i] = NULL;
            ctx->handleCount--;
            removed = true;
        }
    }

    return removed;
}

/**
 * @brief Allocates a handle and initialize block to zeros
 *
 * TODO: Can be extended to have statically allocated block for handles
 * to allow compute memory usage during compile time
 *
 * @param[out] handle pointer to store allocated memory
 * @return EVQ_ERROR_NONE on success
 */
static evq_status_t evq_priv_handle_allocate(evq_handle_priv_t **handle)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = NULL;

    privHandle = evq_malloc(sizeof(evq_handle_priv_t));
    if (NULL == privHandle)
    {
        st = EVQ_ERROR_NMEM;
    }
    else
    {
        memset(privHandle, 0, sizeof(evq_handle_priv_t));
        *handle = privHandle;
    }

    return st;
}

/**
 * @brief Destroys allocated handle
 * @param[in] privHandle handle to destroy
 * @return EVQ_ERROR_NONE on success
 */
static evq_status_t evq_priv_handle_destroy(evq_handle_priv_t *privHandle)
{
    evq_status_t st = EVQ_ERROR_NONE;
    if (NULL != privHandle->rxStream) (void)evq_stream_destroy(privHandle->rxStream);
    if (NULL != privHandle->txStream) (void)evq_stream_destroy(privHandle->txStream);
    if (NULL != privHandle->egroup) (void)evq_egroup_destroy(privHandle->egroup);
    evq_free(privHandle);
    return st;
}

/**
 * @brief Helper function for se_handler for registering handles
 * @param[in] privHandle handle to register
 * @return EVQ_ERROR_NONE on success
 */
static evq_status_t evq_priv_register_handle(evq_context_t *ctx, const evq_se_msg_hdl_t *msg)
{
    evq_status_t       st         = EVQ_ERROR_MUTEX;
    evq_handle_priv_t *privHandle = msg->handle;

    if (EVQ_ERROR_NONE == (st = evq_mutex_lock(ctx->mutex, EVQ_CORE_TIMEOUT)))
    {
        if (NULL == evq_priv_lookup_handle_with_id(ctx, privHandle->handleId)
            && (evq_priv_add_handle(ctx, privHandle)))
        {
            privHandle->isRegistered = true;
            EVQ_LOG_TRACE("Registered handle(%X)\n", privHandle->handleId);
        }
        else
        {
            EVQ_LOG_TRACE("Could not register handle\n");
        }
        evq_mutex_unlock(ctx->mutex);
    }
    return st;
}

/**
 * Helper function for se_handler for unregistering handles
 * @param[in] privHandle handle to unregister
 * @return EVQ_ERROR_NONE on success
 */
static evq_status_t evq_priv_unregister_handle(evq_context_t *ctx, const evq_se_msg_hdl_t *msg)
{
    evq_status_t       st         = EVQ_ERROR_MUTEX;
    evq_handle_priv_t *privHandle = msg->handle;

    if (EVQ_ERROR_NONE == (st = evq_mutex_lock(ctx->mutex, EVQ_CORE_TIMEOUT)))
    {
        if (evq_priv_remove_handle(ctx, privHandle))
        {
            privHandle->isRegistered = false;
            EVQ_LOG_TRACE("Unregistered handle(%X)\n", privHandle->handleId);
            (void)evq_priv_handle_destroy(privHandle);
        }
        else
        {
            EVQ_LOG_TRACE("Handle(%X) not registered\n", privHandle->handleId);
        }
        evq_mutex_unlock(ctx->mutex);
    }
    return st;
}

evq_handle_t evq_priv_lookup_handle_with_id(evq_context_t *ctx, evq_id_t handleId)
{
    evq_handle_t   ret = NULL;

    for (uint16_t i = 0; (NULL == ret) && (i < EVQ_MAX_HANDLES); ++i)
    {
        if ((NULL != ctx->handles[i]) && (ctx->handles[i]->handleId == handleId))
        {
            ret = ctx->handles[i];
        }
    }
    return ret;
}

evq_status_t evq_handle_process(evq_context_t *ctx, const evq_core_message_t *cMsg)
{
    evq_status_t       st = 0;

    switch (cMsg->msgType)
    {
    case EVQ_SE_MSG_HDL_REGISTER:
        (void)evq_priv_register_handle(ctx, &cMsg->msg.hdl_register);
        break;
    case EVQ_SE_MSG_HDL_UNREGISTER:
        (void)evq_priv_unregister_handle(ctx, &cMsg->msg.hdl_unregister);
        break;
    default:
        EVQ_LOG_TRACE("Unhandled message type: %u\n", cMsg->msgType);
        break;
    }

    return st;
}

////////////////////////////////////////////////////////////////////
// Public API Functions
////////////////////////////////////////////////////////////////////

evq_status_t evq_handle_register(evq_handle_t *handle, const evq_handle_config_t *config)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = NULL;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(NULL != config, "config argument is null");

    st = evq_priv_handle_allocate(&privHandle);
    if (EVQ_ERROR_NONE != st)
    {
        EVQ_LOG_TRACE("Could not allocate memory for handle");
        return st;
    }

    privHandle->state         = EVQ_HS_UNINITIALIZED;
    privHandle->handleName    = config->handleName;
    privHandle->handleId      = config->handleId;
    privHandle->isRegistered  = false;
    privHandle->isActive      = false;
    privHandle->egroup        = NULL;
    privHandle->rxStream      = NULL;
    privHandle->txStream      = NULL;
    privHandle->seqNum        = 0;
    privHandle->eventHandler  = config->eventHandler;
    privHandle->eventSubCount = 0;
    memset(privHandle->eventList, 0, sizeof(privHandle->eventList));

    st = evq_stream_create(&privHandle->rxStream, sizeof(evq_message_t), config->streamSize);
    if ((EVQ_ERROR_NONE != st) || (NULL == privHandle->rxStream))
    {
        EVQ_LOG_ERROR("Could not create rx stream\n");
        goto cleanup;
    }

    st = evq_stream_create(&privHandle->txStream, sizeof(evq_message_t), config->streamSize);
    if ((EVQ_ERROR_NONE != st) || (NULL == privHandle->txStream))
    {
        EVQ_LOG_ERROR("Could not create tx stream\n");
        goto cleanup;
    }

    st = evq_egroup_create(&privHandle->egroup);
    if (EVQ_ERROR_NONE != st)
    {
        EVQ_LOG_ERROR("Could not create egroup\n");
        goto cleanup;
    }

    {
        // Send the register command to se stream
        evq_core_message_t cMsg;
        cMsg.msgType                 = EVQ_SE_MSG_HDL_REGISTER;
        cMsg.msg.hdl_register.handle = privHandle;

        st = evq_se_send_msg(&cMsg);
        if (EVQ_ERROR_NONE != st)
        {
            EVQ_LOG_TRACE("Could not send handle register message to core\n");
        }
    }

cleanup:
    if (EVQ_ERROR_NONE == st)
    {
        // Activate handle
        privHandle->isActive = true;
        privHandle->state    = EVQ_HS_IDLE;

        EVQ_LOG_TRACE("Handle(%X) created at %p\n", privHandle->handleId, (void *)privHandle);
        *handle = privHandle;
    }
    else
    {
        // cleanup resources on failure
        (void)evq_priv_handle_destroy(privHandle);
    }
    return st;
}

evq_status_t evq_handle_unregister(evq_handle_t *handle)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = *(evq_handle_priv_t **)handle;

    EVQ_ASSERT(NULL != handle, "handle argument is null");

    // Deactivate handle
    privHandle->isActive = false;
    privHandle->state    = EVQ_HS_UNREGISTERED;

    if (privHandle->isRegistered)
    {
        evq_core_message_t cMsg;
        cMsg.msgType                   = EVQ_SE_MSG_HDL_UNREGISTER;
        cMsg.msg.hdl_unregister.handle = privHandle;

        st = evq_se_send_msg(&cMsg);
        if (EVQ_ERROR_NONE != st)
        {
            EVQ_LOG_TRACE("Could not send handle unregister message to core\n");
        }
        else
        {
            // release handle from user on success
            *handle = NULL;
        }
    }
    else
    {
        // no need to send to core if handle is not registered
        // cleanup resources on failure
        (void)evq_priv_handle_destroy(privHandle);
        *handle = NULL;
    }

    return st;
}