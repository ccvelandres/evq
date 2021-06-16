#include <cstdlib>

#include <evq/evq_log.h>
#include <evq/evq_port.h>

////////////////////////////////////////////////////////////////////
// Memory allocation wrappers
////////////////////////////////////////////////////////////////////

extern "C" void *evq_malloc(uint32_t size)
{
    return std::malloc(size);
}

extern "C" void *evq_calloc(uint32_t nmemb, uint32_t size)
{
    return std::calloc(nmemb, size);
}

extern "C" void *evq_realloc(void *ptr, uint32_t size)
{
    return std::realloc(ptr, size);
}

extern "C" void  evq_free(void *ptr)
{
    std::free(ptr);
}
