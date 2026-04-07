# GPU Backend Integration

## Overview

This document analyzes how llama.cpp handles GPU backends, particularly CUDA, and outlines how we can integrate this capability into the unikernel.

## llama.cpp GPU Backend Architecture

### Backend Interface

llama.cpp uses ggml as its computational backend, which provides a unified interface for multiple hardware backends:

```c
// ggml-backend.h - Backend interface
typedef struct ggml_backend * ggml_backend_t;
typedef struct ggml_backend_buffer_type * ggml_backend_buffer_type_t;

// CUDA backend initialization
ggml_backend_t ggml_backend_cuda_init(int device);
bool ggml_backend_is_cuda(ggml_backend_t backend);

// Buffer management
ggml_backend_buffer_type_t ggml_backend_cuda_buffer_type(int device);
```

### Device Management

```c
int ggml_backend_cuda_get_device_count(void);
void ggml_backend_cuda_get_device_description(int device, char * description, size_t description_size);
void ggml_backend_cuda_get_device_memory(int device, size_t * free, size_t * total);
```

### Memory Management

```c
ggml_backend_buffer_type_t ggml_backend_cuda_host_buffer_type(void);
bool ggml_backend_cuda_register_host_buffer(void * buffer, size_t size);
void ggml_backend_cuda_unregister_host_buffer(void * buffer);
```

## CUDA Implementation Details

### Kernel Organization

The CUDA backend is organized into specialized kernels:

1. **Matrix Multiplication** -- Optimized GEMM operations (MMQ, MMF)
2. **Attention Operations** -- FlashAttention implementations
3. **Normalization** -- Layer norm, RMS norm
4. **Activation Functions** -- Various activation functions
5. **Quantization** -- Conversion between quantization formats
6. **Memory Operations** -- Copy, transpose, etc.

### Quantization Support

CUDA backend supports multiple quantization formats:

- Q4_0, Q4_1, Q5_0, Q5_1, Q8_0
- IQ2_XXS, IQ2_XS, IQ3_XXS
- F16, F32 (full precision)

### Architecture Support

| GPU Architecture | Compute Capability | Key Features |
|---|---|---|
| Maxwell | 50 | Basic CUDA support |
| Pascal | 60, 61 | FP16 intrinsics |
| Volta | 70 | Tensor cores |
| Turing | 75 | Integer tensor cores |
| Ampere | 80, 86 | Improved tensor cores, async ops |
| Ada Lovelace | 89 | Latest architecture |

## Integration Approach

### Challenges in Unikernel Environment

**Hardware Access:**

- Unikernels don't have direct access to GPU hardware via standard drivers
- Need to implement or interface with GPU drivers directly
- Specialized GPU memory allocation required
- GPU interrupt processing in unikernel context

**System Call Limitations:**

- No standard syscalls available for GPU driver interfaces
- Need custom minimal GPU driver interface
- Direct GPU memory mapping in unikernel address space

### Phase 1: Driver Interface Layer

```c
typedef struct {
    int device_id;
    void* gpu_memory_base;
    size_t gpu_memory_size;
    void* gpu_context;
} cuda_unikernel_context_t;

int cuda_uk_init(cuda_unikernel_context_t* ctx, int device_id);
void* cuda_uk_malloc(cuda_unikernel_context_t* ctx, size_t size);
void cuda_uk_free(cuda_unikernel_context_t* ctx, void* ptr);
int cuda_uk_memcpy_htod(cuda_unikernel_context_t* ctx, void* dst, const void* src, size_t size);
int cuda_uk_memcpy_dtoh(cuda_unikernel_context_t* ctx, void* dst, const void* src, size_t size);
```

### Phase 2: ggml Backend Integration

```c
ggml_backend_t ggml_backend_cuda_uk_init(cuda_unikernel_context_t* ctx);
ggml_backend_buffer_type_t ggml_backend_cuda_uk_buffer_type(cuda_unikernel_context_t* ctx);
bool ggml_backend_cuda_uk_compute(ggml_backend_t backend, struct ggml_tensor * tensor);
```

### Phase 3: llama.cpp Integration

```c
struct llama_context_params {
    // ... existing parameters ...
    cuda_unikernel_context_t* cuda_context;
    bool use_cuda_uk;
};
```

## Memory Management

### Unified Memory Model

```c
typedef enum {
    UK_MEM_HOST,      // Host memory
    UK_MEM_DEVICE,    // GPU device memory
    UK_MEM_MANAGED    // Unified memory (if supported)
} uk_memory_type_t;

typedef struct {
    void* ptr;
    size_t size;
    uk_memory_type_t type;
    cuda_unikernel_context_t* ctx;
} uk_memory_allocation_t;
```

### Kernel Compilation

Pre-compiled CUDA kernels are embedded in the unikernel binary:

```c
extern const unsigned char cuda_kernels_ptx[];
extern const size_t cuda_kernels_ptx_size;

static uk_cuda_kernel_t uk_cuda_kernels[] = {
    {"matmul_q4_0_f32", matmul_q4_0_f32_ptx, sizeof(matmul_q4_0_f32_ptx)},
    {"flash_attn_f16", flash_attn_f16_ptx, sizeof(flash_attn_f16_ptx)},
};
```

## Performance Considerations

- **Zero-Copy Operations** -- Minimize CPU-GPU memory transfers
- **Pinned Memory** -- Use pinned memory for faster transfers
- **Continuous Batching** -- Dynamically batch requests for GPU utilization
- **Kernel Fusion** -- Combine multiple operations in single kernels
- **Stream Management** -- Use CUDA streams for overlapping operations

## Implementation Roadmap

| Phase | Timeline | Focus |
|---|---|---|
| 1 | Months 1-2 | Basic driver interface, memory management |
| 2 | Months 3-4 | ggml backend integration, core kernels |
| 3 | Months 5-6 | llama.cpp integration, model loading |
| 4 | Months 7-8 | Optimization, benchmarking |
