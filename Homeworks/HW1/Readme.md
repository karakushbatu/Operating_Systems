# Scheduler Assignment (BLG312E - Homework 1)

## Overview
This assignment implements a preemptive, priority-based process scheduler using a variant of round-robin scheduling. The scheduler simulates process scheduling by reading job details from an external file (`jobs.txt`), creating child processes using `fork()` and `exec()`, and managing process execution with `SIGSTOP` and `SIGCONT` signals.

The entire functionality is encapsulated in a single C source file (`scheduler.c`), which supports two modes:
- **Scheduler Mode**: When executed without the "job" argument, the program reads `jobs.txt` and schedules the jobs.
- **Job Mode**: When executed with the "job" argument (e.g., `./scheduler job jobA 6`), the program simulates the execution of that specific job.

## Files Included
- **scheduler.c**:  
  Contains the complete implementation of both the scheduler and job simulation. In scheduler mode, it reads job details, manages process control, and logs events. In job mode, it simulates a job’s execution.
  
- **jobs.txt**:  
  The input file with the following format:
  TimeSlice 3
   jobA 0 1 6
   jobB 2 2 9
   jobC 4 1 4
- **scheduler.log**:  
   Generated during execution, this file logs all process state changes (fork, exec, SIGSTOP, SIGCONT, termination) along with timestamps.

- **makefile**:  
   Contains the build instructions to compile the project.

- **readme.md**:  
   This file.

- **report.pdf**:  
   A detailed report covering design decisions, scheduling fairness analysis, and discussion of edge cases and failure scenarios.

- **Screen Recording**:  
   A video demonstrating the scheduler's execution in a terminal (to be included in the submission package).

## Compilation and Execution

### Compilation
Ensure you have GCC and Make installed. Open a terminal in the project directory and run:
```bash
make
```
This compiles priority_scheduler.c and produces the executable scheduler.

### Running the Scheduler
To run the scheduler, execute:
```bash
./scheduler
```
The scheduler will:
	•	Read the job details from jobs.txt
	•	Create child processes for each job using fork() and exec()
	•	Manage process execution via SIGSTOP and SIGCONT
	•	Log all events with timestamps in scheduler.log

### Running in Job Mode
To test the job simulation mode independently, run:
```bash
./scheduler job <job_name> <execution_time>
```
For example, to simulate jobA with a total execution time of 6 seconds:
```bash
./scheduler job jobA 6
```
## How it Works
### Scheduler Mode:
	•	Reads jobs.txt to retrieve the time slice and job information.
	•	Maintains a list of jobs including details such as arrival time, priority, total and remaining execution time, process ID, and input order.
	•	Simulates time progression and, at each time slice, selects the next job based on:
	•	Priority (lower numeric value = higher priority)
	•	Arrival Time (earlier arriving jobs are favored)
	•	Remaining Execution Time (smaller remaining time is prioritized)
	•	Input Order (if all other criteria are equal)
	•	If a job’s remaining execution time is less than or equal to the time slice, it is allowed to complete without preemption. Otherwise, the job is preempted with SIGSTOP after the time slice and resumed later using SIGCONT.
### Job Mode:
	•	Simulates the execution of a single job by decrementing its execution time each second.
	•	Supports signal-based pausing (SIGSTOP) and resumption (SIGCONT), mimicking the behavior of preemptive scheduling.
### Logging:
	•	Every significant event (fork, exec, time slice expiration with SIGSTOP, resume with SIGCONT, and termination) is logged with a timestamp in scheduler.log.
	•	This log file provides a detailed trace of the scheduler’s operation for verification and debugging.

## Dependencies
	•	GCC: GNU Compiler Collection for compiling the C source code.
	•	Unix-like Operating System: The scheduler is designed to run on systems such as Linux or macOS.
	•	Make: For building the project using the provided makefile.
	•	Docker (Optional): The scheduler is designed to run within a Docker container if required by the assignment.

## Additional Notes
	•	The scheduler utilizes process control signals (SIGSTOP and SIGCONT) to manage job execution effectively. Completed jobs are terminated immediately without being resumed.
	•	A screen recording demonstrating the scheduler’s execution is included in the submission package.
	•	For any questions or further clarifications, please refer to the detailed report or contact the course instructor.

Happy scheduling and coding!
