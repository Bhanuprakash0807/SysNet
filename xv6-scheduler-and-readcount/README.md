<div align="center">

# ⚙️ XV6 Scheduler

**Kernel-level scheduler implementations inside xv6 — FCFS, Completely Fair Scheduler (CFS), and bonus MLFQ — with virtual runtime tracking, time slice calculation, and comparative performance analysis.**

[![Language](https://img.shields.io/badge/Language-C%20(Kernel)-A8B9CC?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![OS](https://img.shields.io/badge/OS-xv6%20(RISC--V)-blueviolet)](https://github.com/mit-pdos/xv6-riscv)
[![Build](https://img.shields.io/badge/Build-Make%20%2B%20QEMU-orange)](https://www.qemu.org/)

</div>

---

## 📖 Overview

This project modifies the [MIT xv6 teaching OS](https://github.com/mit-pdos/xv6-riscv) to implement and compare three CPU scheduling algorithms at the kernel level. All schedulers are compiled via a `SCHEDULER` macro at build time — the kernel includes only one policy per binary, making comparison clean and isolated.

Each scheduler is implemented inside `kernel/proc.c`, with supporting changes to `kernel/proc.h`, `kernel/trap.c`, and user-space programs.

---

## ✨ Features

### Part A — `getreadcount` System Call

| Feature | Details |
|---|---|
| **`sys_getreadcount()`** | New system call that returns the total number of bytes read via `read()` across all processes since boot |
| **Overflow handling** | Wraps to 0 on overflow |
| **`readcount` user program** | Reads 100 bytes from a file, calls `getreadcount()` before and after, verifies the delta |

---

### Part B — Scheduling Policies

All three policies are selectable at compile time via the `SCHEDULER` macro:

```bash
make clean && make qemu                    # default: Round Robin
make clean && make qemu SCHEDULER=FCFS     # First Come First Serve
make clean && make qemu SCHEDULER=CFS      # Completely Fair Scheduler
```

---

#### Round Robin (Default)

The unmodified xv6 scheduler — cycles through `RUNNABLE` processes in order, each getting one timer tick.

---

#### First Come First Serve (FCFS)

| Feature | Details |
|---|---|
| **Selection** | Always picks the `RUNNABLE` process with the earliest creation time |
| **Non-preemptive** | Once a process is running, it holds the CPU until it terminates or blocks |
| **`creation_time`** | Added to `struct proc`; set in `allocproc()` using the tick counter |

```c
// Scheduler selects minimum creation_time among RUNNABLE processes
struct proc *earliest = 0;
for (p = proc; p < &proc[NPROC]; p++) {
    if (p->state == RUNNABLE)
        if (!earliest || p->creation_time < earliest->creation_time)
            earliest = p;
}
```

---

#### Completely Fair Scheduler (CFS)

| Feature | Details |
|---|---|
| **Nice values** | Each process has a nice value (−20 to +19); affects scheduling weight |
| **Weight calculation** | `weight = 1024 / (1.25 ^ nice)`; nice 0 → 1024, nice −20 → 88761, nice 19 → 15 |
| **Virtual runtime** | Each process tracks `vruntime` — CPU time consumed, normalized by weight |
| **Selection** | Always schedules the `RUNNABLE` process with the smallest `vruntime` |
| **Time slice** | `time_slice = target_latency (48 ticks) / num_runnable_processes`; minimum 3 ticks |
| **Preemption** | Process is preempted after its calculated time slice via the tick interrupt in `kernel/trap.c` |
| **New process insertion** | New processes inserted into runqueue at the current minimum `vruntime` to prevent starvation |

```
[Scheduler Tick]
PID: 3 | vRuntime: 200
PID: 4 | vRuntime: 150
PID: 5 | vRuntime: 180
--> Scheduling PID 4 (lowest vRuntime)
```

---

#### Bonus — MLFQ (Multi-Level Feedback Queue)

| Feature | Details |
|---|---|
| **Queues** | 4 priority queues (0 = highest); time slices: 1, 4, 8, 16 ticks |
| **New processes** | Start at queue 0 |
| **Demotion** | Process uses full time slice → moved to next lower queue |
| **Voluntary yield** | I/O-bound processes re-enter the same queue on wakeup |
| **Preemption** | Higher-priority queue arrival preempts lower-priority running process at next tick |
| **Starvation prevention** | Every 48 ticks, all processes are boosted back to queue 0 |
| **Queue 3** | Uses round-robin among lowest-priority processes |

---

### Performance Comparison

Run `schedulertest` inside xv6 (single CPU, `CPUS=1`) to measure waiting time and running time per scheduler:

| Metric | Round Robin | FCFS | CFS |
|---|---|---|---|
| Avg. Waiting Time | — | — | — |
| Avg. Running Time | — | — | — |

> Actual numbers are documented in [`report.md`](./xv6/report.md) along with vruntime logs and scheduler decision traces.

The report includes:
- Brief explanation of each specification and the changes made
- vruntime log traces showing scheduling decisions
- Performance comparison table across RR, FCFS, CFS (and MLFQ if attempted)

---

## 🗂️ Project Structure

```
xv6/
├── xv6_modifications.patch   # Patch file of all kernel changes
├── readcount.c               # User program for getreadcount syscall
└── report.md                 # Implementation notes + performance comparison
```

Key kernel files modified:

```
kernel/
├── proc.h       # struct proc additions: creation_time, vruntime, nice, priority_queue
├── proc.c       # scheduler(), allocproc(), scheduler decision logging
├── trap.c       # Timer interrupt — time slice enforcement for CFS/MLFQ
└── syscall.c    # sys_getreadcount() registration
```

---

## ⚙️ Build & Run

### Prerequisites

- QEMU (`sudo apt install qemu-system-riscv64` on Linux)
- RISC-V GCC toolchain (`gcc-riscv64-unknown-elf`)

### Build

```bash
# Default Round Robin
make clean && make qemu

# FCFS
make clean && make qemu SCHEDULER=FCFS

# CFS
make clean && make qemu SCHEDULER=CFS

# MLFQ (bonus)
make clean && make qemu SCHEDULER=MLFQ
```

### Run schedulertest

Inside the xv6 shell:

```
$ schedulertest
```

Observe average waiting and running times printed to console. Use `Ctrl-P` (procdump) to inspect live process states.

---

## 📋 Key Implementation Notes

- **Preprocessor guards** isolate each scheduler with `#ifdef SCHEDULER_FCFS` / `#ifdef SCHEDULER_CFS` blocks inside `scheduler()` — the default round-robin code is preserved
- **`procdump`** is extended to print `vruntime`, nice value, and queue level per process for debugging
- **CFS vruntime** is updated in the timer interrupt handler: `vruntime += ticks_run * (1024 / weight)` — heavier-weight processes accumulate vruntime more slowly, giving them more CPU time
- **MLFQ boost** is triggered by a global tick counter mod 48 inside `kernel/trap.c`
- **Single-CPU testing** — set `CPUS=1` in the Makefile to get deterministic scheduling measurements
