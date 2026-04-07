# Getting Started

## Prerequisites

- GCC with 32-bit support (`gcc -m32`)
- GNU linker (`ld`)
- QEMU (`qemu-system-i386`)
- make

On Debian/Ubuntu:

```bash
sudo apt-get install gcc gcc-multilib make qemu-system-x86
```

## Building

```bash
make            # release build
make debug      # debug build with symbols
make clean      # remove build artifacts
```

The build produces `build/kernel.bin`, a 32-bit Multiboot ELF kernel.

## Running in QEMU

```bash
make run        # serial output on terminal (headless)
make run-vga    # opens QEMU window with VGA display
make run-debug  # paused for GDB on port 1234
```

Or manually:

```bash
qemu-system-i386 -kernel build/kernel.bin -serial stdio -display none -no-reboot -m 128M
```

## Expected Output

On boot, serial output shows kernel initialization:

```
BOOT
KERNEL: Serial port initialized
KERNEL: Memory management system initialized
KERNEL: VGA initialized
KERNEL: Hello from our custom unikernel!
KERNEL: Network initialized successfully
KERNEL: All tests completed. Entering network loop...
```

## Key Components

| Component | Source | Description |
|---|---|---|
| Bootloader | `src/boot.S` | Multiboot-compliant entry point |
| Kernel | `src/kernel.c` | VGA, serial, initialization |
| String/Memory | `src/string.c`, `src/memory.c` | libc subset |
| HTTP Server | `src/http.c`, `src/api.c` | REST API framework |
| Network | `src/network.c`, `src/network/` | PCI + e1000 NIC driver |
| LLM Interface | `src/llm.c`, `src/c_model_interface.c` | Model loading |

## Debugging

Attach GDB to a running debug kernel:

```bash
make run-debug &
gdb build/kernel.bin -ex "target remote :1234"
```
