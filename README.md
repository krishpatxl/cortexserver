# CortexServer — Multithreaded HTTP Server in C

CortexServer is a lightweight multithreaded HTTP server built entirely in C using low-level sockets and POSIX threads.

The project focuses on systems programming, concurrency, backend performance, and secure file handling — without relying on external web frameworks.

It was built to understand how real web servers accept connections, distribute work across threads, parse HTTP requests, and maintain performance under load.

---

## Features

- Multithreaded worker pool for concurrent client requests
- Custom HTTP request parsing pipeline
- Static file serving (HTML / CSS assets)
- Basic MIME type handling
- Thread-safe metrics endpoint
- Health check endpoint
- Path traversal protection for secure file access
- Thread-safe request metrics and logging
- Built-in load testing script for performance benchmarking
- Modular architecture for extensibility

---

## Tech Stack

C • POSIX Sockets • Pthreads • Makefile • Bash • macOS/Linux CLI

---

## Project Structure

cortexserver/  
├── include/        Header files  
├── src/            Server source code  
├── public/         Static files (HTML/CSS)  
├── bench/          Load testing tools  
├── tests/          Unit tests  
├── Makefile  
└── README.md  

---

## Build & Run

### Compile
```bash
make clean
make
```

### Start Server
```bash
./cortexserver
```

Server runs at:

```
http://localhost:8080
```

---

## Endpoints

### Health Check
```
GET /health
```
Response:
```
ok
```

### Metrics
```
GET /metrics
```
Shows total requests handled by the server.

### Static Files
```
GET /
GET /static/style.css
```

Serves files from the `/public` directory.

---

## Security Test

Path traversal protection prevents access outside the public directory:

```bash
curl --path-as-is http://localhost:8080/static/../../secret.txt
```

Expected response:

```
invalid path
```

---

## Performance

Tested locally on macOS with a 4-thread worker pool.

### Example Benchmark

```bash
./bench/load.sh http://localhost:8080/health 2000 100
```

### Sample Result

- 200 OK responses: 2000  
- Runtime: ~6 seconds  
- Concurrency level: 100 requests  

---

## Load Testing

Custom Bash benchmarking tool located in:

```
bench/load.sh
```

Usage:

```bash
./bench/load.sh <url> <requests> <concurrency>
```

Example:

```bash
./bench/load.sh http://localhost:8080/health 1000 50
```

---
