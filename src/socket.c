#include <netdb.h>

#include "socket.h"

struct socket {
    int fd;
    const char *error;
};

struct socket socket_tcp_dial(const char *restrict host, const char *restrict port)
{
    int s;
    const char *cause = NULL;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res0;
    int err = getaddrinfo(host, port, &hints, &res);
    if (err) {
        return struct socket {
            .fd = -1;
            .error = gai_strerror(error);
        }
    }

    int s = -1;
    for (struct addrinfo *cur = res; cur != NULL; cur = cur->ai_next) {
        s = socket(cur->ai_family, cur->ai_socktype,
                   cur->ai_protocol);
        if (s < 0) {
            continue;
        }
        
        if (connect(s, cur->ai_addr, cur->ai_addrlen) < 0) {
            close(s);
            s = -1;
            continue;
        }
        
        break;  /* okay we got one */
    }
    if (s < 0) {
        return struct socket {
            .fd = -1;
            .error = strerror(errno);
        }
    }
    freeaddrinfo(res);
}
