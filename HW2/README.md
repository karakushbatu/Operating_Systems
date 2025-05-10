# Online Market Simulation (Homework 2)

This project implements a multi-threaded online shopping simulation in C, using POSIX threads, mutexes, and condition variables to coordinate stock reservations, timeouts, and a limited number of concurrent payment “cashiers.”

---

## Repository Contents

- **studentID_market_sim.c**  
  Final C source implementing:
  - Thread-per-request handling (`pthread_create` / `pthread_join`)
  - Stock reservation with `stock[]` and `reserved_qty[]`
  - Reservation timeout (soft hold, configurable via `reservation_timeout_ms`)
  - “Cashier” limit (max concurrent payments, configurable via `max_concurrent_payments`)
  - Detailed, timestamped logging to `log.txt` in the sample format  

- **Makefile**  
  - `make` builds the simulator  
  - `make run` clears and runs, producing `log.txt`  
  - `make clean` removes binaries and logs  

- **input.txt**  
  Instructor-provided configuration (do *not* modify). Defines:
  - `num_customers` (batch size)
  - `num_products`
  - `reservation_timeout_ms`
  - `max_concurrent_payments`
  - `initial_stock` (comma-separated list)
  - One request per line: `customer_id,product_id,quantity`  

- **log.txt**  
  Sample output from running `make run`, demonstrating all code paths:
  - `// succeed` and `// failed` reservation annotations  
  - `(maximum number of concurrent payments reached!)` waits  
  - `(automatically) retried… //(checked for available cashier slot before timeout expired.)`  
  - `Timeout is expired!!! … returned`  
  - `retry attempt failed … // no more retry for this thread`  

- **README.md**  
  This file.

- **studentID_Fullname_report.pdf**  
  A 2–3 page PDF discussing:
  1. How the payment-slot limit is enforced with mutex + cond vs. semaphores  
  2. Reservation + timeout design and race-condition avoidance  
  3. Thread-termination conditions and cleanup  

---

## Building

Requires GCC (or clang) with POSIX threads support.

```bash
make
```
This produces the executable market_sim.

## Running
```bash
make run
```

## Configuration (input.txt)
The first lines of input.txt define system parameters:
```bash
num_customers=3
num_products=4
reservation_timeout_ms=5000
max_concurrent_payments=2
initial_stock=5,3,4,2
```
num_customers: Number of threads launched concurrently per batch.

num_products: Total distinct products (IDs 0 to num_products–1).

reservation_timeout_ms: How long (in milliseconds) each thread holds a soft reservation before auto-cancel.

max_concurrent_payments: Number of “cashier slots” available at once; extra threads must wait or eventually fail.

initial_stock: Comma-separated initial count for each product.

Following that, each nonblank line with X,Y,Z is a request:

X = customer ID

Y = product ID

Z = quantity

Blank lines delimit batches; each batch of up to num_customers requests starts simultaneously.


## Clean Up
```bash
make clean
```
Removes:

market_sim executable

log.txt

## Notes & Tips

Do not modify input.txt—the grader will test with its own file.

The code uses clock_gettime(CLOCK_REALTIME) for second-resolution timestamps; you can adjust to milliseconds by replacing now_s() with a now_ms() implementation if needed.

All critical sections (stock updates, log writes, payment-slot counters) are protected by mutexes to avoid race conditions.

Condition variables (pthread_cond_timedwait) implement timed waits for payment slots without busy-waiting.






