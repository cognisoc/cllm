// test_config.zig - Simple test for configuration system
const std = @import("std");
const config = @import("config.zig");

pub fn main() !void {
    std.debug.print("Configuration System Test\n", .{});
    
    // Test default configuration values
    std.debug.print("Context size: {}\n", .{config.DEFAULT_CONTEXT_SIZE});
    std.debug.print("Batch size: {}\n", .{config.DEFAULT_BATCH_SIZE});
    std.debug.print("Threads: {}\n", .{config.DEFAULT_THREADS});
    std.debug.print("GPU backend: {s}\n", .{config.DEFAULT_GPU_BACKEND});
    std.debug.print("Has model: {}\n", .{config.DEFAULT_HAS_MODEL});
    std.debug.print("Model path: {s}\n", .{config.DEFAULT_MODEL_PATH});
    std.debug.print("HTTP port: {}\n", .{config.DEFAULT_HTTP_PORT});
    std.debug.print("HTTP host: {s}\n", .{config.DEFAULT_HTTP_HOST});
    std.debug.print("Log level: {s}\n", .{config.DEFAULT_LOG_LEVEL});
    
    // Test compile-time configuration flags
    std.debug.print("USE_GPU: {}\n", .{config.Config.USE_GPU});
    std.debug.print("USE_CUDA: {}\n", .{config.Config.USE_CUDA});
    std.debug.print("USE_METAL: {}\n", .{config.Config.USE_METAL});
    std.debug.print("USE_VULKAN: {}\n", .{config.Config.USE_VULKAN});
    
    // Test configuration loader
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    
    var loader = config.ConfigLoader.init(allocator);
    const loaded_config = try loader.loadConfig();
    
    std.debug.print("Loaded config context size: {}\n", .{loaded_config.context_size});
    std.debug.print("Loaded config batch size: {}\n", .{loaded_config.batch_size});
    std.debug.print("Loaded config threads: {}\n", .{loaded_config.threads});
    std.debug.print("Loaded config GPU backend: {s}\n", .{loaded_config.gpu_backend});
    std.debug.print("Loaded config has model: {}\n", .{loaded_config.has_model});
    std.debug.print("Loaded config model path: {s}\n", .{loaded_config.model_path});
    std.debug.print("Loaded config HTTP port: {}\n", .{loaded_config.http_port});
    std.debug.print("Loaded config HTTP host: {s}\n", .{loaded_config.http_host});
    std.debug.print("Loaded config log level: {s}\n", .{loaded_config.log_level});
    
    // Test JSON configuration parser
    var parser = config.JsonConfigParser.init(allocator);
    const generated_json = try parser.generateJsonConfig(loaded_config);
    defer allocator.free(generated_json);
    
    std.debug.print("Generated JSON config: {s}\n", .{generated_json});
    
    // Test configuration validation
    const is_context_valid = config.ConfigValidator.validateContextSize(loaded_config.context_size);
    const is_batch_valid = config.ConfigValidator.validateBatchSize(loaded_config.batch_size);
    const is_threads_valid = config.ConfigValidator.validateThreads(loaded_config.threads);
    const is_gpu_valid = config.ConfigValidator.validateGpuBackend(loaded_config.gpu_backend);
    const is_port_valid = config.ConfigValidator.validateHttpPort(loaded_config.http_port);
    const is_host_valid = config.ConfigValidator.validateHttpHost(loaded_config.http_host);
    const is_log_valid = config.ConfigValidator.validateLogLevel(loaded_config.log_level);
    
    std.debug.print("Context size validation: {}\n", .{is_context_valid});
    std.debug.print("Batch size validation: {}\n", .{is_batch_valid});
    std.debug.print("Threads validation: {}\n", .{is_threads_valid});
    std.debug.print("GPU backend validation: {}\n", .{is_gpu_valid});
    std.debug.print("HTTP port validation: {}\n", .{is_port_valid});
    std.debug.print("HTTP host validation: {}\n", .{is_host_valid});
    std.debug.print("Log level validation: {}\n", .{is_log_valid});
    
    // Test performance timing
    const start_time = std.time.nanoTimestamp();
    
    // Load config multiple times to test performance
    var i: usize = 0;
    while (i < 1000) : (i += 1) {
        _ = try loader.loadConfig();
        _ = config.ConfigValidator.validateContextSize(loaded_config.context_size);
        _ = config.ConfigValidator.validateBatchSize(loaded_config.batch_size);
    }
    
    const end_time = std.time.nanoTimestamp();
    const elapsed_ns = end_time - start_time;
    const elapsed_ms = @as(f64, @floatFromInt(elapsed_ns)) / 1_000_000.0;
    
    std.debug.print("1000 iterations took: {d:.2} ms\n", .{elapsed_ms});
    std.debug.print("Average per iteration: {d:.2} μs\n", .{elapsed_ms * 1000.0 / 1000.0});
    
    std.debug.print("Configuration system test completed successfully!\n", .{});
}