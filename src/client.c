#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        log_fatalf(
            "Usage: %s ip:port num_conns\nExample: %s 127.0.0.1:1234 10000",
            argv[0], argv[0]);
    }

    struct sockaddr_in server_addr;
    if (!parse_hostport(argv[1], &server_addr)) {
        log_fatalf("Unable to parse host address");
    }

    return 0;
}
