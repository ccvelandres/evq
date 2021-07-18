
#include <sys/lock.h>
#include <sys/reent.h>

#include <libopencm3/cm3/cortex.h>

#include "board.h"

/** this implements stubs for newlib */

_ssize_t _write(int fd, const char* ptr, uint32_t len)
{
    return th_log(ptr, len);
}

void __malloc_lock(struct _reent *r)
{
    (void)r;
    cm_disable_interrupts();
}

void __malloc_unlock(struct _reent *r)
{
    (void)r;
    cm_enable_interrupts();
}