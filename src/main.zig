const std = @import("std");
const config = @import("config.zig");

pub fn main() void {
    std.debug.print("Enhanced Unikernel Build System Test\n", .{});
    std.debug.print("==============================\n", .{});
    std.debug.print("Configuration:\n", .{});
    std.debug.print("  Context Size: {}\n", .{config.Config.CONTEXT_SIZE});
    std.debug.print("  Batch Size: {}\n", .{config.Config.BATCH_SIZE});
    std.debug.print("  Threads: {}\n", .{config.Config.THREADS});
    std.debug.print("  GPU Backend: {s}\n", .{config.Config.GPU_BACKEND});
    std.debug.print("  Has Model: {}\n", .{config.Config.HAS_MODEL});
    
    if (config.Config.USE_GPU) {
        std.debug.print("  GPU Acceleration: ENABLED\n", .{});
    } else {
        std.debug.print("  GPU Acceleration: DISABLED\n", .{});
    }
    
    std.debug.print("\nBuild system ready for enhanced unikernel compilation!\n", .{});
}