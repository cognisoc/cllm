// simple_config_test.zig - Simple test for configuration system
const std = @import("std");
const config = @import("config.zig");

pub fn main() !void {
    std.debug.print("Simple Configuration System Test\n", .{});
    
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
    
    std.debug.print("Simple configuration test completed successfully!\n", .{});
}