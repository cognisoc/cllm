#ifndef MODEL_EMBEDDING_H
#define MODEL_EMBEDDING_H

#include <stddef.h>
#include <stdint.h>

// Model embedding information structure
typedef struct {
    const uint8_t* data;
    size_t size;
    int is_valid;
    uint32_t checksum;
} model_embedding_info_t;

// Function declarations
model_embedding_info_t load_embedded_model(void);
int validate_embedded_model(const model_embedding_info_t* model_info);
uint32_t calculate_model_checksum(const uint8_t* data, size_t size);
int verify_model_integrity(const model_embedding_info_t* model_info);
void print_model_info(const model_embedding_info_t* model_info);

// Model embedding functions
int embed_model_from_file(const char* file_path);
int embed_model_data(const uint8_t* model_data, size_t size);
int generate_model_embedding_code(const uint8_t* model_data, size_t size, const char* output_path);
int verify_embedded_model(const uint8_t* model_data, size_t size);

#endif // MODEL_EMBEDDING_H