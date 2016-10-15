#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void log_printf(const char *restrict format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    vfprintf(stderr, format, arglist);
    fprintf(stderr, "\n");
    va_end(arglist);
}

/* parse_int - Parse an integer from string and store the result in *result.
 * On failure, return false
 */
bool parse_int32_t(int32_t *result, const char *string)
{
    intmax_t i;
    char *endptr;

    errno = 0;
    i = strtoimax(string, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || i > INT32_MAX) {
        return false;
    }

    *result = (int32_t)i;

    return true;
}

/* set_nonblock - Set the O_NONBLOCK flag on fd */
bool set_nonblock(const int fd)
{
    int curflags = fcntl(fd, F_GETFL, 0);
    if (curflags < 0) {
        return false;
    }

    if (fcntl(fd, F_SETFL, curflags | O_NONBLOCK) < 0) {
        return false;
    } else {
        return true;
    }
}
