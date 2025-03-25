# Priority-based Preemptive Scheduler

## Overview
This project implements a **preemptive priority-based process scheduler** using `fork()`, `exec()`, and process control mechanisms (`SIGSTOP`, `SIGCONT`). The scheduler:

- Implements a Round Robin algorithm with priority.
- Reads jobs from an external `jobs.txt` file.
- Preempts and resumes processes based on their time slice.
- Logs all operations in `scheduler.log`.

## Files

- `priority_scheduler.c`: The main implementation of the scheduler.
- `jobs.txt`: Input file defining time slice and jobs.
- `scheduler.log`: Log file recording all process state changes.
- `Makefile`: Build script to compile the scheduler.

## Prerequisites
- A Linux-based environment.
- Docker installed (optional, if running inside a container).

## Compilation
To compile the scheduler, run:

```bash
make
```

This will generate an executable `scheduler`.

## Input File (jobs.txt)

The `jobs.txt` file must be formatted as follows:

```
TimeSlice <time_slice>
<job_name> <arrival_time> <priority_level> <execution_time>
```

Example:

```
TimeSlice 3
jobA 0 1 6
jobB 2 2 9
jobC 4 1 4
```

## Running the Scheduler

1. Ensure `jobs.txt` is correctly formatted.
2. Run the scheduler with:

```bash
./scheduler
```

## Log File (scheduler.log)

The scheduler logs every event with timestamps. Example log output:

```
[2025-03-23 12:00:01] [INFO] Forking new process for jobA (PID: 1234)
[2025-03-23 12:00:04] [INFO] JobA ran for 3 seconds. Time slice expired - Sending SIGSTOP
[2025-03-23 12:00:07] [INFO] Resuming jobA (PID: 1234) - SIGCONT
[2025-03-23 12:00:10] [INFO] JobA completed execution. Terminating (PID: 1234)
```

## How It Works

1. **Initialization:**
   - Read job specifications and time slice from `jobs.txt`.
   - Fork a child process for each job and execute it.

2. **Scheduling Logic:**
   - If a job completes within its time slice, terminate it.
   - If not, pause the process (`SIGSTOP`) and select the next job.
   - Resume previously paused jobs using `SIGCONT`.

3. **Job Selection:**
   - Highest priority (lower value = higher priority) is chosen.
   - If priorities match, the job with the earliest arrival time is selected.
   - If ties persist, the job with the shortest remaining time runs.

## Edge Cases Handled

- Processes that do not respond are forcefully stopped.
- Handles simultaneous arrivals by priority and position.
- Ensures no starvation by rotating processes in Round Robin order.

## Cleanup

To remove generated files:

```bash
make clean
```

## Notes

- Ensure all job binaries (e.g., `jobA`, `jobB`, `jobC`) are executable and present in the working directory.
- Requires superuser privileges for process control signals.

