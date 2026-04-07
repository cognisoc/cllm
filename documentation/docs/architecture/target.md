# Target Platform

## Base Architecture

The target base architecture is **x86** (i386, Multiboot). All development, optimization, and testing efforts are focused on this platform. The kernel boots as a 32-bit Multiboot ELF in QEMU or on bare-metal x86 hardware.

## x86 Optimizations

- **Instruction Set** -- Leveraging SSE, AVX, AVX2, and AVX-512 where available
- **Memory Model** -- Flat 32-bit addressing with 4 MB heap arena
- **Register Usage** -- Efficient use of the x86 register set
- **Alignment** -- Proper memory alignment for performance

## GPU Backend Integration

- **CUDA Support** -- Optimized for x86 + NVIDIA GPU combinations
- **Memory Mapping** -- Integration with GPU memory via PCIe
- **PCIe Interface** -- Direct PCI bus communication with GPUs
- **NUMA Awareness** -- NUMA node optimization for multi-socket systems (future)

## Compile-Time Targeting

- **x86-Specific Code Generation** -- All compiled code optimized for i386
- **Model Quantization** -- Quantization optimized for x86 ALUs
- **Kernel Optimization** -- Compute kernels optimized for x86 + GPU combinations

## Future Considerations

While the current focus is exclusively on x86, the modular architecture allows for potential future expansion to other platforms:

- **x86_64** -- Full 64-bit support with long mode transition
- **ARM64** -- For edge deployment scenarios
- **RISC-V** -- For specialized hardware deployments
