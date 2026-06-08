<div align="center">

# 🐚 C Shell

**A POSIX-compliant Unix shell built from scratch in C — supporting piping, I/O redirection, background execution, job control, and a persistent command history.**

[![Language](https://img.shields.io/badge/Language-C%20(C99)-A8B9CC?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C99)
[![Standard](https://img.shields.io/badge/Standard-POSIX-blue)](https://pubs.opengroup.org/onlinepubs/9699919799/)
[![Build](https://img.shields.io/badge/Build-Make-orange)](https://www.gnu.org/software/make/)

</div>

---

## 📖 Overview

This project implements a fully functional Unix shell in C, using **only the POSIX C library** — no third-party libraries, no shortcuts. The shell parses a formal Context-Free Grammar, executes arbitrary programs, handles piped pipelines, redirects I/O, runs background jobs, and provides job control via signals. It is compiled with strict POSIX compliance flags and broken down into modular `.c`/`.h` files by functionality.

```bash
make all          # builds shell.out in the shell/ directory
./shell.out       # launch the shell
```

---

## ✨ Features

### Part A — Shell Input & Parsing

| Feature | Details |
|---|---|
| **Prompt Display** | Shows `<Username@SystemName:current_path>`; home directory replaced with `~` |
| **Input Handling** | Reads user input, re-prompts after each command |
| **CFG Parser** | Validates input against a formal Context-Free Grammar covering pipes, redirects, sequential and background operators |

The grammar handled by the parser:

```
shell_cmd  →  cmd_group ((& | ;) cmd_group)* &?
cmd_group  →  atomic (| atomic)*
atomic     →  name (name | input | output)*
input      →  < name
output     →  > name | >> name
```

---

### Part B — Shell Intrinsics

| Command | Syntax | Purpose |
|---|---|---|
| `hop` | `hop [~ . .. - path]` | Change working directory; supports `~`, `..`, `-` (previous dir), relative and absolute paths |
| `reveal` | `reveal [-a] [-l] [path]` | List directory contents; `-a` shows hidden files, `-l` shows one per line; lexicographic order |
| `log` | `log [purge \| execute <n>]` | Persistent command history (max 15); survives shell restarts; supports re-execution and purge |

---

### Part C — I/O Redirection & Piping

| Feature | Details |
|---|---|
| **Input Redirection** | `command < file` — reads stdin from file using `open()` + `dup2()` |
| **Output Redirection** | `command > file` (truncate) and `command >> file` (append) |
| **Command Piping** | `cmd1 \| cmd2 \| ... \| cmdN` — creates pipes with `pipe()`, forks per command, chains stdout→stdin |
| **Combined** | `cmd1 < input.txt \| cmd2 > output.txt` — redirection and pipes work together |

All file descriptors are properly closed after `dup2()` to prevent leaks.

---

### Part D — Sequential & Background Execution

| Feature | Details |
|---|---|
| **Sequential** | `cmd1 ; cmd2 ; cmd3` — executes in order, waits for each to finish before the next |
| **Background** | `cmd &` — forks without waiting; prints `[job_number] pid`; prints exit status when done |
| **Completion Notification** | Before each new prompt, checks for completed background processes and prints exit status |

---

### Part E — Job Control & Signals

| Feature | Details |
|---|---|
| **`activities`** | Lists all running/stopped shell-spawned processes with PID, name, and state |
| **`ping`** | `ping <pid> <signal>` — sends signal (mod 32) to a process |
| **Ctrl-C** | Sends `SIGINT` to the foreground process group; shell itself is protected |
| **Ctrl-D** | Sends `SIGKILL` to all children, prints `logout`, exits cleanly |
| **Ctrl-Z** | Sends `SIGTSTP` to foreground process; moves it to background as Stopped |
| **`fg [n]`** | Brings a background/stopped job to foreground; resumes stopped jobs with `SIGCONT` |
| **`bg [n]`** | Resumes a stopped background job with `SIGCONT`; prints error if already running |

---

## 🗂️ Project Structure

```
custom-shell/
 ├── Makefile
 ├── include/
 │   ├── background.h
 │   ├── execute.h
 │   ├── hop.h
 │   ├── input.h
 │   ├── log.h
 │   ├── parser.h
 │   ├── ping.h
 │   ├── prompt.h
 │   ├── reveal.h
 │   └── shell.h
 └── src/
     ├── background.c
     ├── execute.c
     ├── hop.c
     ├── input.c
     ├── log.c
     ├── main.c
     ├── parser.c
     ├── ping.c
     ├── prompt.c
     └── reveal.c ```
```

---

## ⚙️ Build & Run

```bash
cd shell
make all          # produces shell.out
./shell.out       # start the shell
```

Compiled with strict flags for POSIX compliance:

```bash
gcc -std=c99 \
  -D_POSIX_C_SOURCE=200809L \
  -D_XOPEN_SOURCE=700 \
  -Wall -Wextra -Werror \
  -Wno-unused-parameter \
  -fno-asm
```

> **Only POSIX C library headers are used** — no third-party dependencies.

---

## 📋 Key Implementation Notes

- **Parser** validates the full CFG before attempting execution — invalid syntax is caught early and reported cleanly
- **`log` history** is stored on disk and persists across shell sessions; commands with `log` as the root are never stored
- **Signal safety** — the shell installs handlers for `SIGINT`, `SIGTSTP`, and `SIGCHLD`; child process groups are managed to prevent unintended signal propagation
- **`reveal`** sorts entries by ASCII value (not locale-aware collation) to match expected lexicographic ordering
- **File descriptor hygiene** — every `pipe()` and `open()` file descriptor is closed in both parent and child after `dup2()`
