#include <arpa/inet.h>
#include <criterion/criterion.h>
#include <stdbool.h>

#include "util.h"

/* Assert that parse_hostport fails for invalid input */
Test(util, parse_hostport_fail)
{
    struct sockaddr_in addr;
    bool ret = parse_hostport("", &addr);
    cr_assert(!ret);

    ret = parse_hostport("127..", &addr);
    cr_assert(!ret);

    ret = parse_hostport("127.0.0.1", &addr);
    cr_assert(!ret);

    ret = parse_hostport("127.0.0.1:", &addr);
    cr_assert(!ret);
}

/* Assert that parse_hostport succeeds for valid input */
Test(util, parse_hostport)
{
    struct sockaddr_in addr;
    cr_assert(parse_hostport("127.0.0.1:1234", &addr));
    cr_assert_str_eq(inet_ntoa(addr.sin_addr), "127.0.0.1");
    cr_assert(htons(addr.sin_port) == 1234);
}
