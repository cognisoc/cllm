const std = @import("std");

pub fn main() !void {
    try std.fs.File.stdout().writeAll("Hello from Zig!\n");
}