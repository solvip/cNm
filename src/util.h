#pragma once

#include <stdbool.h>
#include <stdlib.h>

#define log_fatalf(...)                                                        \
    do {                                                                       \
        log_printf(__VA_ARGS__);                                               \
        exit(1);                                                               \
    } while (0)

bool parse_int32_t(int *result, const char *string);
void perror_and_exit(const char *msg, const int exitcode);
bool set_nonblock(const int fd);
void log_printf(const char *restrict format, ...);
bool parse_hostport(const char *hostport, struct sockaddr_in *dst);
