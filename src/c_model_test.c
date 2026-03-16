// c_model_test.c - Enhanced C test for model embedding interface with integrity verification
#include <stdio.h>
#include <stdlib.h>
#include "../include/c_model_interface.h"

int main() {
    printf("=== Enhanced C Model Embedding Test ===\n");
    
    // Test 1: Basic model access
    printf("\n1. Testing basic model access...\n");
    const uint8_t* model_data = c_get_embedded_model();
    size_t model_size = c_get_embedded_model_size();
    
    printf("   Model data pointer: %p\n", (void*)model_data);
    printf("   Model size: %zu bytes\n", model_size);
    printf("   First 16 bytes: ");
    for (int i = 0; i < 16 && i < model_size; i++) {
        printf("%02X ", model_data[i]);
    }
    printf("\n");
    
    // Test 2: Model validation
    printf("\n2. Testing model validation...\n");
    int is_valid = c_validate_embedded_model();
    printf("   Model validation: %s\n", is_valid ? "VALID" : "INVALID");
    
    // Test 3: Model loading
    printf("\n3. Testing model loading...\n");
    embedded_model_info_t model_info = c_load_embedded_model();
    printf("   Loaded model info:\n");
    printf("     Data pointer: %p\n", (void*)model_info.data);
    printf("     Size: %zu bytes\n", model_info.size);
    printf("     Valid: %s\n", model_info.is_valid ? "YES" : "NO");
    
    // Test 4: GGUF format validation
    printf("\n4. Testing GGUF format validation...\n");
    int format_valid = c_validate_gguf_format(model_info.data, model_info.size);
    printf("   GGUF format validation: %s\n", format_valid ? "VALID" : "INVALID");
    
    // Test 5: Header validation
    printf("\n5. Testing header validation...\n");
    int header_valid = c_validate_header(model_info.data, model_info.size);
    printf("   Header validation: %s\n", header_valid ? "VALID" : "INVALID");
    
    // Test 6: Model metadata
    printf("\n6. Testing model metadata...\n");
    model_metadata_t metadata = c_get_model_metadata();
    printf("   Magic number: %.4s\n", metadata.magic);
    printf("   Version: %u\n", metadata.version);
    printf("   Tensor count: %u\n", metadata.tensor_count);
    printf("   KV count: %u\n", metadata.kv_count);
    printf("   Architecture: %s\n", metadata.architecture);
    printf("   Context length: %u\n", metadata.context_length);
    
    // Test 7: Model integrity verification
    printf("\n7. Testing model integrity verification...\n");
    int integrity_valid = c_verify_model_integrity(model_info.data, model_info.size);
    printf("   Model integrity verification: %s\n", integrity_valid ? "VALID" : "INVALID");
    
    // Test 8: Checksum calculation
    printf("\n8. Testing checksum calculation...\n");
    uint32_t checksum = c_calculate_model_checksum(model_info.data, model_info.size);
    printf("   Calculated checksum: 0x%08X\n", checksum);
    printf("   Expected checksum: 0x12345678\n");
    printf("   Checksum match: %s\n", (checksum == 0x12345678) ? "YES" : "NO");
    
    printf("\n=== Enhanced C test completed successfully! ===\n");
    return 0;
}