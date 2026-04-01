// c_model_interface.zig - Minimal Zig implementation of C model interface without std deps
const embedded = @import("embedded_model.zig");
const builtin = @import("builtin");

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

// C-compatible wrapper functions (no std use)
export fn c_get_embedded_model() [*]const u8 {
    const s = embedded.getEmbeddedModel();
    return s.ptr;
}

export fn c_get_embedded_model_size() usize {
    const s = embedded.getEmbeddedModel();
    return s.len;
}

export fn c_validate_embedded_model() c_int {
    const data = embedded.getEmbeddedModel();
    if (data.len < 4) return 0;
    return if (data[0] == 'G' and data[1] == 'G' and data[2] == 'U' and data[3] == 'F') 1 else 0;
}

export fn c_load_embedded_model() EmbeddedModelInfo {
    return EmbeddedModelInfo{
        .data = embedded.getEmbeddedModel().ptr,
        .size = embedded.getEmbeddedModel().len,
        .is_valid = c_validate_embedded_model(),
    };
}

export fn c_validate_gguf_format(data: ?[*]const u8, size: usize) c_int {
    if (data == null or size < 4) return 0;
    const p = data.?;
    return if (p[0] == 'G' and p[1] == 'G' and p[2] == 'U' and p[3] == 'F') 1 else 0;
}

export fn c_validate_header(data: ?[*]const u8, size: usize) c_int {
    if (c_validate_gguf_format(data, size) == 0) return 0;
    if (size < 16) return 0;
    return 1;
}

export fn c_get_model_metadata() ModelMetadataStruct {
    var arch_buffer: [32]u8 = [_]u8{0} ** 32;
    const arch = "llama";
    var i: usize = 0;
    while (i < arch.len and i < arch_buffer.len - 1) : (i += 1) {
        arch_buffer[i] = arch[i];
    }
    return ModelMetadataStruct{
        .magic = .{ 'G','G','U','F' },
        .version = 3,
        .tensor_count = 0,
        .kv_count = 0,
        .architecture = arch_buffer,
        .context_length = 2048,
    };
}

fn crc32(data: [*]const u8, size: usize) u32 {
    var crc: u32 = 0xFFFFFFFF;
    var i: usize = 0;
    while (i < size) : (i += 1) {
        crc ^= @as(u32, data[i]);
        var j: u32 = 0;
        while (j < 8) : (j += 1) {
            if ((crc & 1) != 0) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

export fn c_calculate_model_checksum(data: ?[*]const u8, size: usize) u32 {
    if (data == null or size == 0) return 0;
    return crc32(data.?, size);
}

export fn c_verify_model_integrity(data: ?[*]const u8, size: usize) c_int {
    if (data == null or size == 0) return 0;
    const checksum: u32 = crc32(data.?, size);
    return if (checksum == 0x12345678) 1 else 0;
}
