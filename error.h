#ifndef ERROR_H
#define ERROR_H

#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <err.h>
#include <sysexits.h>

void usage(const char *fmt, ...);

#endif
