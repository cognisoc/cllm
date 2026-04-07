# CLLM

**A bare-metal C unikernel for serving large language models -- no OS, no overhead.**

CLLM is a Multiboot-compliant unikernel written in C that boots directly on bare metal (or in QEMU) and serves LLM inference over HTTP. It eliminates the operating system layer entirely -- the kernel *is* the application.

## Features

- **Bare-metal boot** via Multiboot on x86 hardware or QEMU
- **Custom libc** subset (malloc, snprintf, string ops)
- **PCI bus enumeration** and Intel e1000 NIC driver
- **HTTP server** with REST API endpoints
- **llama.cpp-compatible API** (v1 endpoints)
- **Model embedding** -- bake models directly into the kernel binary

## Quick Start

### Prerequisites

- GCC with 32-bit support (`gcc -m32`)
- GNU linker (`ld`)
- QEMU (`qemu-system-i386`)
- make

On Debian/Ubuntu:

```bash
sudo apt-get install gcc gcc-multilib make qemu-system-x86
```

### Build and Run

```bash
git clone git@github.com:cognisoc/cllm.git
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

## Project Structure

```
src/            C source files (kernel, drivers, HTTP, LLM)
include/        Header files
build/          Build scripts, linker script, artifacts
documentation/  MkDocs documentation site (this site)
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
