// c_model_interface.h - Enhanced C interface for accessing embedded models with integrity verification
#ifndef C_MODEL_INTERFACE_H
#define C_MODEL_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Model info structure
typedef struct {
    const uint8_t* data;
    size_t size;
    int is_valid;
} embedded_model_info_t;

// Model metadata structure
typedef struct {
    char magic[4];
    uint32_t version;
    uint32_t tensor_count;
    uint32_t kv_count;
    char architecture[32];
    uint32_t context_length;
} model_metadata_t;

// Model interface functions
const uint8_t* c_get_embedded_model(void);
size_t c_get_embedded_model_size(void);
int c_validate_embedded_model(void);

// Model loading functions
embedded_model_info_t c_load_embedded_model(void);

// Model validation functions
int c_validate_gguf_format(const uint8_t* data, size_t size);
int c_validate_header(const uint8_t* data, size_t size);

// Model metadata functions
model_metadata_t c_get_model_metadata(void);

// Enhanced integrity verification functions
int c_verify_model_integrity(const uint8_t* data, size_t size);
uint32_t c_calculate_model_checksum(const uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif // C_MODEL_INTERFACE_H