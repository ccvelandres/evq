#include <stdlib.h>

#include <evq/evq_log.h>
#include <evq/evq_port.h>

////////////////////////////////////////////////////////////////////
// Memory allocation wrappers
////////////////////////////////////////////////////////////////////

__attribute__((weak)) void *evq_malloc(uint32_t size)
{
    return malloc(size);
}

__attribute__((weak)) void *evq_calloc(uint32_t nmemb, uint32_t size)
{
    return calloc(nmemb, size);
}

__attribute__((weak)) void *evq_realloc(void *ptr, uint32_t size)
{
    return realloc(ptr, size);
}

__attribute__((weak)) void  evq_free(void *ptr)
{
    free(ptr);
}
