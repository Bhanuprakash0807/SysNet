# Custom Shell

This project is an interactive shell implemented in C. It provides a prompt, command parsing, pipelines, redirection, built-in commands, background job tracking, and signal handling for foreground processes.

## Features

- Custom prompt showing the current user, host, and working directory.
- Tokenizer and parser for commands, pipes, redirection, `&`, and `;`.
- Built-in commands for directory navigation and file listing.
- Background job tracking with status reporting.
- Command history logging with replay support.
- `ping` command for sending signals to processes.
- Ctrl-C and Ctrl-Z handling for foreground jobs.

## Project Structure

```text
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
    └── reveal.c
```

## Build

Run the build from inside this folder:

```bash
make
```

The resulting executable is:

```text
shell.out
```

## Run

Start the shell with:

```bash
./shell.out
```

## Example Commands

Navigation:

```bash
hop
hop ..
hop /tmp
```

Directory listing:

```bash
reveal -a
reveal -l
reveal -al /etc
```

Pipelines and redirection:

```bash
cat input.txt | grep hello > output.txt
```

Background jobs and status:

```bash
sleep 5 &
activities
```

History and replay:

```bash
log
log execute 2
log purge
```

Signal sending:

```bash
ping 12345 9
```

## Notes

- The shell saves recent commands in `.shell_log`.
- Foreground jobs are tracked so Ctrl-C and Ctrl-Z can be forwarded to them.
- The parser accepts command groups separated by `|`, `&`, and `;`.