<div align="center">

# 📡 S.H.A.M. — Reliable UDP Protocol

**A TCP-like reliable transport protocol built on top of raw UDP in C — implementing three-way handshake, sliding window, cumulative ACKs, retransmission, flow control, and real-time chat.**

[![Language](https://img.shields.io/badge/Language-C%20(C99)-A8B9CC?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C99)
[![Protocol](https://img.shields.io/badge/Transport-UDP-blue)](https://en.wikipedia.org/wiki/User_Datagram_Protocol)
[![Build](https://img.shields.io/badge/Build-Make-orange)](https://www.gnu.org/software/make/)

</div>

---

## 📖 Overview

S.H.A.M. (**S**equenced, **H**andshaked, **A**cknowledged **M**essaging) is a reliable transport protocol implemented entirely over UDP datagrams. Starting from an inherently unreliable channel, this project builds TCP's core reliability mechanisms from scratch — connection management, ordered delivery, retransmission on timeout, and receiver-side flow control — without using TCP itself.

The implementation supports two modes: **file transfer** (client sends a file, server verifies it via MD5 checksum) and **real-time chat** (bidirectional interactive messaging).

---

## ✨ Features

### Protocol Design

| Mechanism | Details |
|---|---|
| **S.H.A.M. Packet Header** | Custom `struct sham_header` with sequence number, acknowledgment number, flags (SYN/ACK/FIN), and window size |
| **Three-Way Handshake** | SYN → SYN-ACK → ACK connection establishment before any data flows |
| **Four-Way Termination** | FIN → ACK → FIN → ACK graceful teardown from either side |
| **Data Segmentation** | Files/input split into fixed 1024-byte chunks; each chunk is one S.H.A.M. packet payload |
| **Sliding Window** | Sender keeps up to N unacknowledged packets in flight without waiting for each ACK |
| **Cumulative ACKs** | Receiver acknowledges the highest contiguous byte received; out-of-order packets are buffered |
| **Retransmission Timeout** | Per-packet timer (default 500ms); only the lost packet is retransmitted, not the entire window |
| **Flow Control** | Receiver advertises available buffer space in `window_size`; sender never exceeds it |
| **Simulated Packet Loss** | Optional `loss_rate` argument causes the receiver to programmatically drop packets for testing |

---

### Packet Header Structure

```c
struct sham_header {
    uint32_t seq_num;      // byte-stream position of first data byte
    uint32_t ack_num;      // next byte the receiver expects (cumulative)
    uint16_t flags;        // SYN=0x1, ACK=0x2, FIN=0x4
    uint16_t window_size;  // receiver's available buffer space
};
```

---

### Retransmission — How It Works

```
Sender transmits window of 4 packets:
  SND DATA SEQ=1
  SND DATA SEQ=1025   ← lost in transit
  SND DATA SEQ=2049
  SND DATA SEQ=3073

Receiver buffers 1, 2049, 3073; sends:
  ACK=1025            ← stuck, packet 2 missing

Timer for SEQ=1025 expires:
  TIMEOUT SEQ=1025
  RETX DATA SEQ=1025

Receiver now has 1–4096 contiguous; sends:
  ACK=4097            ← cumulative, covers retransmit + buffered packets

Sender cancels timers for 1025, 2049, 3073. No unnecessary retransmits.
```

---

### Modes of Operation

#### File Transfer Mode (default)

```bash
# Start server
./server <port> [loss_rate]

# Start client
./client <server_ip> <server_port> <input_file> <output_file> [loss_rate]
```

- Client sends the file over the S.H.A.M. protocol
- Server receives, saves, and prints the MD5 checksum of the received file
- MD5 output format: `MD5: <32-char-lowercase-hash>`
- Use this to verify byte-perfect transfer even under simulated packet loss

#### Chat Mode

```bash
./server <port> --chat [loss_rate]
./client <server_ip> <server_port> --chat [loss_rate]
```

- Both sides enter a concurrent read loop using `select()` to monitor stdin and the socket simultaneously
- Messages are sent reliably over S.H.A.M.
- Type `/quit` to initiate the four-way FIN handshake and close the connection

---

### Verbose Logging

Activate with `RUDP_LOG=1` environment variable. Writes timestamped logs to `server_log.txt` / `client_log.txt`.

```bash
RUDP_LOG=1 ./server 8080
RUDP_LOG=1 ./client 127.0.0.1 8080 file.txt received.txt
```

Log format (microsecond precision via `gettimeofday()`):

```
[2025-08-03 17:38:15.123456] [LOG] RCV SYN SEQ=100
[2025-08-03 17:38:15.123589] [LOG] SND SYN-ACK SEQ=5000 ACK=101
[2025-08-03 17:38:15.124821] [LOG] RCV ACK FOR SYN
[2025-08-03 17:38:15.125111] [LOG] RCV DATA SEQ=101 LEN=1024
[2025-08-03 17:38:15.125185] [LOG] SND ACK=1125 WIN=8192
[2025-08-03 17:38:15.626234] [LOG] TIMEOUT SEQ=1125
[2025-08-03 17:38:15.626301] [LOG] RETX DATA SEQ=1125 LEN=1024
[2025-08-03 17:38:15.628991] [LOG] RCV ACK=2149
```

All required log events are covered: handshake, data send/receive, ACKs, retransmits, flow control window updates, and simulated drops.

---

## 🗂️ Project Structure

```
networking/
├── client.c        # Client: handshake, file send / chat loop
├── server.c        # Server: handshake, file receive / chat loop, MD5 output
├── sham.h          # Packet header struct, flag constants, shared utilities
└── Makefile
```

---

## ⚙️ Build & Run

### Prerequisites

**Linux:**
```bash
sudo apt update && sudo apt install libssl-dev
gcc client.c -o client -lcrypto
gcc server.c -o server -lcrypto
```

**macOS:**
```bash
brew install openssl
gcc client.c -o client -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib -lcrypto
gcc server.c -o server -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib -lcrypto
```

Or simply:
```bash
cd networking
make all
```

### File Transfer Example

```bash
# Terminal 1 — server
RUDP_LOG=1 ./server 8080 0.1

# Terminal 2 — client (10% simulated packet loss)
RUDP_LOG=1 ./client 127.0.0.1 8080 bigfile.bin received.bin 0.1
```

Server prints: `MD5: d41d8cd98f00b204e9800998ecf8427e`

Compare with: `md5sum bigfile.bin` to verify integrity.

### Chat Example

```bash
./server 8080 --chat
./client 127.0.0.1 8080 --chat
# Type messages; /quit to close
```

---

## 📋 Key Implementation Notes

- **`select()`** is used in chat mode to multiplex stdin and the network socket in a single thread — no threads needed
- **Receiver-side buffering** holds out-of-order packets until the missing packet arrives, then delivers them in order
- **Flow control** is enforced on every send: `LastByteSent - LastByteAcked ≤ receiver_window_size`
- **Packet loss simulation** is receiver-side only — the receiver randomly discards packets based on `loss_rate` before processing, exercising the sender's retransmission path
- **MD5 verification** uses OpenSSL's `libcrypto` — the only external dependency
