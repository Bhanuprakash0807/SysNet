# Systems Programming Projects

A collection of three systems-programming projects focused on networking, shell design, and kernel-level work.

## Projects

### [Reliable UDP Protocol](reliable-udp-protocol/README.md)
A custom client/server transport built on UDP with handshake-based session setup, reliable file transfer, retransmission, chat mode, and configurable packet-loss simulation.

### [Custom Shell](custom-shell/README.md)
An interactive shell written in C with command parsing, pipes, redirection, background jobs, logging, replay, and signal handling.

### [xv6 Scheduler and Read Count](xv6-scheduler-and-readcount/README.md)
xv6 kernel work that adds scheduler variants and a `getreadcount()` system call, along with a user program that demonstrates the feature.

## Highlights

- Systems-level C programming across user space and kernel space.
- Custom protocol design and UDP reliability handling.
- Shell parsing, process control, and job management.
- xv6 kernel modifications and user-space syscall testing.

## Repository Map

```text
systems-programming-projects/
├── custom-shell/
├── reliable-udp-protocol/
└── xv6-scheduler-and-readcount/
```

Each project folder includes its own README with build steps, run instructions, and example usage.
