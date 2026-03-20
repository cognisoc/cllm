// net_discovery.zig - Discover networking capabilities in Zig std library
const std = @import("std");

pub fn main() !void {
    std.debug.print("=== Networking Discovery ===\n", .{});
    
    // Check what's available in std.net
    std.debug.print("\nAvailable in std.net:\n", .{});
    
    // Try to create a TCP server using available functions
    std.debug.print("\nTrying to parse IP address...\n", .{});
    
    const address = try std.net.Address.parseIp4("127.0.0.1", 8080);
    std.debug.print("   Parsed address: {any}\n", .{address});
    
    // Test creating multiple addresses
    std.debug.print("\nTesting address creation performance...\n", .{});
    
    const start_time = std.time.nanoTimestamp();
    
    var i: usize = 0;
    while (i < 10000) : (i += 1) {
        _ = std.net.Address.initIp4([4]u8{ 127, 0, 0, 1 }, @truncate(@as(u16, @intCast(i % 65535 + 1))));
    }
    
    const end_time = std.time.nanoTimestamp();
    const elapsed_ns = end_time - start_time;
    const elapsed_ms = @as(f64, @floatFromInt(elapsed_ns)) / 1_000_000.0;
    
    std.debug.print("   10000 address creations took: {d:.2} ms\n", .{elapsed_ms});
    std.debug.print("   Average per creation: {d:.2} μs\n", .{elapsed_ms * 1000.0 / 10000.0});
    
    std.debug.print("\n=== Networking discovery completed successfully! ===\n", .{});
}