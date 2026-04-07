# CLLM

**A bare-metal C unikernel for serving large language models -- no OS, no overhead.**

[![Build](https://github.com/Skelf-Research/cllm/actions/workflows/build.yml/badge.svg)](https://github.com/Skelf-Research/cllm/actions)
[![License](https://img.shields.io/github/license/Skelf-Research/cllm)](LICENSE)
![Platform](https://img.shields.io/badge/platform-x86-blue)
![Language](https://img.shields.io/badge/language-C-lightgrey)

## What is CLLM?

CLLM is a Multiboot-compliant unikernel written in C that boots directly on bare metal (or in QEMU) and serves LLM inference over HTTP. It eliminates the operating system layer entirely -- the kernel *is* the application.

The kernel includes a custom libc subset, PCI bus enumeration, an Intel e1000 NIC driver, an HTTP server with REST API endpoints, and a model loading interface compatible with llama.cpp.

## Quick Start

```bash
# Prerequisites: gcc (with -m32 support), make, qemu-system-i386
sudo apt-get install gcc gcc-multilib make qemu-system-x86

# Build and run
git clone git@github.com:Skelf-Research/cllm.git
cd cllm
make run
```

Serial output appears on your terminal. Press `Ctrl-A X` to exit QEMU.

## Make Targets

| Target | Description |
|---|---|
| `make` | Build release kernel (`build/kernel.bin`) |
| `make debug` | Build with debug symbols |
| `make run` | Build and boot in QEMU (serial on stdio) |
| `make run-vga` | Build and boot in QEMU (VGA window) |
| `make run-debug` | Build and boot paused for GDB on `:1234` |
| `make clean` | Remove build artifacts |

## Architecture

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

The kernel boots via Multiboot, initializes serial and VGA output, brings up an e1000 network interface via PCI, and enters a packet-processing loop that serves HTTP requests for LLM inference.

## Project Structure

```
src/            C source files (kernel, drivers, HTTP, LLM)
include/        Header files
build/          Build scripts, linker script, artifacts
docs/           Architecture and design documentation
llama.cpp/      llama.cpp headers for model integration
```

## Roadmap

- [x] Multiboot kernel with VGA + serial output
- [x] Custom libc (malloc, snprintf, string ops)
- [x] PCI enumeration and e1000 NIC driver
- [x] HTTP server with REST API endpoints
- [x] llama.cpp-compatible API (v1 endpoints)
- [ ] Integrate llama.cpp inference engine
- [ ] GPU passthrough (CUDA backend)
- [ ] Streaming token generation
- [ ] vLLM optimizations for transformer serving

## Documentation

- [Architecture Overview](docs/architecture.md)
- [Build Instructions](docs/BUILD_INSTRUCTIONS.md)
- [Project Specification](docs/specs.md)
- [GPU Backend Analysis](docs/gpu_backend_analysis.md)
- [llama.cpp Integration Plan](docs/llama_integration.md)
- [HTTP Server Design](docs/http_server_plan.md)

## License

See [LICENSE](LICENSE) for details.
