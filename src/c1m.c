#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static bool parse_int32_t(int *result, const char *string);

#ifndef test
int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,
                "Invalid usage\nExpected: %s <port>\nExample: %s 1234\n",
                argv[0], argv[0]);
        exit(1);
    }

    int32_t port;
    if (!parse_int32_t(&port, argv[1])) {
        fprintf(stderr, "Unable to parse port number as integer\n");
        exit(1);
    }
}
#endif

/* parse_int - Parse an integer from string and store the result in *result.
 * On failure, return false
 */
static bool parse_int32_t(int32_t *result, const char *string) {
    intmax_t i;
    char *endptr;

    errno = 0;
    i = strtoimax(string, &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        return false;
    }

    if (i > INT32_MAX) {
        return false;
    }

    *result = (int32_t)i;

    return true;
}
