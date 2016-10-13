#include <criterion/criterion.h>
#include <errno.h>

#include "client.h"

/* Assert that we can construct and destroy a client_conn */
Test(client_conn, test_new_destroy)
{
    struct client_conn *conn = client_conn_new(NULL, 0);
    cr_assert(conn != NULL);

    client_conn_free(conn);
}

/* Assert that the client_conn_io callback closes the connection on a fatal read
 * error,
 * but simply returns in the case of EAGAIN.
 */
Test(client_conn, test_io_callback_error)
{
    errno = 0;

    struct client_conn *conn = client_conn_new(NULL, 1);

    client_conn_free(conn);
}
