# CLLM

**A bare-metal C unikernel for serving large language models -- no OS, no overhead.**

[![Build](https://github.com/cognisoc/cllm/actions/workflows/build.yml/badge.svg)](https://github.com/cognisoc/cllm/actions)
[![License](https://img.shields.io/github/license/cognisoc/cllm)](LICENSE)
![Platform](https://img.shields.io/badge/platform-x86-blue)
![Language](https://img.shields.io/badge/language-C-lightgrey)

**[Website](https://cllm.cognisoc.com)** · **[Docs](https://docs.cognisoc.com/cllm/)** · **[GitHub](https://github.com/cognisoc/cllm)**

Where a typical inference server runs inside a container on top of a Linux userland and kernel, CLLM removes the OS, the container, and the userland entirely -- it boots as a Multiboot unikernel straight into an HTTP inference server on bare x86, with its own libc subset, e1000 NIC driver, and REST API, for a minimal attack surface and no OS overhead.

## What is CLLM?

CLLM is a Multiboot-compliant unikernel written in C that boots directly on bare metal (or in QEMU) and serves LLM inference over HTTP. It eliminates the operating system layer entirely -- the kernel *is* the application.

The kernel includes a custom libc subset, PCI bus enumeration, an Intel e1000 NIC driver, an HTTP server with REST API endpoints, and a model loading interface compatible with llama.cpp.

## Quick Start

```bash
# Prerequisites: gcc (with -m32 support), make, qemu-system-i386
sudo apt-get install gcc gcc-multilib make qemu-system-x86

# Build and run tests
git clone git@github.com:cognisoc/cllm.git
cd cllm
make
make test
make run        # requires QEMU
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
| `make test` | Run host-side unit tests |
| `make test-llama` | Build llama.cpp and run host API test |
| `make clean` | Remove build artifacts |

## Architecture

```
+-----------------------------------------------------------+
|  QEMU / Bare Metal  (x86, Multiboot)                      |
+-----------------------------------------------------------+
|  boot.S             Multiboot entry, stack, serial init   |
|  kernel.c           Kernel main, VGA terminal, serial I/O |
|  allocator.c        Heap allocator (malloc/free)          |
|  memory.c           Memory management initialization      |
|  string.c           libc subset (snprintf, memcpy, ...)   |
|  network.c          PCI enumeration + e1000 NIC driver      |
|  http.c / api.c     HTTP request parser and routing       |
|  api_v1.c           OpenAI-compatible REST API skeleton   |
|  llm.c              Model loading and inference skeleton  |
+-----------------------------------------------------------+
```

The kernel boots via Multiboot, initializes serial and VGA output, brings up an e1000 network interface via PCI, and enters a packet-processing loop that serves HTTP requests for LLM inference.

## Project Structure

```
src/            C source files (kernel, drivers, HTTP, LLM)
include/        Header files
build/          Build scripts, linker script, artifacts
tests/          Host-side unit tests and integration helpers
documentation/  MkDocs documentation site
llama.cpp/      llama.cpp upstream submodule
```

## Current Status

The project is in **Phase 1**. The kernel builds cleanly, boots in QEMU, has a working heap allocator, a minimal TCP/IP stack serving HTTP requests, and upstream llama.cpp added as a pinned submodule with a host-side API test. The following capabilities are still placeholders and are tracked in the production-readiness plan:

- Real LLM inference linked into the kernel binary (requires C++ runtime and OS stubs)
- GPU acceleration (CUDA/Vulkan/Metal)
- Streaming token generation

## Roadmap

- [x] Multiboot kernel with VGA + serial output
- [x] Buildable C unikernel with zero warnings
- [x] Working heap allocator (`malloc`/`free`) with coalescing
- [x] Host-side unit tests and CI smoke test
- [x] llama.cpp-compatible API skeleton (v1 endpoints)
- [x] TCP/IP networking and HTTP server over sockets
- [x] llama.cpp added as pinned upstream submodule with host API test
- [ ] Link llama.cpp inference into the kernel binary (requires C++ runtime + stubs)
- [ ] GPU passthrough (CUDA backend)
- [ ] Streaming token generation
- [ ] vLLM optimizations for transformer serving

## Documentation

- [Architecture Overview](documentation/docs/architecture/overview.md)
- [Getting Started](documentation/docs/getting-started.md)
- [Project Specification](documentation/docs/specification.md)
- [GPU Backend Analysis](documentation/docs/architecture/gpu-backend.md)
- [llama.cpp Integration](documentation/docs/design/llama-integration.md)
- [HTTP Server Design](documentation/docs/design/http-server.md)

## License

See [LICENSE](LICENSE) for details.

---

## Part of the Cognisoc stack

**[Cognisoc](https://www.cognisoc.com)** builds open-source LLM inference for every language and every device — *LLM inference, everywhere.* This project is one of six:

| Project | Language | What it does |
|---|---|---|
| [mullama](https://github.com/cognisoc/mullama) | Python · Node · Go · PHP · Rust · C | Local LLM runtime & server, drop-in Ollama alternative |
| [unillm](https://github.com/cognisoc/unillm) | Rust | Modular inference runtime, 47 architectures |
| [llamafu](https://github.com/cognisoc/llamafu) | Dart / Flutter | On-device inference for mobile apps |
| [llmdot](https://github.com/cognisoc/llmdot) | C# / .NET | Local GGUF inference for the .NET ecosystem |
| **cllm** **(this project)** | C | Bare-metal unikernel — boots straight into inference |
| [zigllm](https://github.com/cognisoc/zigllm) | Zig | Learn LLMs by building one, from tensors to text |

🌐 [cognisoc.com](https://www.cognisoc.com) · 📚 [docs.cognisoc.com](https://docs.cognisoc.com) · 🐙 [github.com/cognisoc](https://github.com/cognisoc)

---
