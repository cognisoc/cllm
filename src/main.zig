const std = @import("std");
const config = @import("config.zig");

pub fn main() void {
    std.debug.print("Enhanced Unikernel Build System Test\n", .{});
    std.debug.print("==============================\n", .{});
    std.debug.print("Configuration:\n", .{});
    std.debug.print("  Context Size: {}\n", .{config.DEFAULT_CONTEXT_SIZE});
    std.debug.print("  Batch Size: {}\n", .{config.DEFAULT_BATCH_SIZE});
    std.debug.print("  Threads: {}\n", .{config.DEFAULT_THREADS});
    std.debug.print("  GPU Backend: {s}\n", .{config.DEFAULT_GPU_BACKEND});
    std.debug.print("  Has Model: {}\n", .{config.DEFAULT_HAS_MODEL});
    
    if (config.Config.USE_GPU) {
        std.debug.print("  GPU Acceleration: ENABLED\n", .{});
    } else {
        std.debug.print("  GPU Acceleration: DISABLED\n", .{});
    }
    
    std.debug.print("\nBuild system ready for enhanced unikernel compilation!\n", .{});
}