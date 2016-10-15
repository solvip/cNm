#include <arpa/inet.h> // for htons/ntohs
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ev.h>

#include "client_conn.h"
#include "util.h"

const int listen_backlog = 128;

static int socket_bind_listen_or_exit(const int32_t port);

static void listener_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    if (revents & EV_ERROR) {
        log_printf(
            "error in listener_callback; nothing to do but break and die\n");
        ev_break(loop, EVBREAK_ALL);

        return;
    }

    struct sockaddr client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int client_fd = accept(w->fd, &client_addr, &client_addr_size);
    if (client_fd < 0) {
        log_printf("unable to accept: %s", strerror(errno));
        return;
    }

    if (!set_nonblock(client_fd)) {
        log_printf(
            "Closing newly accepted connection; unable to set nonblocking: %s",
            strerror(errno));
        close(client_fd);
        return;
    }

    struct client_conn *c = client_conn_new(loop, client_fd);
    if (c == NULL) {
        log_printf(
            "Closing newly accepted connection; unable to allocate memory: %s",
            strerror(errno));
        client_conn_close(&c);
        return;
    }

    client_conn_start_watchers(c);

    return;
}

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        log_fatalf("Invalid usage\nExpected: %s <port>\nExample: %s 1234\n",
                   argv[0], argv[0]);
    }

    int32_t port;
    if (!parse_int32_t(&port, argv[1])) {
        log_fatalf("Unable to parse port number as integer\n");
    }

    signal(SIGPIPE, SIG_IGN);

    int listenfd = socket_bind_listen_or_exit(port);
    if (!set_nonblock(listenfd)) {
        log_fatalf("unable to set O_NONBLOCK on listenfd: %s", strerror(errno));
        ;
    }

    struct ev_loop *loop =
        ev_default_loop(EVBACKEND_EPOLL | EVBACKEND_KQUEUE | EVFLAG_NOENV);
    if (loop == NULL) {
        log_fatalf("unable to initialize ev loop.  Are you sure you have "
                   "epoll or kqueue?");
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
        log_fatalf("unable to create socket: %s", strerror(errno));
    }

    int rc =
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (rc < 0) {
        log_fatalf("unable to set SO_REUSEADDR: %s", strerror(errno));
    }

    rc = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
    if (rc < 0) {
        log_fatalf("unable to set SO_REUSEPORT: %s", strerror(errno));
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {0},
        .sin_zero = {0},
    };
    rc = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rc < 0) {
        log_fatalf("unable to bind socket: %s", strerror(errno));
    }

    if (listen(listenfd, listen_backlog) < 0) {
        log_fatalf("unable to listen on socket: %s", strerror(errno));
    }

    return listenfd;
}
