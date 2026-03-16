// simple_model_test.c - Simple test for model embedding
#include <stdio.h>
#include <stdlib.h>
#include "../include/c_model_interface.h"

int main() {
    printf("=== Simple Model Embedding Test ===\n");
    
    // Test getting embedded model info
    printf("1. Testing embedded model access...\n");
    const uint8_t* model_data = c_get_embedded_model();
    size_t model_size = c_get_embedded_model_size();
    
    printf("   Model data pointer: %p\n", (void*)model_data);
    printf("   Model size: %zu bytes\n", model_size);
    
    // Test model validation
    printf("2. Testing model validation...\n");
    int is_valid = c_validate_embedded_model();
    printf("   Model validation result: %s\n", is_valid ? "VALID" : "INVALID");
    
    // Test loading model info
    printf("3. Testing model loading...\n");
    embedded_model_info_t model_info = c_load_embedded_model();
    printf("   Loaded model info:\n");
    printf("     Data pointer: %p\n", (void*)model_info.data);
    printf("     Size: %zu bytes\n", model_info.size);
    printf("     Valid: %s\n", model_info.is_valid ? "YES" : "NO");
    
    // Test GGUF format validation
    printf("4. Testing GGUF format validation...\n");
    int format_valid = c_validate_gguf_format(model_info.data, model_info.size);
    printf("   GGUF format validation: %s\n", format_valid ? "VALID" : "INVALID");
    
    printf("\n=== Test completed successfully! ===\n");
    return 0;
}