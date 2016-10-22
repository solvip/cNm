#pragma once

#include <stdint.h>

/* socket_tcp_dial - Create a TCP socket towards host:port.
 * On success, return a file descriptor.
 * On failure, the return value will be <0 and errno will probably be set.
 */
int socket_tcp_dial(const char *restrict host, const char *restrict port);
