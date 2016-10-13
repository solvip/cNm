#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ev.h>

#define container_of(ptr, type, member)                                        \
    ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) -                     \
              offsetof(type, member)))

/* the signature of read(2) */
typedef ssize_t (*reader)(int filedes, void *buf, size_t nbyte);

struct client_conn {
    /* libev watches */
    struct ev_io r_io;
    struct ev_io w_io;
    struct ev_timer timer;

    /* Our event loop */
    struct ev_loop *loop;

    /* The read function called to read from fd.
     * We use a function pointer here to be able to mock
     * it out during tests
     */
    reader read;

    /* the underlying fd */
    int fd;
};

struct client_conn *client_conn_new(struct ev_loop *loop, int conn_fd);
void client_conn_start_watchers(struct client_conn *c);
void client_conn_free(struct client_conn *c);

bool client_conn_close(struct client_conn *c);
bool client_conn_shutdown(struct client_conn *c);
