C1M / R1M
=========

C1M / R1M is an experiment in getting a single server to handle a 1M concurrent connections,
each doing 0.1 RPS, resulting in 100k RPS.

# Architecture

# Server configuration

Raise ulimit & maximum number of system-wide open files:
```
ulimit -n 104856
echo 2097120 > /proc/sys/fs/file-max
```

# Client configuration

Raise the local port range on client nodes:
```
echo 1024 65535 > /proc/sys/net/ipv4/ip_local_port_range
```

# Development Dependencies

libev for for kqueue/epoll magic: http://software.schmorp.de/pkg/libev.html

criterion for tests: https://github.com/Snaipe/Criterion


# Findings

An EC2 t2.medium can easily sustain hundreds of thousands of connections using epoll; but it runs
out of (kernel?) memory at close to 500k connections and the OOM killer starts killing clients.
