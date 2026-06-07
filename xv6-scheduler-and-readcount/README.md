# xv6 Scheduler and Read Count

This folder contains documentation and patch files for xv6 kernel work. The changes add a `getreadcount()` system call and scheduler support for multiple policies, including round-robin, FCFS, and CFS-style behavior.

## Features

- Added `getreadcount()` to report the cumulative number of bytes read.
- Scheduler-related kernel fields were added for FCFS and CFS bookkeeping.
- The scheduler logic was extended to support multiple policies through a compile-time `SCHEDULER` setting.
- A user program, `readcount.c`, demonstrates the system call.

## Project Structure

```text
xv6-scheduler-and-readcount/
├── readcount.c
├── report.md
└── xv6_modifications.patch
```

## How To Use

This project is designed to be applied to a matching xv6 source tree.

1. Apply `xv6_modifications.patch` to the xv6 repository.
2. Build xv6 using the scheduler configuration supported by your tree.
3. Boot xv6 and run the `readcount` user program.

## Example

Once xv6 is booted, run:

```text
readcount
```

The program prints the initial and final read counters and verifies that the count increases after a read operation.

## Scheduler Notes

- RR is the baseline policy.
- FCFS uses process creation time for ordering.
- CFS adds fields such as `vruntime`, `weight`, `time_slice`, `nice`, and `slice_start` for fairness tracking.

## Reference

The implementation summary in `report.md` explains which xv6 source files were changed and how the syscall and scheduler work were integrated.