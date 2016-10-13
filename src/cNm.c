#include <arpa/inet.h> // for htons/ntohs
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ev.h>

#include "client.h"

#define container_of(ptr, type, member)                                        \
    ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) -                     \
              offsetof(type, member)))

const int listen_backlog = 128;

static bool parse_int32_t(int *result, const char *string);
static void perror_and_exit(const char *msg, const int exitcode);
static int socket_bind_listen_or_exit(const int32_t port);
static bool set_nonblock(const int fd);

static void listener_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    if(revents & EV_ERROR) {
        fprintf(stderr, "error in listener_callback; nothing to do but break and die\n");
        ev_break(loop, EVBREAK_ALL);

        return;
    }
    
    struct sockaddr client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int client_fd = accept(w->fd, &client_addr, &client_addr_size);
    if (client_fd < 0) {
        perror("unable to accept");
        return;
    }

    if (!set_nonblock(client_fd)) {
        perror("Closing newly accepted connection; unable to set nonblocking");
        close(client_fd);
        return;
    }

    struct client_conn *c = client_conn_new(loop, client_fd);
    if (c == NULL) {
        perror("Closing newly accepted connection; unable to allocate memory");
        client_conn_close(c);
        client_conn_free(c);
        return;
    }

    client_conn_start_watchers(c);

    return;
}

int main(int argc, const char *argv[])
{
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

    signal(SIGPIPE, SIG_IGN);

    int listenfd = socket_bind_listen_or_exit(port);
    if (!set_nonblock(listenfd)) {
        perror_and_exit("unable to set O_NONBLOCK on listenfd", 1);
    }

    struct ev_loop *loop =
        ev_default_loop(EVBACKEND_EPOLL | EVBACKEND_KQUEUE | EVFLAG_NOENV);
    if (loop == NULL) {
        perror_and_exit("unable to initialize ev loop.  Are you sure you have "
                        "epoll or kqueue?",
                        1);
    }
    ev_io listener_watcher;

    ev_io_init(&listener_watcher, listener_cb, listenfd, EV_READ);
    ev_io_start(loop, &listener_watcher);

    ev_run(loop, 0);

    shutdown(listenfd, SHUT_RDWR);
    close(listenfd);
}

static int socket_bind_listen_or_exit(int32_t port)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror_and_exit("unable to create socket", 1);
    }

    int rc =
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (rc < 0) {
        perror_and_exit("unable to set SO_REUSEADDR", 1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {0},
        .sin_zero = {0},
    };
    rc = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rc < 0) {
        perror_and_exit("unable to bind socket", 1);
    }

    if (listen(listenfd, listen_backlog) < 0) {
        perror_and_exit("unable to listen on socket", 1);
    }

    return listenfd;
}

static void perror_and_exit(const char *msg, int exitcode)
{
    perror(msg);
    exit(exitcode);
}

/* parse_int - Parse an integer from string and store the result in *result.
 * On failure, return false
 */
static bool parse_int32_t(int32_t *result, const char *string)
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
static bool set_nonblock(const int fd)
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
