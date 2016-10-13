#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for shutdown
#include <unistd.h>     // for read, close

#include <ev.h>

#include "client.h"

static void read_io_callback(struct ev_loop *loop, struct ev_io *watcher,
                             int revents);
static void write_io_callback(struct ev_loop *loop, struct ev_io *watcher,
                              int revents);
static void timer_callback(struct ev_loop *loop, struct ev_timer *watcher,
                           int revents);

/* client_conn_new - Create a new heap-allocated client_conn */
struct client_conn *client_conn_new(struct ev_loop *loop, int conn_fd)
{
    struct client_conn *c = calloc(1, sizeof(struct client_conn));
    if (c == NULL) {
        return NULL;
    }
    c->loop = loop;
    c->read = read;
    c->fd = conn_fd;

    return c;
}

/* client_conn_init - Initialize watchers and start them */
void client_conn_start_watchers(struct client_conn *c)
{
    ev_io_init(&c->r_io, read_io_callback, c->fd, EV_READ);
    ev_io_start(c->loop, &c->r_io);

    ev_io_init(&c->w_io, write_io_callback, c->fd, EV_WRITE);
    ev_io_start(c->loop, &c->w_io);

    ev_timer_init(&c->timer, timer_callback, 10.0, 10.0);
    ev_timer_start(c->loop, &c->timer);
}

/* client_conn_free - Free the heap-allocated client_conn
 */
void client_conn_free(struct client_conn *c)
{
    ev_io_stop(c->loop, &c->r_io);
    ev_io_stop(c->loop, &c->w_io);
    ev_timer_stop(c->loop, &c->timer);

    free(c);
}

/* client_conn_shutdown - Shut down both sides of the connection.
 * The file descriptor is not closed.
 */
bool client_conn_shutdown(struct client_conn *c)
{
    if (shutdown(c->fd, SHUT_RDWR) < 0) {
        return false;
    } else {
        return true;
    }
}

/* client_conn_close - Close the connection associated with c.
 * It is not shut down beforehand.
 */
bool client_conn_close(struct client_conn *c)
{
    if (close(c->fd) < 0) {
        return false;
    } else {
        return true;
    }
}

static void read_io_callback(struct ev_loop *loop, struct ev_io *watcher,
                             int revents)
{
    struct client_conn *c = container_of(watcher, struct client_conn, r_io);
    assert(watcher == &c->r_io);

    if (revents & EV_ERROR) {
        fprintf(
            stderr,
            "received an error in read_io_callback; don't know what to do\n");
    }

    /* Got a read event, but we're not interested in those: we just consume
     * until EAGAIN. */
    ssize_t bytes_read;
    char buf[512];
    do {
        bytes_read = read(c->fd, buf, sizeof(buf));
    } while (bytes_read > 0);

    if (bytes_read < 0 && errno != EAGAIN) {
        perror("unable to read from socket");
        client_conn_close(c);
        client_conn_free(c);

        return;
    }

    return;
}

static void write_io_callback(struct ev_loop *loop, struct ev_io *watcher,
                              int revents)
{
    struct client_conn *c = container_of(watcher, struct client_conn, w_io);
    assert(watcher == &c->w_io);

    if (revents & EV_ERROR) {
        fprintf(
            stderr,
            "received an error in write_io_callback; don't know what to do\n");
    }

    ssize_t bytes_written = write(c->fd, "A", 1);
    if (bytes_written < 0 && errno == EAGAIN) {
        /* Try later */
        return;
    } else if (bytes_written < 0) {
        perror("unable to write to socket");
        client_conn_close(c);
        client_conn_free(c);

        return;
    }

    /* Stop the watcher; the timer will enable us when it's time to go again */
    ev_io_stop(c->loop, &c->w_io);
}

static void timer_callback(struct ev_loop *loop, struct ev_timer *watcher,
                           int revents)
{
    struct client_conn *c = container_of(watcher, struct client_conn, timer);
    assert(watcher == &c->timer);

    if (revents & EV_ERROR) {
        fprintf(
            stderr,
            "received an error in write_io_callback; don't know what to do\n");
    }

    /* The timer fired; nothing to do except ensure that the writer is started
     */
    ev_io_start(c->loop, &c->w_io);
}
