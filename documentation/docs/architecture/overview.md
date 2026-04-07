# Architecture Overview

This document provides a high-level overview of the CLLM unikernel architecture.

## Components

```
+-----------------------------------------------------------+
|  QEMU / Bare Metal  (x86, Multiboot)                     |
+-----------------------------------------------------------+
|  boot.S             Multiboot entry, stack, serial init   |
|  kernel.c           Kernel main, VGA terminal, serial I/O |
|  memory.c           Heap allocator (malloc/free)          |
|  string.c           libc subset (snprintf, memcpy, ...)   |
|  network.c          PCI enumeration + e1000 NIC driver    |
|  http.c / api.c     HTTP server, request routing          |
|  api_v1.c           llama.cpp-compatible REST API         |
|  llm.c              Model loading and inference interface |
+-----------------------------------------------------------+
```

1. **Unikernel Core** -- Minimal operating system services (boot, memory, I/O)
2. **LLM Engine** -- Integration of llama.cpp for model inference
3. **HTTP Server** -- REST API for model interaction
4. **Configuration System** -- JSON-based build and runtime configuration
5. **GPU Support** -- CUDA and other backend integrations (planned)

## Data Flow

1. Configuration is loaded at build time or startup
2. Models are loaded into memory (or baked into the kernel binary)
3. HTTP requests are processed by the server
4. LLM engine performs inference
5. Results are returned via HTTP responses

## Memory Management

The unikernel implements a custom heap allocator optimized for LLM workloads, providing `malloc` and `free` within a 4 MB statically-allocated arena.
