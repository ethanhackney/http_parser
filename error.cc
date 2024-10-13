#include "error.h"

void usage(const char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        errno = EINVAL;
        verr(EX_USAGE, fmt, va);
        va_end(va);
        exit(EXIT_FAILURE);
}
