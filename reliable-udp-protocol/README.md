# Reliable UDP Protocol

This project implements a UDP-based client/server protocol with reliability features layered on top of datagrams. It supports both file transfer and an interactive chat mode, along with connection setup, ordered delivery, retransmission, and optional packet-loss simulation.

## Features

- Three-way handshake before data transfer begins.
- Four-way close sequence when the session ends.
- File transfer with sequence numbers and ACK-based retransmission.
- Chat mode for interactive message exchange.
- Optional loss simulation using a configurable loss rate.
- Logging support controlled through the `RUDP_LOG=1` environment variable.
- OpenSSL MD5 support is linked in through the Makefile.

## Project Structure

```text
reliable-udp-protocol/
├── Makefile
├── client.c
├── server.c
├── sham.h
└── sham_utils.c
```

## Build

Run the build from inside this folder:

```bash
make
```

This produces two binaries:

- `server`
- `client`

On Linux, make sure the OpenSSL development package is installed so `-lcrypto` can be resolved.

## Run

Start the server:

```bash
./server <port> [--chat] [loss_rate]
```

Run the client in file-transfer mode:

```bash
./client <server_ip> <server_port> <input_file> <output_file_name> [loss_rate]
```

Run the client in chat mode:

```bash
./client <server_ip> <server_port> --chat [loss_rate]
```

## Examples

File transfer:

```bash
./server 9000
./client 127.0.0.1 9000 sample.txt received.txt
```

Chat mode with simulated packet loss:

```bash
RUDP_LOG=1 ./server 9000 --chat 0.10
RUDP_LOG=1 ./client 127.0.0.1 9000 --chat 0.10
```

## Notes

- The server writes received files to `received_file.txt` by default.
- Logs are written only when `RUDP_LOG=1` is set.
- The protocol is custom and is intended as an educational reliable-transport layer over UDP.