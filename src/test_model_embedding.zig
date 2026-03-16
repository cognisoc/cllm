// test_model_embedding.zig - Simple test for model embedding
const std = @import("std");
const model_embedding = @import("model_embedding.zig");

pub fn main() !void {
    std.debug.print("Testing model embedding functionality...\n", .{});
    
    // Test getting embedded model
    const model_data = model_embedding.getEmbeddedModel();
    std.debug.print("Got embedded model of size: {} bytes\n", .{model_data.len});
    
    // Test model size
    const model_size = model_embedding.getModelSize();
    std.debug.print("Model size from function: {} bytes\n", .{model_size});
    
    // Verify sizes match
    if (model_data.len != model_size) {
        std.debug.print("ERROR: Size mismatch!\n", .{});
        return error.SizeMismatch;
    }
    
    // Test model loader
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    
    var loader = model_embedding.ModelLoader.init(allocator);
    const loaded_model = try loader.loadEmbeddedModel();
    
    std.debug.print("Loaded model of size: {} bytes\n", .{loaded_model.len});
    
    // Test validation
    const is_valid = try loader.validateModel(loaded_model);
    std.debug.print("Model validation: {}\n", .{is_valid});
    
    std.debug.print("Model embedding test completed successfully!\n", .{});
}