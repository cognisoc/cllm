// simple_model_test.zig - Simple test for model embedding functionality
const std = @import("std");
const model_embedding = @import("model_embedding.zig");

pub fn main() !void {
    std.debug.print("=== Simple Model Embedding Test ===\n", .{});
    
    // Test basic model access
    const model_data = model_embedding.getEmbeddedModel();
    const model_size = model_embedding.getModelSize();
    
    std.debug.print("Model data pointer: {any}\n", .{model_data.ptr});
    std.debug.print("Model size: {} bytes\n", .{model_size});
    
    // Test model validation
    const is_valid_gguf = model_embedding.ModelValidator.validateGGUF(model_data);
    const is_valid_header = model_embedding.ModelValidator.validateHeader(model_data);
    
    std.debug.print("GGUF format validation: {}\n", .{is_valid_gguf});
    std.debug.print("Header validation: {}\n", .{is_valid_header});
    
    // Test model loader
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    
    var loader = model_embedding.ModelLoader.init(allocator);
    const loaded_model = try loader.loadEmbeddedModel();
    
    std.debug.print("Loaded model size: {} bytes\n", .{loaded_model.len});
    
    const is_loaded_valid = try loader.validateModel(loaded_model);
    std.debug.print("Loaded model validation: {}\n", .{is_loaded_valid});
    
    // Test model metadata
    const metadata = model_embedding.getModelMetadata();
    
    std.debug.print("Magic number: {s}\n", .{metadata.magic});
    std.debug.print("Version: {}\n", .{metadata.version});
    std.debug.print("Tensor count: {}\n", .{metadata.tensor_count});
    std.debug.print("KV count: {}\n", .{metadata.kv_count});
    std.debug.print("Architecture: {s}\n", .{metadata.architecture});
    std.debug.print("Context length: {}\n", .{metadata.context_length});
    
    std.debug.print("\n=== Test completed successfully! ===\n", .{});
}