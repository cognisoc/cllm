const embedded_model_data = &[_]u8{};
pub fn getEmbeddedModel() []const u8 { return embedded_model_data; }
pub fn getModelSize() usize { return embedded_model_data.len; }
