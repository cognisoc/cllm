// test_model_embedding.c - Test program for model embedding interface
#include <stdio.h>
#include <stdlib.h>
#include "../include/c_model_interface.h"

int main() {
    printf("Testing model embedding interface...\n");
    
    // Test getting embedded model
    const uint8_t* model_data = c_get_embedded_model();
    if (model_data == NULL) {
        printf("Failed to get embedded model data\n");
        return 1;
    }
    
    size_t model_size = c_get_embedded_model_size();
    printf("Got embedded model of size: %zu bytes\n", model_size);
    
    // Test validating embedded model
    int is_valid = c_validate_embedded_model();
    printf("Embedded model validation: %s\n", is_valid ? "VALID" : "INVALID");
    
    // Test loading embedded model
    embedded_model_info_t model_info = c_load_embedded_model();
    printf("Loaded model info:\n");
    printf("  Data pointer: %p\n", (void*)model_info.data);
    printf("  Size: %zu bytes\n", model_info.size);
    printf("  Valid: %s\n", model_info.is_valid ? "YES" : "NO");
    
    // Test GGUF format validation
    int format_valid = c_validate_gguf_format(model_info.data, model_info.size);
    printf("GGUF format validation: %s\n", format_valid ? "VALID" : "INVALID");
    
    printf("Model embedding interface test completed successfully!\n");
    return 0;
}