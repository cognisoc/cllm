# Build Instructions

## Prerequisites

- GCC with 32-bit and 64-bit support
- Zig compiler (optional, for enhanced features)
- QEMU (for testing)
- Standard build tools (make, ld, etc.)

## Building the Kernel

### Quick Build
```bash
cd build
./build.sh
```

This will create `kernel.bin` in the build directory.

### 32-bit Build
```bash
cd build
./build32.sh
```

This will create `kernel32.bin` in the build directory.

### Simple Test Kernel
```bash
cd build
./build_simple.sh
```

This will create `simple_kernel.bin` in the build directory.

## Testing

### Running in QEMU
```bash
# For 32-bit kernel
qemu-system-i386 -kernel kernel32.bin

# For 64-bit kernel (if supported)
qemu-system-x86_64 -kernel kernel.bin
```

Note: Due to bootloader implementation issues, the kernel may not produce visible output in QEMU.

## Project Structure

- `src/` - Source code (C and Zig)
- `include/` - Header files
- `build/` - Build scripts and output
- `docs/` - Documentation

## Key Components

1. **Bootloader** (`boot.S`) - Multiboot-compliant bootloader
2. **Kernel** (`kernel.c`) - Main kernel implementation
3. **String Utilities** (`string.c`) - Custom string functions
4. **HTTP Server** (`http.c`) - HTTP request/response handling
5. **API Handlers** (`api.c`) - REST API endpoints
6. **Model Interface** (`c_model_interface.c`) - Model loading and validation

## Troubleshooting

1. If you see "undefined reference to `strlen`" errors, make sure all object files are rebuilt
2. If the kernel hangs in QEMU, check the bootloader implementation
3. For 64-bit builds, ensure your system supports 64-bit compilation

## Future Enhancements

1. Add actual LLM model loading and inference
2. Implement proper memory management
3. Add network stack for remote model serving
4. Integrate with real GGUF model files
5. Add support for streaming responses
6. Implement model validation with actual checksums