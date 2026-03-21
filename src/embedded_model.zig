// embedded_model.zig - Embedded model data
const embedded_model_data = &[_]u8{
    // GGUF magic number
    'G', 'G', 'U', 'F',
    // Version (3)
    0x03, 0x00, 0x00, 0x00,
    // Tensor count
    0x00, 0x00, 0x00, 0x00,
    // KV count
    0x00, 0x00, 0x00, 0x00,
    // Placeholder data
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
};

pub fn getEmbeddedModel() []const u8 {
    return embedded_model_data;
}

pub fn getModelSize() usize {
    return embedded_model_data.len;
}
