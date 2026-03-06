# CortexServer — Multithreaded HTTP Server in C

CortexServer is a lightweight multithreaded HTTP server built entirely in C using low-level sockets and POSIX threads.  
The project focuses on systems programming, concurrency, and backend performance engineering without relying on external frameworks.

It was built to better understand how real web servers handle connections, distribute work across threads, and maintain performance under load.

---

## Features

- Multithreaded worker pool for handling concurrent client requests
- Custom HTTP request parsing pipeline
- Thread-safe request metrics and logging
- Health check and metrics endpoints
- Load-testing scripts for performance benchmarking
- Clean modular structure for extensibility

---

## Tech Stack

C • POSIX Sockets • Pthreads • Makefile • Bash • macOS/Linux CLI

---

## Project Structure
