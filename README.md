C1M / R1M
=========

C1M / R1M is an experiment in getting a single server to handle a 1M concurrent connections,
each doing 1 RPS, resulting in 1M RPS.

# Architecture

# Server configuration

Out of the box, the default Linux TCP parameters do not work for this scale.

# Development Dependencies

libev for for kqueue/epoll magic: http://software.schmorp.de/pkg/libev.html

criterion for tests: https://github.com/Snaipe/Criterion



