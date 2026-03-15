/*
 * c_bindings.h - C bindings for Zig components
 */

#ifndef C_BINDINGS_H
#define C_BINDINGS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct zig_http_server zig_http_server_t;
typedef struct zig_model_interface zig_model_interface_t;
typedef struct zig_memory_pool zig_memory_pool_t;

// Callback function type
typedef void (*callback_t)(const char* data, int len, void* user_data);

// Memory management functions
void* zig_malloc(size_t size);
void zig_free(void* ptr);
void* zig_realloc(void* ptr, size_t size);
size_t zig_get_memory_usage(void);

// Memory pool functions
zig_memory_pool_t* zig_memory_pool_create(size_t block_size, size_t num_blocks);
void zig_memory_pool_destroy(zig_memory_pool_t* pool);
void* zig_memory_pool_alloc(zig_memory_pool_t* pool, size_t size);
void zig_memory_pool_free(zig_memory_pool_t* pool, void* ptr);

// HTTP server functions
zig_http_server_t* zig_http_server_create(const char* host, uint16_t port);
void zig_http_server_destroy(zig_http_server_t* server);
int zig_http_server_start(zig_http_server_t* server);
void zig_http_server_stop(zig_http_server_t* server);
void zig_http_server_set_callback(zig_http_server_t* server, callback_t callback, void* user_data);

// Model interface functions
zig_model_interface_t* zig_model_interface_create(void);
void zig_model_interface_destroy(zig_model_interface_t* interface);
int zig_model_interface_load_model(zig_model_interface_t* interface, const char* model_path);
int zig_model_interface_is_ready(zig_model_interface_t* interface);

// Completion request structure
typedef struct {
    const char* prompt;
    size_t prompt_len;
    float temperature;
    int max_tokens;
    int stream;
} completion_request_t;

// Completion response structure
typedef struct {
    const char* content;
    size_t content_len;
    int has_more;
    const char* error;
} completion_response_t;

// Completion functions
completion_response_t* zig_model_interface_complete(
    zig_model_interface_t* interface, 
    const completion_request_t* request
);

void zig_completion_response_free(completion_response_t* response);

// Callback function type for streaming responses
typedef void (*stream_callback_t)(const char* chunk, size_t chunk_len, void* user_data);

// Streaming completion functions
int zig_model_interface_complete_stream(
    zig_model_interface_t* interface,
    const completion_request_t* request,
    stream_callback_t callback,
    void* user_data
);

#ifdef __cplusplus
}
#endif

#endif // C_BINDINGS_H