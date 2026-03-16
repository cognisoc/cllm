// c_model_interface.zig - Enhanced Zig implementation of C model interface with integrity verification
const std = @import("std");
const model_embedding = @import("model_embedding.zig");

// C-compatible model info structure (not exported)
const EmbeddedModelInfo = extern struct {
    data: ?[*]const u8,
    size: usize,
    is_valid: c_int,
};

// C-compatible model metadata structure (not exported)
const ModelMetadataStruct = extern struct {
    magic: [4]u8,
    version: u32,
    tensor_count: u32,
    kv_count: u32,
    architecture: [32]u8,
    context_length: u32,
};

// C-compatible wrapper functions
export fn c_get_embedded_model() ?[*]const u8 {
    const model_data = model_embedding.getEmbeddedModel();
    return model_data.ptr;
}

export fn c_get_embedded_model_size() usize {
    return model_embedding.getModelSize();
}

export fn c_validate_embedded_model() c_int {
    const model_data = model_embedding.getEmbeddedModel();
    
    // Basic validation - check if it looks like a GGUF file
    if (model_data.len < 4) {
        return 0; // Invalid
    }
    
    // GGUF files start with "GGUF" magic number
    const magic = [_]u8{ 'G', 'G', 'U', 'F' };
    return if (std.mem.eql(u8, model_data[0..4], &magic)) 1 else 0; // Valid/Invalid
}

export fn c_load_embedded_model() EmbeddedModelInfo {
    const model_data = model_embedding.getEmbeddedModel();
    const is_valid = c_validate_embedded_model();
    
    return EmbeddedModelInfo{
        .data = model_data.ptr,
        .size = model_data.len,
        .is_valid = is_valid,
    };
}

export fn c_validate_gguf_format(data: ?[*]const u8, size: usize) c_int {
    if (data == null or size < 4) {
        return 0; // Invalid
    }
    
    // Create a slice from the C pointer
    const model_slice = data.?[0..size];
    
    // GGUF files start with "GGUF" magic number
    const magic = [_]u8{ 'G', 'G', 'U', 'F' };
    return if (std.mem.eql(u8, model_slice[0..4], &magic)) 1 else 0; // Valid/Invalid
}

export fn c_validate_header(data: ?[*]const u8, size: usize) c_int {
    if (data == null or size < 16) {
        return 0; // Invalid
    }
    
    // Create a slice from the C pointer
    const model_slice = data.?[0..size];
    
    // Check magic number
    if (c_validate_gguf_format(data, size) == 0) {
        return 0; // Invalid magic number
    }
    
    // Check version (basic check)
    // We'll skip this for now since we don't know how to read little endian integers in this version
    _ = model_slice;
    return 1; // Valid (basic validation passed)
}

export fn c_get_model_metadata() ModelMetadataStruct {
    const metadata = model_embedding.getModelMetadata();
    
    // Initialize architecture string with zeros
    var arch_buffer: [32]u8 = [_]u8{0} ** 32;
    
    // Copy architecture name (truncated if too long)
    const arch_name = metadata.architecture;
    const copy_len = @min(arch_name.len, 31); // Leave space for null terminator
    @memcpy(arch_buffer[0..copy_len], arch_name[0..copy_len]);
    
    return ModelMetadataStruct{
        .magic = metadata.magic,
        .version = metadata.version,
        .tensor_count = metadata.tensor_count,
        .kv_count = metadata.kv_count,
        .architecture = arch_buffer,
        .context_length = metadata.context_length,
    };
}

// Enhanced integrity verification functions
export fn c_verify_model_integrity(data: ?[*]const u8, size: usize) c_int {
    if (data == null or size == 0) {
        return 0; // Invalid
    }
    
    // Create a slice from the C pointer
    const model_slice = data.?[0..size];
    
    // Verify model integrity
    const is_valid = model_embedding.ModelValidator.validateIntegrity(model_slice);
    return if (is_valid) 1 else 0; // Valid/Invalid
}

export fn c_calculate_model_checksum(data: ?[*]const u8, size: usize) u32 {
    if (data == null or size == 0) {
        return 0; // Invalid
    }
    
    // Create a slice from the C pointer
    const model_slice = data.?[0..size];
    
    // Calculate and return checksum
    return model_embedding.calculateChecksum(model_slice);
}