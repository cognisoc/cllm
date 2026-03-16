// enhanced_architecture_test.zig - Test for enhanced architecture components
const std = @import("std");
const model_embedding = @import("model_embedding.zig");

pub fn main() !void {
    std.debug.print("=== Enhanced Architecture Test ===\n", .{});
    
    // Test 1: Basic model embedding
    std.debug.print("\n1. Testing basic model embedding...\n", .{});
    const model_data = model_embedding.getEmbeddedModel();
    const model_size = model_embedding.getModelSize();
    
    std.debug.print("   Model data pointer: {any}\n", .{model_data.ptr});
    std.debug.print("   Model size: {} bytes\n", .{model_size});
    std.debug.print("   First 16 bytes: ", .{});
    for (model_data[0..@min(16, model_data.len)]) |byte| {
        std.debug.print("{X:0>2} ", .{byte});
    }
    std.debug.print("\n", .{});
    
    // Test 2: Model validation
    std.debug.print("\n2. Testing model validation...\n", .{});
    const is_valid_gguf = model_embedding.ModelValidator.validateGGUF(model_data);
    const is_valid_header = model_embedding.ModelValidator.validateHeader(model_data);
    
    std.debug.print("   GGUF format validation: {}\n", .{is_valid_gguf});
    std.debug.print("   Header validation: {}\n", .{is_valid_header});
    
    // Test 3: Model loader
    std.debug.print("\n3. Testing model loader...\n", .{});
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    
    var loader = model_embedding.ModelLoader.init(allocator);
    const loaded_model = try loader.loadEmbeddedModel();
    
    std.debug.print("   Loaded model size: {} bytes\n", .{loaded_model.len});
    
    const is_loaded_valid = try loader.validateModel(loaded_model);
    std.debug.print("   Loaded model validation: {}\n", .{is_loaded_valid});
    
    // Test 4: Model metadata
    std.debug.print("\n4. Testing model metadata...\n", .{});
    const metadata = model_embedding.getModelMetadata();
    
    std.debug.print("   Magic number: {s}\n", .{metadata.magic});
    std.debug.print("   Version: {}\n", .{metadata.version});
    std.debug.print("   Tensor count: {}\n", .{metadata.tensor_count});
    std.debug.print("   KV count: {}\n", .{metadata.kv_count});
    std.debug.print("   Architecture: {s}\n", .{metadata.architecture});
    std.debug.print("   Context length: {}\n", .{metadata.context_length});
    
    // Test 5: Model integrity verification
    std.debug.print("\n5. Testing model integrity verification...\n", .{});
    const is_integrity_valid = model_embedding.ModelValidator.validateIntegrity(model_data);
    const calculated_checksum = model_embedding.calculateChecksum(model_data);
    
    std.debug.print("   Model integrity validation: {}\n", .{is_integrity_valid});
    std.debug.print("   Calculated checksum: 0x{X:0>8}\n", .{calculated_checksum});
    std.debug.print("   Expected checksum: 0x{X:0>8}\n", .{@as(u32, 0x12345678)});
    std.debug.print("   Checksum match: {}\n", .{calculated_checksum == 0x12345678});
    
    // Test 6: Performance timing
    std.debug.print("\n6. Testing performance...\n", .{});
    const start_time = std.time.nanoTimestamp();
    
    // Load model multiple times to test performance
    var i: usize = 0;
    while (i < 1000) : (i += 1) {
        _ = model_embedding.getEmbeddedModel();
        _ = model_embedding.getModelSize();
    }
    
    const end_time = std.time.nanoTimestamp();
    const elapsed_ns = end_time - start_time;
    const elapsed_ms = @as(f64, @floatFromInt(elapsed_ns)) / 1_000_000.0;
    
    std.debug.print("   1000 iterations took: {d:.2} ms\n", .{elapsed_ms});
    std.debug.print("   Average per iteration: {d:.2} μs\n", .{elapsed_ms * 1000.0 / 1000.0});
    
    std.debug.print("\n=== Enhanced architecture test completed successfully! ===\n", .{});
}