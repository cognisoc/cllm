# llama.cpp Integration

## Overview

To integrate llama.cpp into the unikernel, we extract its core components and replace system dependencies with unikernel-native implementations.

### llama.cpp Structure

1. **Core Library** -- The main `llama` library with a C-style interface in `include/llama.h`
2. **Dependencies:**
    - ggml library (core computation)
    - Standard C/C++ libraries
    - Optional: BLAS libraries for acceleration
    - Optional: CUDA libraries for GPU support
3. **Build System** -- Uses CMake (not applicable in our unikernel build)

## Integration Approach

### 1. Extract Core Components

Include the essential parts of llama.cpp and ggml. Remove or replace system dependencies that aren't available in the unikernel environment.

### 2. Memory Management

Replace standard `malloc`/`free` with the unikernel's custom heap allocator. All memory allocations are handled within the unikernel's 4 MB arena.

### 3. I/O Handling

- Replace file I/O with direct memory loading of model data (models are baked into the kernel binary)
- Replace console output with serial/VGA terminal output functions

### 4. Threading

Either implement threading support in the unikernel or use single-threaded mode for initial integration.

## Key Components

| Component | Header | Purpose |
|---|---|---|
| llama library | `llama.h` | Main inference interface |
| ggml library | `ggml.h` | Core computation |
| Model loading | -- | GGUF model format handling |

## Challenges

1. **System Dependencies** -- Many standard library functions are not available
2. **Memory Constraints** -- LLMs require significant memory; the unikernel heap is currently 4 MB
3. **Performance** -- Need to optimize for the specific hardware target
4. **GPU Support** -- Phase 3 will add CUDA support (see [GPU Backend](../architecture/gpu-backend.md))

## Implementation Steps

1. **Minimal Build** -- Extract only the essential components from llama.cpp
2. **Replace System Dependencies** -- Create wrappers for required system functions
3. **HTTP Integration** -- Wire up API endpoints to model inference
4. **Optimize** -- Remove unnecessary code and tune for the unikernel environment
