#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ev.h>

#define container_of(ptr, type, member)                                        \
    ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) -                     \
              offsetof(type, member)))

struct client_conn {
    /* libev watches */
    struct ev_io r_io;
    struct ev_io w_io;
    struct ev_timer timer;

    /* Our event loop */
    struct ev_loop *loop;

    /* the underlying fd */
    int fd;
};

struct client_conn *client_conn_new(struct ev_loop *loop, int conn_fd);
void client_conn_start_watchers(struct client_conn *c);
void client_conn_close(struct client_conn **c);
