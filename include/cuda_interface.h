#ifndef CUDA_INTERFACE_H
#define CUDA_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

// CUDA unikernel context
typedef struct {
    int device_id;
    void* gpu_memory_base;
    size_t gpu_memory_size;
    void* gpu_context;
    int is_initialized;
} cuda_unikernel_context_t;

// Dimension structure for CUDA kernels
typedef struct {
    uint32_t x, y, z;
} dim3;

// Error codes
typedef enum {
    CUDA_UK_SUCCESS = 0,
    CUDA_UK_ERROR_NOT_SUPPORTED = 1,
    CUDA_UK_ERROR_OUT_OF_MEMORY = 2,
    CUDA_UK_ERROR_INVALID_VALUE = 3,
    CUDA_UK_ERROR_INITIALIZATION_FAILED = 4
} cuda_uk_error_t;

// Initialize CUDA in unikernel environment
cuda_uk_error_t cuda_uk_init(cuda_unikernel_context_t* ctx, int device_id);

// Memory management
void* cuda_uk_malloc(cuda_unikernel_context_t* ctx, size_t size);
void cuda_uk_free(cuda_unikernel_context_t* ctx, void* ptr);
cuda_uk_error_t cuda_uk_memcpy_htod(cuda_unikernel_context_t* ctx, void* dst, const void* src, size_t size);
cuda_uk_error_t cuda_uk_memcpy_dtoh(cuda_unikernel_context_t* ctx, void* dst, const void* src, size_t size);

// Kernel execution
cuda_uk_error_t cuda_uk_launch_kernel(cuda_unikernel_context_t* ctx, 
                                     const char* kernel_name,
                                     dim3 grid_dim, dim3 block_dim,
                                     void** args, size_t args_size);

// Device management
int cuda_uk_get_device_count(void);
void cuda_uk_get_device_description(int device, char* description, size_t description_size);

// Check if CUDA is available
int cuda_uk_is_available(void);

#endif // CUDA_INTERFACE_H